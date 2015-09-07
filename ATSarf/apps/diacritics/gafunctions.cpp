#include <algorithm>
#include <string>
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "gafunctions.h"

bool dgGeneticAlgorithm(QHash<QString, quint8>& hash, QStringList& listStemPOS) {
    // Initialization
    QVector<QVector<int> > population(NUM_OF_SOLUTIONS, QVector<int>(NUM_OF_FEATURES));
    QVector<QVector<int> > parents(2, QVector<int>(NUM_OF_FEATURES));
    QVector<double> fitness(NUM_OF_SOLUTIONS);

    // start with random individuals with stem length, step POS, and diacritic position set
    srand(time(NULL));
    for(int i=0; i<NUM_OF_SOLUTIONS; i++ ) {
        int randSL = rand() % STEM_LENGTH;//rand() / (float)RAND_MAX >= 0.5 ? 1:0;
        population[i][randSL] = 1;
        int randSP = STEM_LENGTH + (rand() % STEM_POS);
        population[i][randSP] = 1;
        int randDP = (STEM_LENGTH + STEM_POS) + (rand() % DIAC_POS);
        population[i][randDP] = 1;
    }

    // evaluation(fitness function)
    if(!evaluation(hash,population,fitness,listStemPOS)) {
        cout << "Problem in initial evaluation!!\n";
        return false;
    }

    bool stop = false;
    int solutionIndex = -1;
    cout << "Initial evaluation..." << endl;
    for(int i=0; i<fitness.count(); i++) {
        cout << fitness[i] << ' ';
        if(fitness[i] >= 0.8) {
            stop = true;
            solutionIndex = i;
        }
    }

    int iterations = 1;
    while(!stop) {
        // Selection
        int parent1Index = -1;
        int parent2Index = -1;
        if(!selection(parents,population,fitness,parent1Index,parent2Index)) {
            cout << "Problem in parent selection!!\n";
            return false;
        }

        // crossover
        QVector<int> child(NUM_OF_FEATURES);
        if(!crossover(parents,child)) {
            cout << "Problem in crossover!!\n";
            return false;
        }

        // mutation
        if(!mutation(child)) {
            cout << "Problem in mutation!!\n";
            return false;
        }

        for(int i=0; i<child.count(); i++) {
            if(child.at(i) == 1) {
                cout << i << ' ';
            }
        }
        cout << endl;

        // evaluate child
        QVector<QVector<int> > individual;
        individual.append(child);
        QVector<double> childFitness(1);
        if(!evaluation(hash,individual,childFitness,listStemPOS)) {
            cout << "Problem in evaluation!!\n";
            return false;
        }

        // replace less fit parent with child if the latter is better
        if(childFitness[0] >= fitness[parent1Index] && fitness[parent1Index] >= fitness[parent2Index]) {
            fitness[parent1Index] = childFitness[0];
            for(int i=0; i< NUM_OF_FEATURES; i++) {
                population[parent1Index][i] = child[i];
            }
        }
        else if(childFitness[0] >= fitness[parent2Index] && fitness[parent2Index] > fitness[parent1Index]) {
            fitness[parent2Index] = childFitness[0];
            for(int i=0; i< NUM_OF_FEATURES; i++) {
                population[parent2Index][i] = child[i];
            }
        }

        // check if any solution reached the target fitness
        cout << "Fitness at iteration " << iterations+1 << endl;
        for(int i=0; i<fitness.count(); i++) {
            cout << fitness[i] << ' ';
            if(fitness[i] >= 0.8) {
                stop = true;
                solutionIndex = i;
            }
        }

        iterations++;
        if(iterations == 10000) {
            stop = true;
        }
    }

    if(solutionIndex == -1) {
        cout << "No solution found with 10000 iterations" << endl;
    }
    else {
        cout << "The fitness of the solution is: " << fitness.at(solutionIndex) << endl;
        cout << "The solution is:" << endl;
        for(int i=0; i<STEM_LENGTH; i++) {
            if(population[solutionIndex][i] == 1) {
                cout << "Stem length: " << i+1 << endl;
            }
        }
        for(int i=STEM_LENGTH; i<STEM_LENGTH + STEM_POS; i++) {
            if(population[solutionIndex][i] == 1) {
                cout << "POS tag: " << listStemPOS[i-STEM_LENGTH].toStdString() << endl;
            }
        }
        if(population[solutionIndex][NUM_OF_FEATURES-3] == 1) {
            cout << "diacritic position at stem start" <<endl;
        }
        else if(population[solutionIndex][NUM_OF_FEATURES-2] == 1) {
            cout << "diacritic position at stem middle" <<endl;
        }
        else if(population[solutionIndex][NUM_OF_FEATURES-1] == 1) {
            cout << "diacritic position at stem end" <<endl;
        }
    }
    return true;
}

bool evaluation(QHash<QString, quint8>& hash, QVector<QVector<int> >& population, QVector<double>& fitness, QStringList& stemPOSList) {

    // Stores the count of high reduction 1-diacritic solutions matching the individuals
    QVector<int> hRedSol(population.count());
    // Stores the count of 1-riacritic solutions matching the individuals
    QVector<int> totalMSol(population.count());

    // reinitialize fitness values
    for(int i=0; i<fitness.count(); i++) {
        fitness[i] = 0;
    }

    // Iterate over all the Arabic stems in the lexicon
    QHashIterator<QString, quint8> it(hash);
    while (it.hasNext()) {
        it.next();
        QString word = it.key();
        int count = 0;

        // Get the morphological solutions of the word
        WordAnalysis wa(&word,&count);
        wa();

        // skip unambiguous words, i.e. single morphological solution
        if(wa.solutions.count() <= 1) {
            continue;
        }

        // Iteraye over the morphological analyses of the word
        for(int i=0; i<wa.solutions.count(); i++) {
            const Solution& sol = wa.solutions.at(i);
            // skip solutions with '+' in stem POS or containing affixes
            if(!(sol.isValid)) {
                continue;
            }

            // Detach diacritics from raw_data and store in separate structure
            int diacritic_Counter = 0;
            QVector<QVector<QChar> > wordDiacritics(sol.length);
            int letterIndex = 0;
            for(int i=1; i<sol.vStem.count(); i++) {
                QChar currentLetter= sol.vStem[i];
                if(isDiacritic(currentLetter)) {
                    wordDiacritics[letterIndex].append(currentLetter);
                    diacritic_Counter++;
                }
                else {
                    letterIndex++;
                }
            }

            // Skip evaluating this morpho. solution if the stem has no diacritics
            if(diacritic_Counter == 0) {
                continue;
            }

            // Iterate over the current proposed solutions and update evaluation
            for(int j=0; j<population.count(); j++) {
                const QVector<int>& individual = population.at(j);

                // Skip if some unmatching stem length is set to 1 in current individual
                bool skipInd = false;
                for(int len=1; len<=STEM_LENGTH; len++) {
                    if(individual[len-1] == 1 && len != sol.length) {
                        skipInd = true;
                        break;
                    }
                }
                if(skipInd) {
                    continue;
                }

                // Skip if some unmatching stem POS is set to 1 in current individual
                int posIndex = STEM_LENGTH + stemPOSList.indexOf(sol.stemPOS);
                for(int pos = STEM_LENGTH; pos< STEM_LENGTH + STEM_POS; pos++) {
                    if(individual[pos] == 1 && pos != posIndex) {
                        skipInd = true;
                        break;
                    }
                }
                if(skipInd) {
                    continue;
                }

                // evaluate individuals with only one diacritic position set
                int numofDiac = individual[NUM_OF_FEATURES-3] + individual[NUM_OF_FEATURES-2] + individual[NUM_OF_FEATURES-1];
                if(numofDiac != 1) {
                    continue;
                }

                // Check if diacritic position in the individual is first, middle, or end of the stem
                if(individual[NUM_OF_FEATURES-3] == 1) {
                    // diacritic at stem beginning
                    for(int k=0; k< wordDiacritics.first().count(); k++) {
                        // construct the one diacritic stem
                        QString oneDiacWord = removeDiacritics(sol.vStem);
                        oneDiacWord.insert(1,wordDiacritics.first().at(k));

                        // Iterate over the morpho. solutions and count consistent solutions
                        double oneDiacSols = 0;
                        for(int m=0; m<wa.solutions.count(); m++) {
                            if(equal(oneDiacWord,wa.solutions.at(m).vStem)) {
                                oneDiacSols++;
                            }
                        }

                        // update fitness counter if morpho. reduction is greater than or equal to 0.5
                        totalMSol[j] = totalMSol[j] + 1;
                        if((wa.solutions.count()-oneDiacSols)/wa.solutions.count() >= 0.5) {
                            hRedSol[j] = hRedSol[j] + 1;
                        }
                    }
                }
                else if(individual[NUM_OF_FEATURES-2] == 1) {
                    // diacritic at stem middle
                    for(int m=1; m< wordDiacritics.count()-1; m++) {
                        for(int n=0; n< wordDiacritics.at(m).count(); n++) {
                            // construct the one diacritic stem
                            QString oneDiacWord = removeDiacritics(sol.vStem);
                            oneDiacWord.insert(m+1,wordDiacritics[m][n]);

                            // Iterate over the morpho. solutions and count consistent solutions
                            double oneDiacSols = 0;
                            for(int l=0; l<wa.solutions.count(); l++) {
                                if(equal(oneDiacWord,wa.solutions.at(l).vStem)) {
                                    oneDiacSols++;
                                }
                            }

                            // update fitness counter if morpho. reduction is greater than or equal to 0.5
                            totalMSol[j] = totalMSol[j] + 1;
                            if((wa.solutions.count()-oneDiacSols)/wa.solutions.count() >= 0.5) {
                                hRedSol[j] = hRedSol[j] + 1;
                            }
                        }
                    }
                }
                else if(individual[NUM_OF_FEATURES-1] == 1){
                    // diacritic at stem end
                    for(int k=0; k< wordDiacritics.last().count(); k++) {
                        // construct the one diacritic stem
                        QString oneDiacWord = removeDiacritics(sol.vStem);
                        oneDiacWord.append(wordDiacritics.last().at(k));

                        // Iterate over the morpho. solutions and count consistent solutions
                        double oneDiacSols = 0;
                        for(int m=0; m<wa.solutions.count(); m++) {
                            if(equal(oneDiacWord,wa.solutions.at(m).vStem)) {
                                oneDiacSols++;
                            }
                        }

                        // update fitness counter if morpho. reduction is greater than or equal to 0.5
                        totalMSol[j] = totalMSol[j] + 1;
                        if((wa.solutions.count()-oneDiacSols)/wa.solutions.count() >= 0.5) {
                            hRedSol[j] = hRedSol[j] + 1;
                        }
                    }
                }
//                else {
//                    cout << "non-of the diacritic positions are set" << endl;
//                    //return false;
//                }
            }
        }
    }

    for(int i=0; i<fitness.count(); i++) {
        if(totalMSol[i] == 0) {
            fitness[i] = 0;
        }
        else {
            fitness[i] = (hRedSol[i]*1.0)/ totalMSol[i];
        }
    }

    return true;
}

bool selection(QVector<QVector<int> >& parents, QVector<QVector<int> >& population, QVector<double>& fitness, int& parent1Index, int& parent2Index) {
    double totalScore = 0;
    for(int i=0; i< fitness.count(); i++) {
            totalScore += (fitness[i]*100);
    }

    if(totalScore == 0) {
        // select two parents randomly
        srand(time(NULL));
        int bcValue = rand() % population.count();
        for(int i=0; i<population.at(bcValue).count(); i++) {
            parents[0][i] = population[bcValue][i];
        }
        parent1Index = bcValue;
        bcValue = rand() % population.count();
        for(int i=0; i<population.at(bcValue).count(); i++) {
            parents[1][i] = population[bcValue][i];
        }
        parent2Index = bcValue;
        return true;
    }

    double low = 0;
    double high = 0;
    for(int i=0; i<2; i++) {
        double bcValue = rand() / (float)RAND_MAX * totalScore;
        for(int j=0; j<population.count(); j++) {
            high += (fitness.at(j)*100);
            if(bcValue >= low && bcValue <= high) {
                // choose this individual in new population
                for(int k=0; k<NUM_OF_FEATURES; k++) {
                    parents[i][k] = population[j][k];
                }
                if(i == 0) {
                    parent1Index = j;
                }
                else {
                    parent2Index = j;
                }
                break;
            }
            low += (fitness.at(j)*100);
        }
        low = 0;
        high = 0;
    }
    return true;
}

bool crossover(QVector<QVector<int> >& parents, QVector<int>& child) {
    srand(time(NULL));
    // randomly select the single point crossover
    int spc = 1 + (rand() % (NUM_OF_FEATURES-1));
    for(int i=0; i<spc; i++) {
        child[i] = parents[0][i];
    }
    for(int i=spc; i<NUM_OF_FEATURES; i++) {
        child[i] = parents[1][i];
    }
    return true;
}

bool mutation(QVector<int>& individual) {

    srand(time(NULL));
    for(int i=0; i<individual.count(); i++) {
        double randMu = rand() / (float)RAND_MAX;
        if(randMu <= 0.1) {
            individual[i] = 1 - individual[i];
        }
    }
    return true;
}

bool dpIterApriori(QHash<QString, quint8>& hash, QString target, int supp, double conf) {

    // Define hashes to use for k and k-1 frequent items
    QHash<QString, int> *currItemCount = new QHash<QString, int>();
    QHash<QString, int> *prevItemCount = new QHash<QString, int>();
    // Define hash to use for item to index mapping
    QHash<QString, int> fMap;
    fMap.insert(target,0);
    bool isDone = false;
    int k = 1;

    // iterate first time to build the count of single items
    cout << "Creating 1-itemset patterns...\n";
    iAIterateDataSet(hash,currItemCount,prevItemCount,fMap,k,target);
    k++;
    isDone = currItemCount->isEmpty();
    while(!isDone) {
        // clean: move k-1 frequent items to previous
        prevItemCount->clear();
        delete prevItemCount;
        prevItemCount = currItemCount;
        currItemCount = new QHash<QString, int>();

        // iterate over data and create the k'th frequent item sets
        cout << "Creating " << k << "-itemset patterns...\n";
        iAIterateDataSet(hash,currItemCount,prevItemCount,fMap,k,target);
        isDone = currItemCount->isEmpty();
        if(!isDone) {
            isDone = !(iAGenerateRules(currItemCount,prevItemCount,fMap,target,supp,conf));
        }
        k++;
    }
    return true;
}

bool iAIterateDataSet(QHash<QString, quint8>& hash, QHash<QString, int> *itemCount, QHash<QString, int> *prevItemCount, QHash<QString, int> &fMap, int k, QString target) {
#ifndef TEST
    QHashIterator<QString, quint8> it(hash);
    while (it.hasNext()) {
        it.next();
        QString word = it.key();
        int count = 0;

        // Get the morphological solutions of the word
        WordAnalysis wa(&word,&count);
        wa();

        // skip unambiguous words, i.e. single morphological solution
        if(wa.solutions.count() <= 1) {
            continue;
        }

        // Iterate over the morphological analyses of the word
        for(int i=0; i<wa.solutions.count(); i++) {
            const Solution& sol = wa.solutions.at(i);
            // skip solutions with '+' in stem POS or containing affixes
            if(!(sol.isValid)) {
                //skippedM++;
                continue;
            }

            // Detach diacritics from raw_data and store in separate structure
            int diacritic_Counter = 0;
            QVector<QVector<QChar> > wordDiacritics(sol.length);
            int letterIndex = 0;
            for(int j=1; j<sol.vWord.count(); j++) {
                QChar currentLetter= sol.vWord[j];
                if(isDiacritic(currentLetter)) {
                    wordDiacritics[letterIndex].append(currentLetter);
                    diacritic_Counter++;
                }
                else {
                    letterIndex++;
                }
            }

            // Skip evaluating this morpho. solution if the stem has no diacritics
            if(diacritic_Counter == 0) {
                //skippedMD++;
                continue;
            }

            // Process diacritics at first, middle, and end of the stem
            for(int m=0; m< wordDiacritics.count(); m++) {
                for(int n=0; n< wordDiacritics.at(m).count(); n++) {
                    // construct the one diacritic stem
                    QString oneDiacWord = removeDiacritics(sol.vWord);
                    oneDiacWord.insert(m+1,wordDiacritics[m][n]);

                    // Count consistent morphological solutions
                    double oneDiacSols = 0;
                    for(int l=0; l<wa.solutions.count(); l++) {
                        if(oneDiacConsistency(oneDiacWord,wa.solutions[l].vWord,m)) {
                            oneDiacSols++;
                        }
                    }

                    // Count consistent vocalizations
                    double oneDiacVoc = 0;
                    QSet<QString> voc;
                    for(int l=0; l<wa.solutions.count(); l++) {
                        if(!(voc.contains(wa.solutions.at(l).vWord))) {
                            voc.insert(wa.solutions.at(l).vWord);
                            if(oneDiacConsistency(oneDiacWord,wa.solutions[l].vWord,m)) {
                                oneDiacVoc++;
                            }
                        }
                    }

                    // Here we have a transaction to process
                    QVector<int> transaction;
                    // word length
                    QString wl = "wl" + QString::number(sol.length);
                    if(fMap.contains(wl)) {
                        transaction.append(fMap.value(wl));
                    }
                    else {
                        int index = fMap.count();
                        fMap.insert(wl,index);
                        transaction.append(index);
                    }
                    // number of morphemes
                    QString nm = "nm" + QString::number(sol.number_of_morphemes);
                    if(fMap.contains(nm)) {
                        transaction.append(fMap.value(nm));
                    }
                    else {
                        int index = fMap.count();
                        fMap.insert(nm,index);
                        transaction.append(index);
                    }
                    // stem length
                    QString sl = "sl" + QString::number(sol.stem_length);
                    if(fMap.contains(sl)) {
                        transaction.append(fMap.value(sl));
                    }
                    else {
                        int index = fMap.count();
                        fMap.insert(sl,index);
                        transaction.append(index);
                    }
                    // stem POS
                    QString s = 's' + sol.stemPOS;
                    if(fMap.contains(s)) {
                        transaction.append(fMap.value(s));
                    }
                    else {
                        int index = fMap.count();
                        fMap.insert(s,index);
                        transaction.append(index);
                    }

                    // prefixes and POS tags
                    for(int j=0; j<sol.prefixes.count(); j++) {
                        QString p = 'p' + QString::number(sol.prefixes.count()-j) + sol.prefixes[j];
                        if(fMap.contains(p)) {
                            transaction.append(fMap.value(p));
                        }
                        else {
                            int index = fMap.count();
                            fMap.insert(p,index);
                            transaction.append(index);
                        }
                    }
                    for(int j=0; j<sol.prefixPOSs.count(); j++) {
                        QString pp = "pp" + QString::number(sol.prefixPOSs.count()-j) + sol.prefixPOSs[j];
                        if(fMap.contains(pp)) {
                            transaction.append(fMap.value(pp));
                        }
                        else {
                            int index = fMap.count();
                            fMap.insert(pp,index);
                            transaction.append(index);
                        }
                    }

                    // suffixes and POS tags
                    for(int j=0; j<sol.suffixes.count(); j++) {
                        QString x = 'x' + QString::number(j+1) + sol.suffixes[j];
                        if(fMap.contains(x)) {
                            transaction.append(fMap.value(x));
                        }
                        else {
                            int index = fMap.count();
                            fMap.insert(x,index);
                            transaction.append(index);
                        }
                    }
                    for(int j=0; j<sol.suffixPOSs.count(); j++) {
                        QString xp = "xp" + QString::number(j+1) + sol.suffixPOSs[j];
                        if(fMap.contains(xp)) {
                            transaction.append(fMap.value(xp));
                        }
                        else {
                            int index = fMap.count();
                            fMap.insert(xp,index);
                            transaction.append(index);
                        }
                    }

                    // diacritic added
                    QString d = 'd' + wordDiacritics[m][n];
                    if(fMap.contains(d)) {
                        transaction.append(fMap.value(d));
                    }
                    else {
                        int index = fMap.count();
                        fMap.insert(d,index);
                        transaction.append(index);
                    }

                    // diacritic position
                    QString diacritic_position;
                    if((sol.prefix_length != 0) && (m == 0)) {
                        diacritic_position = "dpprefixs";
                    }
                    else if((sol.prefix_length != 0) && (m > 0) && (m < (sol.prefix_length-1))) {
                        diacritic_position = "dpprefixm";
                    }
                    else if((sol.prefix_length != 0) && (m == (sol.prefix_length-1))) {
                        diacritic_position = "dpprefixe";
                    }
                    else if(m == sol.prefix_length) {
                        diacritic_position = "dpstems";
                    }
                    else if((m > (sol.prefix_length)) && (m < (sol.prefix_length + sol.stem_length - 1))) {
                        diacritic_position = "dpstemm";
                    }
                    else if(m == (sol.prefix_length + sol.stem_length - 1)) {
                        diacritic_position = "dpsteme";
                    }
                    else if((sol.suffix_length != 0) && (m == (sol.prefix_length + sol.stem_length))) {
                        diacritic_position = "dpsuffixs";
                    }
                    else if((sol.suffix_length != 0) && (m > (sol.prefix_length + sol.stem_length)) && (m < (sol.length - 1))) {
                        diacritic_position = "dpsuffixm";
                    }
                    else if((sol.suffix_length != 0) && (m == (sol.length -1))) {
                        diacritic_position = "dpsuffixe";
                    }
                    else {
                        cout << "Couldn't set diacritic position!" << endl;
                        return false;
                    }

                    if(fMap.contains(diacritic_position)) {
                        transaction.append(fMap.value(diacritic_position));
                    }
                    else {
                        int index = fMap.count();
                        fMap.insert(diacritic_position,index);
                        transaction.append(index);
                    }

                    // calculate morpho. reduction and discretize
                    QString mReduction;
                    double morphoReduction = (wa.solutions.count()-oneDiacSols)/wa.solutions.count();
                    if( morphoReduction >= 0.5) {
                        // high reduction
                        mReduction = "mHIGH";
                    }
                    else if(morphoReduction >= 0.2) {
                        // average reduction
                        mReduction = "mAVRG";
                    }
                    else {
                        // low reduction
                        mReduction = "mLOW";
                    }

                    if(fMap.contains(mReduction)) {
                        transaction.append(fMap.value(mReduction));
                    }
                    else {
                        int index = fMap.count();
                        fMap.insert(mReduction,index);
                        transaction.append(index);
                    }

                    // calculate vocalization reduction and discretize
                    QString vReduction;
                    double vocReduction = (voc.count()-oneDiacVoc)/voc.count();
                    if(vocReduction >= 0.5) {
                        vReduction = "vHIGH";
                    }
                    else if(vocReduction >= 0.2) {
                        vReduction = "vAVRG";
                    }
                    else {
                        vReduction = "vLOW";
                    }

                    if(fMap.contains(vReduction)) {
                        transaction.append(fMap.value(vReduction));
                    }
                    else {
                        int index = fMap.count();
                        fMap.insert(vReduction,index);
                        transaction.append(index);
                    }
#else
    QVector<QVector<QString> > data(10);
    data[0] << "a" << "b" << "c";
    data[1] << "a" << "d" << "e";
    data[2] << "b" << "c" << "d";
    data[3] << "a" << "b" << "c" << "d";
    data[4] << "b" << "c";
    data[5] << "a" << "b" << "d";
    data[6] << "d" << "e";
    data[7] << "a" << "b" << "c" << "d";
    data[8] << "c" << "d" << "e";
    data[9] << "a" << "b" << "c";

    for(int i=0; i<data.count(); i++) {
        QVector<int> transaction;

        for(int m=0; m<data.at(i).count(); m++) {

            QString dataItem = data[i][m];
            if(fMap.contains(dataItem)) {
                transaction.append(fMap.value(dataItem));
            }
            else {
                int index = fMap.count();
                fMap.insert(dataItem,index);
                transaction.append(index);
            }
        }
#endif
                    // skip if number of features in transaction is less than the target size of the k-frequent itemset
                    if(transaction.count() < k) {
                        continue;
                    }

                    // find the possible k
                    string bitmask(k, 1); // K leading 1's
                    bitmask.resize(transaction.count(), 0); // N-K trailing 0's

                    // find all permutations, check for apriori, and add to k-Item Sets
                    do {
                        QVector<int> kItemIndex;
                        for (int j = 0; j < transaction.count(); ++j) // [0..N-1] integers
                        {
                            if (bitmask[j]) {
                                kItemIndex.append(transaction[j]);
                            }
                        }

                        // check for apriori property, i.e. all k-1 subsets are frequent
                        bool add = true;

                        if(!(prevItemCount->isEmpty())) {
                            string bitmask2(kItemIndex.count()-1, 1); // K leading 1's
                            bitmask2.resize(kItemIndex.count(), 0); // N-K trailing 0's

                            do {
                                bool skip = false;
                                QString k1Item;
                                for (int o = 0; o < kItemIndex.count(); ++o) // [0..N-1] integers
                                {
                                    if (bitmask2[o]) {
                                        if(kItemIndex[o] == fMap.value(target)) {
                                            skip = true;
                                            break;
                                        }
                                        k1Item.append(QString::number(kItemIndex[o]) + '|');
                                    }
                                }
                                if(!skip) {
                                    k1Item.chop(1);
                                    if(!(prevItemCount->contains(k1Item))) {
                                        add = false;
                                        break;
                                    }
                                }
                            } while (prev_permutation(bitmask2.begin(), bitmask2.end()));
                        }

                        if(add) {
                            QString kItem;
                            for(int j=0; j<kItemIndex.count(); j++) {
                                kItem.append(QString::number(kItemIndex[j]) + '|');
                            }
                            kItem.chop(1);
                            if(itemCount->contains(kItem)) {
                                int iCount = itemCount->value(kItem);
                                itemCount->insert(kItem,iCount+1);
                            }
                            else {
                                itemCount->insert(kItem,1);
                            }
                        }
                    } while (prev_permutation(bitmask.begin(), bitmask.end()));
                    /* END */
#ifdef TEST
            }
#else
                }
            }
        }
    }
#endif
    return true;
}

bool iAGenerateRules(QHash<QString, int> *currItemCount, QHash<QString, int> *prevItemCount, QHash<QString, int> &fMap, QString target, int supp, double conf) {
    bool isFirst = true;
    QHashIterator<QString, int> it(*currItemCount);
    while (it.hasNext()) {
        it.next();
        QString itemSet = it.key();
        int itemSetCount = it.value();

        // remove item sets with support les than the minimum
        if(itemSetCount < supp) {
            currItemCount->remove(itemSet);
            continue;
        }

        // skip item sets that don't include the target from rule analysis
        QStringList items = itemSet.split("|");
        QString targetIndex = QString::number(fMap.value(target));
        if(!(items.contains(targetIndex))) {
            continue;
        }

        // evaluate for rule generation
        int index = items.indexOf(targetIndex);
        if(index == -1) {
            cout << "Couldn't find the target item index!\n";
            return false;
        }
        items.removeAt(index);
        QString antecedent = items.join("|");

        // skip this rule if antecedent is not frequent, i.e. not present in prevItemCount
        if(!(prevItemCount->contains(antecedent))) {
            continue;
        }

        // skip this rule if confidence is less than minimum
        int antecedentCount = prevItemCount->value(antecedent);
        // remove item sets with support les than the minimum
//        if(antecedentCount < supp) {
//            continue;
//        }

        double ruleConf = itemSetCount * 100.0 / antecedentCount;
        if(ruleConf < conf) {
            continue;
        }

        // Print the valid rule
        if(isFirst) {
            isFirst = false;
            cout << "Generating " << items.count()+1 << "-itemset rules..." << endl;
        }

        theSarf->out << target << " <- ";
        for(int i=0; i<items.count(); i++) {
            QString itemIndexS = items[i];
            int itemIndex = itemIndexS.toInt();
            QString item = fMap.key(itemIndex);
            theSarf->out << item << ' ';
        }

        theSarf->out << '(' << itemSetCount << ',' << ruleConf << ')' << endl;
    }
    return !isFirst;
}

bool dpDecisionTree(QHash<QString, quint8>& hash, QString morpheme_type) {
    DTNode* root = new DTNode();
    root->buildTree(hash,morpheme_type);
    QVector<QString> path;
    printPathsRecur(root,path);
    return true;
}

bool DTNode::buildTree(QHash<QString, quint8>& hash, QString morpheme_type) {
    if(this->isClass) {
        return true;
    }

    // save path feature values
    QVector<QString> bValues;
    DTNode* temp = this;
    while(temp->parent != NULL) {
        bValues.append(temp->bValue);
        temp = temp->parent;
    }

    // find feature with highest information gain
    // contains the feature to branch on
    QString feature;
    // contains the values of the feature to branch on
    QVector<QString> fValues;
    // flag to identify classes
    bool isClass = false;
    double accuracy = 0;
    // call method to iterate over the data set
    dTIterateDataSet(hash,bValues,fValues,feature,isClass,accuracy, morpheme_type);
    // add branch nodes for this feature and call buildTree on each
    if(isClass) {
        // We reached a class
        this->isClass = true;
        this->fValue = feature;
        this->accuracy = accuracy;
        return true;
    }
    else {
        // branch
        this->fValue = feature;
        for(int i=0; i<fValues.count(); i++) {
            DTNode* node = new DTNode();
            node->bValue = fValues[i];
            node->parent = this;
            this->nextHash.insert(fValues[i],node);
        }
    }
    QHashIterator<QString, DTNode*> i(this->nextHash);
     while (i.hasNext()) {
         i.next();
         bool good = i.value()->buildTree(hash, morpheme_type);
         if(!good) {
             return false;
         }
     }
     return true;
}

bool dTIterateDataSet(QHash<QString, quint8>& hash, QVector<QString>& pathFeatures, QVector<QString>& fValues, QString& feature, bool& isClass, double& accuracy, QString morpheme_type) {
    // Entries to keep track of total count of low, average, and high reduction tuples
    long cCount[3] = {0,0,0};
    // MultiHash to keep track of each feature and its values
    QMultiHash<QString, QString> fvHash;
    // Hash to keep track of each feature value's occurence in a high reduction tuple
    QHash<QString, long> vHHash;
    // Hash to keep track of each feature value's occurence in an average reduction tuple
    QHash<QString, long> vAHash;
    // Hash to keep track of each feature value's occurence in a low reduction tuple
    QHash<QString, long> vLHash;
    QHashIterator<QString, quint8> it(hash);
    while (it.hasNext()) {
        it.next();
        QString word = it.key();
        int count = 0;
        int letterCount = word.count();

        // Get the morphological solutions of the word
        WordAnalysis wa(&word,&count,morpheme_type);
        wa();

        // skip unambiguous words, i.e. single morphological solution
        if(wa.solutions.count() <= 1) {
            continue;
        }

        int** diacCMap = new int*[letterCount];
        for(int i=0; i<letterCount; i++) {
            diacCMap[i] = new int[5];
            for(int j=0; j< 5; j++) {
                diacCMap[i][j] = 0;
            }
        }

        if(!oneDiacConMap(diacCMap,wa)) {
            return false;
        }

        // Iterate over the morphological analyses of the word
        for(int i=0; i<wa.solutions.count(); i++) {
            const Solution& sol = wa.solutions.at(i);
            // skip solutions with '+' in stem POS or containing affixes
            if(!(sol.isValid) || sol.length == sol.vWord.count()) {
                //skippedM++;
                continue;
            }

            // Detach diacritics from raw_data and store in separate structure
            int letterIndex = 0;
            for(int j=1; j<sol.vWord.count(); j++) {
                QChar currentLetter= sol.vWord[j];
                if(isDiacritic(currentLetter)) {
                    int diacIndex = currentLetter.unicode() - fatha.unicode();
                    if(diacIndex < 0 || diacIndex > 4) {
                        continue;
                    }
                    // Here we have a transaction to process
                    QVector<QString> transaction;
                    // stem length
                    QString sl = "sl|" + QString::number(sol.stem_length);
                    transaction.append(sl);
                    // stem POS
                    QString s = "s|" + sol.stemPOS;
                    transaction.append(s);

                    if(morpheme_type == "A") {
                        // word length
                        QString wl = "wl|" + QString::number(sol.length);
                        transaction.append(wl);
                        // number of morphemes
                        QString nm = "nm|" + QString::number(sol.number_of_morphemes);
                        transaction.append(nm);

                        // prefixes and POS tags
                        for(int j=0; j<sol.prefixes.count(); j++) {
                            QString p = 'p' + QString::number(sol.prefixes.count()-j) + '|' + sol.prefixes[j];
                            transaction.append(p);
                        }
                        for(int j=0; j<sol.prefixPOSs.count(); j++) {
                            QString pp = "pp" + QString::number(sol.prefixPOSs.count()-j) + '|' + sol.prefixPOSs[j];
                            transaction.append(pp);
                        }

                        // suffixes and POS tags
                        for(int j=0; j<sol.suffixes.count(); j++) {
                            QString x = 'x' + QString::number(j+1) + '|' +sol.suffixes[j];
                            transaction.append(x);
                        }
                        for(int j=0; j<sol.suffixPOSs.count(); j++) {
                            QString xp = "xp" + QString::number(j+1) + '|' + sol.suffixPOSs[j];
                            transaction.append(xp);
                        }
                    }

                    // diacritic added
                    QString d = "d|";
                    d.append(currentLetter);
                    transaction.append(d);

                    // diacritic position
                    QString diacritic_position;
                    if(morpheme_type == "A") {
                        if((sol.prefix_length != 0) && (letterIndex == 0)) {
                            diacritic_position = "dp|prefixs";
                        }
                        else if((sol.prefix_length != 0) && (letterIndex > 0) && (letterIndex < (sol.prefix_length-1))) {
                            diacritic_position = "dp|prefixm";
                        }
                        else if((sol.prefix_length != 0) && (letterIndex == (sol.prefix_length-1))) {
                            diacritic_position = "dp|prefixe";
                        }
                        else if(letterIndex == sol.prefix_length) {
                            diacritic_position = "dp|stems";
                        }
                        else if((letterIndex > (sol.prefix_length)) && (letterIndex < (sol.prefix_length + sol.stem_length - 1))) {
                            diacritic_position = "dp|stemm";
                        }
                        else if(letterIndex == (sol.prefix_length + sol.stem_length - 1)) {
                            diacritic_position = "dp|steme";
                        }
                        else if((sol.suffix_length != 0) && (letterIndex == (sol.prefix_length + sol.stem_length))) {
                            diacritic_position = "dp|suffixs";
                        }
                        else if((sol.suffix_length != 0) && (letterIndex > (sol.prefix_length + sol.stem_length)) && (letterIndex < (sol.length - 1))) {
                            diacritic_position = "dp|suffixm";
                        }
                        else if((sol.suffix_length != 0) && (letterIndex == (sol.length -1))) {
                            diacritic_position = "dp|suffixe";
                        }
                        else {
                            cout << "Couldn't set diacritic position!" << endl;
                            return false;
                        }
                    }
                    else {
                        if(letterIndex == 0) {
                            diacritic_position = "dp|stems";
                        }
                        else if(letterIndex == sol.stem_length-1) {
                            diacritic_position = "dp|steme";
                        }
                        else {
                            diacritic_position = "dp|stemm";
                        }
                    }

                    transaction.append(diacritic_position);

                    // calculate morpho. reduction and discretize
                    QString mReduction;
                    double morphoReduction = (wa.solutions.count()-diacCMap[letterIndex][diacIndex])/wa.solutions.count();
                    if( morphoReduction >= 0.5) {
                        // high reduction
                        mReduction = "mHIGH";
                    }
                    else if(morphoReduction >= 0.2) {
                        // average reduction
                        mReduction = "mAVRG";
                    }
                    else {
                        // low reduction
                        mReduction = "mLOW";
                    }

//                    // calculate vocalization reduction and discretize
//                    QString vReduction;
//                    double vocReduction = (voc.count()-oneDiacVoc)/voc.count();
//                    if(vocReduction >= 0.5) {
//                        vReduction = "vHIGH";
//                    }
//                    else if(vocReduction >= 0.2) {
//                        vReduction = "vAVRG";
//                    }
//                    else {
//                        vReduction = "vLOW";
//                    }

//                    if(fMap.contains(vReduction)) {
//                        transaction.append(fMap.value(vReduction));
//                    }
//                    else {
//                        int index = fMap.count();
//                        fMap.insert(vReduction,index);
//                        transaction.append(index);
//                    }

                    bool skip = false;
                    for(int j=0; j<pathFeatures.count(); j++) {
                        // get feature name and value of current feature on path
                        QString pFName = pathFeatures[j].split('|').at(0);
                        QString pFValue = pathFeatures[j].split('|').at(1);

                        for(int k=0; k<transaction.count(); k++) {
                            // get feature name and value of current tupple entries
                            QString tFName = transaction[k].split('|').at(0);
                            QString tFValue = transaction[k].split('|').at(1);

                            if(pFName == tFName) {
                                if(pFValue == tFValue) {
                                    // feature value is already on the path so exclude
                                    transaction.remove(k);
                                    break;
                                }
                                else {
                                    // feature value is not on current path so skip tupple
                                    skip = true;
                                    break;
                                }
                            }
                        }
                        if(skip) {
                            break;
                        }
                    }
                    if(skip) {
                        continue;
                    }

                    if(transaction.count() == 0) {
                        continue;
                    }

                    for(int j=0; j<transaction.count(); j++) {
                        QString tFName = transaction[j].split('|').at(0);
                        if(!(fvHash.contains(tFName,transaction[j]))) {
                            fvHash.insert(tFName,transaction[j]);
                        }
                    }

                    if(mReduction == "mLOW") {
                        cCount[0] = cCount[0]+1;
                    }
                    else if(mReduction == "mAVRG") {
                        cCount[1] = cCount[1]+1;
                    }
                    else if(mReduction == "mHIGH") {
                        cCount[2] = cCount[2]+1;
                    }
                    else {
                        cout << "Invalid class detected!\n";
                        return false;
                    }

                    for(int j=0; j<transaction.count(); j++) {
                        if(mReduction == "mLOW") {
                            if(vLHash.contains(transaction[j])) {
                                long tCount = vLHash.value(transaction[j]);
                                tCount++;
                                vLHash.insert(transaction[j],tCount);
                            }
                            else {
                                vLHash.insert(transaction[j],1);
                            }
                        }
                        else if(mReduction == "mAVRG") {
                            if(vAHash.contains(transaction[j])) {
                                long tCount = vAHash.value(transaction[j]);
                                tCount++;
                                vAHash.insert(transaction[j],tCount);
                            }
                            else {
                                vAHash.insert(transaction[j],1);
                            }
                        }
                        else if(mReduction == "mHIGH") {
                            if(vHHash.contains(transaction[j])) {
                                long tCount = vHHash.value(transaction[j]);
                                tCount++;
                                vHHash.insert(transaction[j],tCount);
                            }
                            else {
                                vHHash.insert(transaction[j],1);
                            }
                        }
                    }
                }
                else {
                    letterIndex++;
                }
            }
        }

        for(int i=0; i<letterCount; i++) {
            delete[] diacCMap[i];
        }
        delete[] diacCMap;
    }

    /** Calculate the gain for each feature and choose feature to branch on **/
    long double totalCount = cCount[0] + cCount[1] + cCount[2];

    // Case 1: All the tuples processed belong to the same class
    if(totalCount == cCount[0]) {
        isClass = true;
        accuracy = 1;
        feature = "mLOW";
        return true;
    }
    else if(totalCount == cCount[1]) {
        isClass = true;
        accuracy = 1;
        feature = "mAVRG";
        return true;
    }
    else if(totalCount == cCount[2]) {
        isClass = true;
        accuracy = 1;
        feature = "mHIGH";
        return true;
    }

    // Case 2: We don't have any more features to split on
    if(fvHash.isEmpty()) {
        if(cCount[0] >= cCount[1] && cCount[0] >= cCount[2]) {
            isClass = true;
            accuracy = cCount[0] / totalCount;
            feature = "mLOW";
            return true;
        }
        else if(cCount[1] >= cCount[0] && cCount[1] >= cCount[2]) {
            isClass = true;
            accuracy = cCount[1] / totalCount;
            feature = "mAVRG";
            return true;
        }
        else {
            isClass = true;
            accuracy = cCount[2] / totalCount;
            feature = "mHIGH";
            return true;
        }
    }

    // Case 3: We don't have any more samples to use for splitting

    // Calculate the information gain for each feature and choose the one with the highest value
    double info = information(cCount[0],cCount[1],cCount[2]);
    QList<QString> features = fvHash.uniqueKeys();
    double maxGain = -1;
    QString splitFeature;
    for(int i=0; i<features.count(); i++) {
        double infoF = 0;
        QList<QString> vals = fvHash.values(features[i]);
        for(int j=0; j<vals.count(); j++) {
            long vl;
            if(vLHash.contains(vals[j])) {
                vl = vLHash.value(vals[j]);
            }
            else {
                vl = 0;
            }

            long va;
            if(vAHash.contains(vals[j])) {
                va = vAHash.value(vals[j]);
            }
            else {
                va = 0;
            }

            long vh;
            if(vHHash.contains(vals[j])) {
                vh = vHHash.value(vals[j]);
            }
            else {
                vh = 0;
            }
            infoF += (vl+va+vh)/totalCount*information(vl,va,vh);
        }

        double fGain = info - infoF;
        if(fGain > maxGain) {
            maxGain = fGain;
            splitFeature = features[i];
        }
    }

    if(maxGain == -1) {
        cout << "Problem finding feature with maximum gain!!\n";
        return false;
    }

    feature = splitFeature;
    QList<QString> sfValues = fvHash.values(feature);
    for(int i=0; i<sfValues.count(); i++) {
        fValues.append(sfValues[i]);
    }
    return true;
}

double information(long class1, long class2, long class3) {
    long double total = class1 + class2 + class3;
    double v1 = 0;
    if(class1 != 0) {
        v1 = class1/total*log2(class1/total);
    }
    double v2 = 0;
    if(class2 != 0) {
        v2 = class2/total*log2(class2/total);
    }
    double v3 = 0;
    if(class3 != 0) {
        v3 = class3/total*log2(class3/total);
    }
    return (-v1-v2-v3);
}

bool oneDiacConsistency(QString& oneDiacWord, QString& word2, int diacLetterPos) {
    Diacritics d1,d2;
    QChar diac = oneDiacWord[diacLetterPos+1];
    if(isDiacritic(diac)) {
        d1.append(diac);
    }
    else {
        cout << "Problem in oneDiacConsistency function!\n";
        return false;
    }

    int letterPos = -1, i = 0;
    while(letterPos != diacLetterPos) {
        if(!isDiacritic(word2[i])) {
            letterPos++;
        }
        i++;
    }

    while(isDiacritic(word2[i]) && i < word2.count()) {
        d2.append(word2[i]);
        i++;
    }

    return d1.isConsistent(d2);
}

bool oneDiacConMap(int** diacCMap, WordAnalysis& wa) {

    for(int i=0; i<wa.solutions.count(); i++) {
        int letterIndex = 0;
        const Solution& sol = wa.solutions.at(i);
        QString raw_data = sol.vWord;
        // skip solutions with '+' in stem POS or containing affixes
        if(!(sol.isValid)) {
            //skippedM++;
            continue;
        }

        int rdCount = raw_data.count();
        bool noDiac = true;
        for(int j=1; j<rdCount; j++) {
            if(isDiacritic(raw_data[j])) {
                noDiac = false;
                int code;
                int code1 = raw_data[j].unicode() - fatha.unicode();
                if(j+1 < rdCount && isDiacritic(raw_data[j+1])) {
                    j++;
                    int code2 = raw_data[j].unicode() - fatha.unicode();
                    if(code1 != 3 || code2 == 3 || (j+1 < rdCount && isDiacritic(raw_data[j+1]))) {
                        theSarf->out << "Weird diacritics!!: " << raw_data << ' ' << code1 << ' ' << code2 << endl;
                        continue;
                    }
                    code  = code1 + code2 + 2;
                }
                switch(code) {
                case 0:
                    diacCMap[letterIndex][0] += 1;
                    break;
                case 1:
                    diacCMap[letterIndex][1] += 1;
                    break;
                case 2:
                    diacCMap[letterIndex][2] += 1;
                    break;
                case 3:
                    diacCMap[letterIndex][0] += 1;
                    diacCMap[letterIndex][1] += 1;
                    diacCMap[letterIndex][2] += 1;
                    diacCMap[letterIndex][3] += 1;
                    break;
                case 4:
                    diacCMap[letterIndex][4] += 1;
                    break;
                case 5:
                    if(code1 == 5) {
                        continue;
                    }
                    diacCMap[letterIndex][0] += 1;
                    diacCMap[letterIndex][3] += 1;
                    break;
                case 6:
                    diacCMap[letterIndex][1] += 1;
                    diacCMap[letterIndex][3] += 1;
                    break;
                case 7:
                    diacCMap[letterIndex][2] += 1;
                    diacCMap[letterIndex][3] += 1;
                    break;
                case 34:
                    diacCMap[letterIndex][0] += 1;
                    break;
                case 62997:
                    diacCMap[letterIndex][0] += 1;
                    diacCMap[letterIndex][3] += 1;
                    break;
                default:
                    theSarf->out << "Invalid code!!: " << raw_data << ' ' << code  << ' ' << code1 << endl;
                }
            }
            else {
                if(noDiac) {
                    diacCMap[letterIndex][0] += 1;
                    diacCMap[letterIndex][1] += 1;
                    diacCMap[letterIndex][2] += 1;
                    diacCMap[letterIndex][3] += 1;
                    diacCMap[letterIndex][4] += 1;
                }
                letterIndex++;
                noDiac = true;
            }
        }
    }
    return true;
}

void printPathsRecur(DTNode* node, QVector<QString>& path)
{
  if (node==NULL)
    return;

  /* append this node to the path array */
  if(!(node->bValue.isEmpty())) {
    path.append(node->bValue);
  }

  /* it's a leaf, so print the path that led to here  */
  if (node->isClass)
  {
      theSarf->out << node->fValue << " <- ";
      for (int i=0; i<path.count(); i++)
      {
        theSarf->out << ' ' << path[i];
      }
      theSarf->out << " (" << node->accuracy << ")\n";
  }
  else
  {
    /* otherwise try both subtrees */
    QHashIterator<QString, DTNode*> i(node->nextHash);
     while (i.hasNext()) {
         i.next();
         printPathsRecur(i.value(), path);
         path.pop_back();
     }
  }
}

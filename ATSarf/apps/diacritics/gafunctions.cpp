extern "C" {
#include "apriori.h"
}
#include <algorithm>
#include <string>
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "gafunctions.h"

bool dgGeneticAlgorithm(QHash<QString, qint8>& hash, QStringList& listStemPOS) {
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

bool evaluation(QHash<QString, qint8>& hash, QVector<QVector<int> >& population, QVector<double>& fitness, QStringList& stemPOSList) {

    // Stores the count of high reduction 1-diacritic solutions matching the individuals
    QVector<int> hRedSol(population.count());
    // Stores the count of 1-riacritic solutions matching the individuals
    QVector<int> totalMSol(population.count());

    // reinitialize fitness values
    for(int i=0; i<fitness.count(); i++) {
        fitness[i] = 0;
    }

    // Iterate over all the Arabic stems in the lexicon
    QHashIterator<QString, qint8> it(hash);
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

bool dgApriori(QHash<QString, qint8>& hash) {

    FILE *fp = NULL;
    fp = apriori_initialize("transaction.tab", "rules");
    if(fp == NULL) {
        cout << "couldn't initialize apriori!!\n";
        return 0;
    }

    //qDebug() << "number of Arabic stems is: " << hash.count() << endl;
    //long long tcount = 0;
    //int skippedW = 0;
    //int skippedM = 0;
    //int skippedMD = 0;
    // Iterate over all the Arabic stems in the lexicon
    QHashIterator<QString, qint8> it(hash);
    while (it.hasNext()) {
        it.next();
        QString word = it.key();
        int count = 0;

        // Get the morphological solutions of the word
        WordAnalysis wa(&word,&count);
        wa();
        //tcount += wa.getAmbiguity();

        // skip unambiguous words, i.e. single morphological solution
        if(wa.solutions.count() <= 1) {
            //skippedW++;
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
                        if(equal(oneDiacWord,wa.solutions.at(l).vWord)) {
                            oneDiacSols++;
                        }
                    }

                    // Count consistent vocalizations
                    double oneDiacVoc = 0;
                    QSet<QString> voc;
                    for(int l=0; l<wa.solutions.count(); l++) {
                        if(!(voc.contains(wa.solutions.at(l).vWord))) {
                            voc.insert(wa.solutions.at(l).vWord);
                            if(equal(oneDiacWord,wa.solutions.at(l).vWord)) {
                                oneDiacVoc++;
                            }
                        }
                    }

                    // Count consistent glosses
//                    double oneDiacG = 0;
//                    QSet<QString> glosses;
//                    for(int l=0; l<wa.solutions.count(); l++) {
//                        if(!(glosses.contains(wa.solutions.at(l).vWord))) {
//                            glosses.insert(wa.solutions.at(l).vWord);
//                            if(oneDiacWord == wa.solutions.at(l).vWord) {
//                                oneDiacG++;
//                            }
//                        }
//                    }

                    QString transaction;
                    // word length
                    transaction.append("wl" + QString::number(sol.length) + ' ');
                    // number of morphemes
                    transaction.append("nm" + QString::number(sol.number_of_morphemes) + ' ');
                    // stem length
                    transaction.append("sl" + QString::number(sol.stem_length) + ' ');
                    // stem POS
                    transaction.append('s' + sol.stemPOS + ' ');

                    // prefixes and POS tags
                    for(int j=0; j<sol.prefixes.count(); j++) {
                        transaction.append( 'p' + QString::number(sol.prefixes.count()-j) + sol.prefixes[j] + ' ');
                    }
                    for(int j=0; j<sol.prefixPOSs.count(); j++) {
                        transaction.append( "pp" + QString::number(sol.prefixPOSs.count()-j) + sol.prefixPOSs[j] + ' ');
                    }

                    // suffixes and POS tags
                    for(int j=0; j<sol.suffixes.count(); j++) {
                        transaction.append( 'x' + QString::number(j+1) + sol.suffixes[j] + ' ');
                    }
                    for(int j=0; j<sol.suffixPOSs.count(); j++) {
                        transaction.append( "xp" + QString::number(j+1) + sol.suffixPOSs[j] + ' ');
                    }

                    // diacritic added
                    transaction.append( 'd' + wordDiacritics[m][n] + ' ');

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
                    transaction.append(diacritic_position + ' ');

                    // calculate morpho. reduction and discretize
                    double morphoReduction = (wa.solutions.count()-oneDiacSols)/wa.solutions.count();
                    if( morphoReduction >= 0.5) {
                        // high reduction
                        transaction.append("mHIGH ");
                    }
                    else if(morphoReduction >= 0.2) {
                        // average reduction
                        transaction.append("mAVRG ");
                    }
                    else {
                        // low reduction
                        transaction.append("mLOW ");
                    }

                    // calculate vocalization reduction and discretize
                    double vocReduction = (voc.count()-oneDiacVoc)/voc.count();
                    if(vocReduction >= 0.5) {
                        transaction.append("vHIGH");
                    }
                    else if(vocReduction >= 0.2) {
                        transaction.append("vAVRG");
                    }
                    else {
                        transaction.append("vLOW");
                    }

                    const char * _transaction = transaction.toStdString().c_str();
                    fprintf(fp, "%s", _transaction);
                    //printf("%s\n", _transaction);
                    rewind(fp);
                    int k = addTransaction();
                    if(k<0) {
                        cout << "couldn't add a transaction!!\n";
                        return false;
                    }
                    rewind(fp);
                    ftruncate(fileno(fp), 0);
                }
            }
        }
    }
    cout << "Read all the transactions...\n";
    hash.clear();
    cout << "Starting apriori algorithm...\n";
    int k = apriori_start("output.rules",80,-1);
    if(k<0) {
        cout << "couldn't run algorithm!!\n";
        return false;
    }
    //qDebug() << "Number of morphological analyses is " << tcount << endl;
    //qDebug() << "Number of unambiguous stems is " << skippedW << endl;
    //qDebug() << "Number of skipped morphological analyses is " << skippedM << endl;
    //qDebug() << "Number of skipped morphological analyses with no diacritics is " << skippedMD << endl;
    return true;
}

bool dpIterApriori(QHash<QString, int>& hash, QString target, int supp, double conf) {

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
    iterateDataSet(hash,currItemCount,prevItemCount,fMap,k,target);
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
        iterateDataSet(hash,currItemCount,prevItemCount,fMap,k,target);
        isDone = currItemCount->isEmpty();
        if(!isDone) {
            isDone = !(generateRules(currItemCount,prevItemCount,fMap,target,supp,conf));
        }
        k++;
    }
    return true;
}

bool iterateDataSet(QHash<QString, int>& hash, QHash<QString, int> *itemCount, QHash<QString, int> *prevItemCount, QHash<QString, int> &fMap, int k, QString target) {
#ifndef TEST
    QHashIterator<QString, int> it(hash);
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
                        if(equal(oneDiacWord,wa.solutions.at(l).vWord)) {
                            oneDiacSols++;
                        }
                    }

                    // Count consistent vocalizations
                    double oneDiacVoc = 0;
                    QSet<QString> voc;
                    for(int l=0; l<wa.solutions.count(); l++) {
                        if(!(voc.contains(wa.solutions.at(l).vWord))) {
                            voc.insert(wa.solutions.at(l).vWord);
                            if(equal(oneDiacWord,wa.solutions.at(l).vWord)) {
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
                        mReduction = "mLOW ";
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

bool generateRules(QHash<QString, int> *currItemCount, QHash<QString, int> *prevItemCount, QHash<QString, int> &fMap, QString target, int supp, double conf) {
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

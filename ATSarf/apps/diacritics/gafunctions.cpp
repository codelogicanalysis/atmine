extern "C" {
#include "apriori.h"
}
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
            for(int i=1; i<sol.stem.count(); i++) {
                QChar currentLetter= sol.stem[i];
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
                        QString oneDiacWord = removeDiacritics(sol.stem);
                        oneDiacWord.insert(1,wordDiacritics.first().at(k));

                        // Iterate over the morpho. solutions and count consistent solutions
                        double oneDiacSols = 0;
                        for(int m=0; m<wa.solutions.count(); m++) {
                            if(equal(oneDiacWord,wa.solutions.at(m).stem)) {
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
                            QString oneDiacWord = removeDiacritics(sol.stem);
                            oneDiacWord.insert(m+1,wordDiacritics[m][n]);

                            // Iterate over the morpho. solutions and count consistent solutions
                            double oneDiacSols = 0;
                            for(int l=0; l<wa.solutions.count(); l++) {
                                if(equal(oneDiacWord,wa.solutions.at(l).stem)) {
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
                        QString oneDiacWord = removeDiacritics(sol.stem);
                        oneDiacWord.append(wordDiacritics.last().at(k));

                        // Iterate over the morpho. solutions and count consistent solutions
                        double oneDiacSols = 0;
                        for(int m=0; m<wa.solutions.count(); m++) {
                            if(equal(oneDiacWord,wa.solutions.at(m).stem)) {
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

    qDebug() << "number of Arabic stems is: " << hash.count() << endl;
    long long tcount = 0;
    int skippedW = 0;
    int skippedM = 0;
    int skippedMD = 0;
    // Iterate over all the Arabic stems in the lexicon
    QHashIterator<QString, qint8> it(hash);
    while (it.hasNext()) {
        it.next();
        QString word = it.key();
        int count = 0;

        // Get the morphological solutions of the word
        WordAnalysis wa(&word,&count);
        wa();
        tcount += wa.getAmbiguity();

        // skip unambiguous words, i.e. single morphological solution
        if(wa.solutions.count() <= 1) {
            skippedW++;
            continue;
        }

        // Iterate over the morphological analyses of the word
        for(int i=0; i<wa.solutions.count(); i++) {
            const Solution& sol = wa.solutions.at(i);
            // skip solutions with '+' in stem POS or containing affixes
            if(!(sol.isValid)) {
                skippedM++;
                continue;
            }

            // Detach diacritics from raw_data and store in separate structure
            int diacritic_Counter = 0;
            QVector<QVector<QChar> > wordDiacritics(sol.length);
            int letterIndex = 0;
            for(int j=1; j<sol.stem.count(); j++) {
                QChar currentLetter= sol.stem[j];
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
                skippedMD++;
                continue;
            }

            // Process diacritics at first, middle, and end of the stem
            for(int m=0; m< wordDiacritics.count(); m++) {
                for(int n=0; n< wordDiacritics.at(m).count(); n++) {
                    // construct the one diacritic stem
                    QString oneDiacWord = removeDiacritics(sol.stem);
                    oneDiacWord.insert(m+1,wordDiacritics[m][n]);

                    // Iterate over the morpho. solutions and count consistent solutions
                    double oneDiacSols = 0;
                    for(int l=0; l<wa.solutions.count(); l++) {
                        if(equal(oneDiacWord,wa.solutions.at(l).stem)) {
                            oneDiacSols++;
                        }
                    }

                    QString transaction;
                    transaction.append(QString::number(sol.length) + ' ' + sol.stemPOS + ' ');
                    if(m == 0) {
                        transaction.append("stems ");
                    }
                    else if(m == wordDiacritics.count()-1) {
                        transaction.append("steme ");
                    }
                    else {
                        transaction.append("stemm ");
                    }

                    // calculate morpho. reduction and discretize
                    if((wa.solutions.count()-oneDiacSols)/wa.solutions.count() >= 0.5) {
                        // high reduction
                        transaction.append("HIGH");
                    }
                    else if((wa.solutions.count()-oneDiacSols)/wa.solutions.count() >= 0.2) {
                        // average reduction
                        transaction.append("AVRG");
                    }
                    else {
                        // low reduction
                        transaction.append("LOW");
                    }
                    const char * _transaction = transaction.toStdString().c_str();
                    fprintf(fp, _transaction);
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
    int k = apriori_start("stem.rules",80,-1);
    if(k<0) {
        cout << "couldn't run algorithm!!\n";
        return false;
    }
    qDebug() << "Number of morphological analyses is " << tcount << endl;
    qDebug() << "Number of unambiguous stems is " << skippedW << endl;
    qDebug() << "Number of skipped morphological analyses is " << skippedM << endl;
    qDebug() << "Number of skipped morphological analyses with no diacritics is " << skippedMD << endl;
    return true;
}

#ifndef GAEVALUATION_H
#define GAEVALUATION_H

#include <QHash>
#include <stdlib.h>
#include <time.h>
#include "wordanalysis.h"

#define  NUM_OF_FEATURES 57
#define  NUM_OF_SOLUTIONS 6
#define STEM_LENGTH 16
#define STEM_POS 38
#define DIAC_POS 3

bool evaluation(QHash<QString, qint8>& hash, QVector<QVector<int> >& population, QVector<double>& fitness, QStringList& stemPOSList) {

    // Stores the count of high reduction 1-diacritic solutions matching the individuals
    QVector<int> hRedSol(NUM_OF_SOLUTIONS);
    // Stores the count of 1-riacritic solutions matching the individuals
    QVector<int> totalMSol(NUM_OF_SOLUTIONS);

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

    for(int i=0; i<NUM_OF_SOLUTIONS; i++) {
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
        int bcValue = rand() % NUM_OF_SOLUTIONS;
        for(int i=0; i<population.at(bcValue).count(); i++) {
            parents[0][i] = population[bcValue][i];
        }
        parent1Index = bcValue;
        bcValue = rand() % NUM_OF_SOLUTIONS;
        for(int i=0; i<population.at(bcValue).count(); i++) {
            parents[1][i] = population[bcValue][i];
        }
        parent2Index = bcValue;
        return true;
    }

    double low = 0;
    double high = 0;
    for(int i=0; i<2; i++) {
        double bcValue = rand() / (float)(RAND_MAX+1) * totalScore;
        for(int j=0; j<NUM_OF_SOLUTIONS; j++) {
            high += (fitness.at(j)*100);
            if(bcValue >= low && bcValue < high) {
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

#endif // GAEVALUATION_H

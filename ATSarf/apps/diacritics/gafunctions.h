#ifndef GAEVALUATION_H
#define GAEVALUATION_H

#include <QHash>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "wordanalysis.h"

#define  NUM_OF_FEATURES 57
#define  NUM_OF_SOLUTIONS 6
#define STEM_LENGTH 16
#define STEM_POS 38
#define DIAC_POS 3

bool dgGeneticAlgorithm(QHash<QString, qint8>& hash, QStringList& listStemPOS);

bool evaluation(QHash<QString, qint8>& hash, QVector<QVector<int> >& population, QVector<double>& fitness, QStringList& stemPOSList);

bool selection(QVector<QVector<int> >& parents, QVector<QVector<int> >& population, QVector<double>& fitness, int& parent1Index, int& parent2Index);

bool crossover(QVector<QVector<int> >& parents, QVector<int>& child);

bool mutation(QVector<int>& individual);

bool dgApriori(QHash<QString, qint8>& hash);

#endif // GAEVALUATION_H

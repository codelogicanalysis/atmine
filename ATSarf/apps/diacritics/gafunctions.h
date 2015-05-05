#ifndef GAEVALUATION_H
#define GAEVALUATION_H

#include <QHash>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <QStack>
#include <cmath>
#include "wordanalysis.h"

#define  NUM_OF_FEATURES 57
#define  NUM_OF_SOLUTIONS 6
#define STEM_LENGTH 16
#define STEM_POS 38
#define DIAC_POS 3
//#define TEST

class DTNode {
public:
    DTNode() {
        fValue = "";
        bValue = "";
        isClass = false;
        parent = NULL;
    }
    bool buildTree(QHash<QString, int>& hash, QString morpheme_type);

    QHash<QString,DTNode*> nextHash;
    DTNode* parent;
    bool isClass;
    double accuracy;
    // Branch feature value from parent
    QString bValue;
    // next feature branch or class
    QString fValue;
};

bool dgGeneticAlgorithm(QHash<QString, qint8>& hash, QStringList& listStemPOS);

bool evaluation(QHash<QString, qint8>& hash, QVector<QVector<int> >& population, QVector<double>& fitness, QStringList& stemPOSList);

bool selection(QVector<QVector<int> >& parents, QVector<QVector<int> >& population, QVector<double>& fitness, int& parent1Index, int& parent2Index);

bool crossover(QVector<QVector<int> >& parents, QVector<int>& child);

bool mutation(QVector<int>& individual);

bool dgApriori(QHash<QString, qint8>& hash);

bool dpIterApriori(QHash<QString, int>& hash, QString target, int supp, double conf);

bool iAIterateDataSet(QHash<QString, int>& hash, QHash<QString, int> *itemCount, QHash<QString, int> *prevItemCount, QHash<QString, int> &fMap, int k, QString target);

bool iAGenerateRules(QHash<QString, int> *currItemCount, QHash<QString, int> *prevItemCount, QHash<QString, int> &fMap, QString target, int supp, double conf);

bool dpDecisionTree(QHash<QString, int>& hash, QString morpheme_type);

bool dTIterateDataSet(QHash<QString, int>& hash, QVector<QString>& pathFeatures, QVector<QString>& fValues, QString& feature, bool& isClass, double &accuracy, QString morpheme_type);

double information(long, long, long);

bool oneDiacConsistency(QString& oneDiacWord, QString& word2, int diacLetterPos);

void printPathsRecur(DTNode* node, QVector<QString>& path);
#endif // GAEVALUATION_H

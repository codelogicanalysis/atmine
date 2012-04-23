#ifndef DECISIONTREEREGRESSION_H
#define DECISIONTREEREGRESSION_H

#include <QList>
#include <QString>
#include "Tree.h"
#include "ATMProgressIFC.h"
#include "distinguishingLargeFileIterator.h"

typedef double RegressionNodeType;
typedef QString RegressionEdgeType;
typedef Node<RegressionNodeType,RegressionEdgeType> RegressionNode;
typedef Tree<RegressionNodeType,RegressionEdgeType> RegressionTree;

class DecisionTreeRegression {
private:
	QString fileName;
	QList<int> featureColumns;
	int weightColumn;
	int targetColumn;
	int devThreshold;
private:
	RegressionNode * buildTreeNode(QList<int> workingfeatureColumns,ConditionMap map,ATMProgressIFC * prg);
public:
	DecisionTreeRegression(QString fileName, QList<int> featureColumns, int weightColumn, int targetColumn, int threshold=0.1);
	RegressionTree * buildTree(ATMProgressIFC * prg);
};

#endif // DECISIONTREEREGRESSION_H

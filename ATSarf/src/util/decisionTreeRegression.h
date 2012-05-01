#ifndef DECISIONTREEREGRESSION_H
#define DECISIONTREEREGRESSION_H

#include <QList>
#include <QString>
#include "Tree.h"
#include "ATMProgressIFC.h"
#include "distinguishingLargeFileIterator.h"

#define COPY_NEEDED_TO_TEMPORARY_FILE

typedef QString RegressionNodeType;
typedef QString RegressionEdgeType;
typedef Node<RegressionNodeType,RegressionEdgeType> RegressionNode;
typedef Tree<RegressionNodeType,RegressionEdgeType> RegressionTree;

class DecisionTreeRegression {
public:
	class TerminationRule {
	public:
		int minInstances;
		double minStandardDevPercentage;
		TerminationRule(int instances=3,double devPercentage=0.05): minInstances(instances), minStandardDevPercentage(devPercentage) {}
	};
private:
	QString fileName;
	QList<int> featureColumns;
	QStringList columnNames;
	int weightColumn;
	int targetColumn;
	int itrCnt;

	//used for termination
	TerminationRule terminationRule;
	double terminationDevThreshold;
private:
	void initialize(QString fileName, QList<int> & featureColumns, int weightColumn, int targetColumn,ATMProgressIFC * prg);
	RegressionNode * buildTreeNode(QList<int> workingfeatureColumns,ConditionMap map,ATMProgressIFC * prg);
public:
	DecisionTreeRegression(QString fileName, QList<int> featureColumns, int targetColumn, int weightColumn, TerminationRule rule,ATMProgressIFC * prg);
	DecisionTreeRegression(QString fileName, QList<int> featureColumns, int targetColumn, int weightColumn, ATMProgressIFC * prg);
	RegressionTree * buildTree(ATMProgressIFC * prg);
	~DecisionTreeRegression();
};



#endif // DECISIONTREEREGRESSION_H

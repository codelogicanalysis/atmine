#include <assert.h>
#include "decisionTreeRegression.h"
#include "distinguishingLargeFileIterator.h"
#include "iterativeMathFunctions.h"

DecisionTreeRegression::DecisionTreeRegression(QString fileName, QList<int> featureColumns, int weightColumn, int targetColumn,int devThreshold){
	this->fileName=fileName;
	this->featureColumns=featureColumns;
	this->weightColumn=weightColumn;
	this->targetColumn=targetColumn;
	this->devThreshold=devThreshold;
}

RegressionNode * DecisionTreeRegression::buildTreeNode(QList<int> workingfeatureColumns,ConditionMap map,ATMProgressIFC * prg) {
	if (workingfeatureColumns.size()>0) {
		int indexBest=0;
		double bestReduction=0;
		QStringList values;
		QList<double> means,deviations;
		for (int i=0;i<workingfeatureColumns.size();i++) {
			DistinguishingLargeFileIterator itr(fileName,targetColumn,weightColumn,map,workingfeatureColumns[i],prg);
			double reduction=IterativeMathFunctions::weightedStandardDeviationReduction(itr,values,means,deviations);
			if (reduction>bestReduction) {
				indexBest=i;
				bestReduction=reduction;
			}
		}
		int bestCol=workingfeatureColumns[indexBest];
		workingfeatureColumns.removeAt(indexBest);
		RegressionNode * node=new RegressionNode(bestCol);
		for (int i=0;i<values.size();i++) {
			QString val=values[i];
			map[bestCol]=val;
			RegressionNode * child;
			if (deviations[i]<devThreshold || workingfeatureColumns.size()==0)
				child=new RegressionNode(means[i]);
			else
				child=buildTreeNode(workingfeatureColumns,map,prg);
			node->addNode(val,child);
		}
		return node;

	} else {
		assert(false);
		return NULL; //must not reach here
	}
}

RegressionTree * DecisionTreeRegression::buildTree(ATMProgressIFC * prg) {
	ConditionMap map;
	QList<int> workingfeatureColumns=featureColumns;
	RegressionNode * node=buildTreeNode(workingfeatureColumns,map,prg);
	return new RegressionTree(node);
}

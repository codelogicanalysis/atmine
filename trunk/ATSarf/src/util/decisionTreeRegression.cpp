#include <assert.h>
#include <QFile>
#include <QTextStream>
#include "logger.h"
#include "decisionTreeRegression.h"
#include "distinguishingLargeFileIterator.h"
#include "iterativeMathFunctions.h"


void DecisionTreeRegression::initialize(QString fileName, QList<int> & featureColumns, int targetColumn, int weightColumn,ATMProgressIFC * prg){
#ifdef COPY_NEEDED_TO_TEMPORARY_FILE
	QString tempFileName=fileName+".temp";
	QFile temp(tempFileName);
	temp.remove();
	if (!temp.open(QIODevice::WriteOnly)) {
		error << "Unable to open temporary file '"<<tempFileName<<"'\n";
		return;
	}
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly)) {
		error << "Unable to open file '"<<fileName<<"'\n";
		return;
	}
	QTextStream tempStream(&temp), fileStream(&file) ;
	tempStream.setCodec("utf-8");
	fileStream.setCodec("utf-8");
	int size=file.size();
	int pos=0;
	if (prg!=NULL)
		prg->setCurrentAction("Writing Needed Info to temp file");
	while (!file.atEnd()) {
		QString line=fileStream.readLine();
		QStringList entries=line.split('\t');
		pos+=LargeFileIterator::getSize(line);
		for (int i=0;i<featureColumns.size();i++) {
			int index=featureColumns[i];
			QString feature=entries[index];
			tempStream<<feature<<"\t";
		}
		if (weightColumn>=0) {
			QString weight=entries[weightColumn];
			tempStream<<weight<<"\t";
		}
		QString target=entries[targetColumn];
		tempStream<<target<<"\n";
		if (prg!=NULL)
			prg->report(((double)pos)/size*100+0.5);
	}
	temp.close();
	file.close();
	this->featureColumns.clear(); //not necessary, but additional safety
	for (int i=0;i<featureColumns.size();i++) {
		this->featureColumns.append(i);
	}
	if (weightColumn>=0)
		this->weightColumn=featureColumns.size();
	else
		this->weightColumn=-1;
	targetColumn=featureColumns.size()+1;
	this->fileName=tempFileName;
#else
	this->fileName=fileName;
	this->featureColumns=featureColumns;
	this->weightColumn=weightColumn;
	this->targetColumn=targetColumn;
#endif
}

DecisionTreeRegression::~DecisionTreeRegression() {
#ifdef COPY_NEEDED_TO_TEMPORARY_FILE
	if (!QFile::remove(fileName)) {
		error << "Unable to delete temporary file '"<<fileName<<"'\n";
	}
#endif
}

DecisionTreeRegression::DecisionTreeRegression(QString fileName, QList<int> featureColumns, int targetColumn, int weightColumn,ATMProgressIFC * prg){
	initialize(fileName,featureColumns,weightColumn,targetColumn,prg);
}

DecisionTreeRegression::DecisionTreeRegression(QString fileName, QList<int> featureColumns, int targetColumn, int weightColumn, TerminationRule rule,ATMProgressIFC * prg){
	initialize(fileName,featureColumns,weightColumn,targetColumn,prg);
	this->terminationRule=rule;
}


RegressionNode * DecisionTreeRegression::buildTreeNode(QList<int> workingfeatureColumns,ConditionMap map,ATMProgressIFC * prg) {
	if (workingfeatureColumns.size()>0) {
		int indexBest=0;
		double bestReduction=0;
		QStringList bestValues;
		ItDevList bestDevs;
		for (int i=0;i<workingfeatureColumns.size();i++) {
			itrCnt++;
			DistinguishingLargeFileIterator itr(fileName,targetColumn,weightColumn,map,workingfeatureColumns[i],prg);
			if (columnNames.size()==0)
				columnNames=itr.getColumnNames();
			prg->setCurrentAction(QString("Iterator %1 [%2]").arg(itrCnt).arg(columnNames[workingfeatureColumns[i]]));
			QStringList values;
			ItDevList devs;
			ItDev totalDev;
			double reduction=IterativeMathFunctions::weightedStandardDeviationReduction(itr,values,devs,totalDev);
			if (workingfeatureColumns.size()==featureColumns.size() && i==0) //update once in first call
				terminationDevThreshold=terminationRule.minStandardDevPercentage*(totalDev.getValue());
			if (reduction>bestReduction) {
				bestDevs=devs;
				bestValues=values;
				indexBest=i;
				bestReduction=reduction;
			}
		}
		if (bestValues.size()>0) {
			int bestCol=workingfeatureColumns[indexBest];
			workingfeatureColumns.removeAt(indexBest);
			QString bestColumnName=columnNames[bestCol];
			RegressionNode * node=new RegressionNode(bestColumnName);
			for (int i=0;i<bestValues.size();i++) {
				QString val=bestValues[i];
				map[bestCol]=val;
				RegressionNode * child=NULL;
				double stDev=bestDevs[i].getValue();
				double numInstances=bestDevs[i].getInstancesNum();
				if (!(stDev<terminationDevThreshold || workingfeatureColumns.size()==0 || numInstances<terminationRule.minInstances)) {
					child=buildTreeNode(workingfeatureColumns,map,prg);
				}
				if (child==NULL) {
					double mean=bestDevs[i].getWeightedMean();
					child=new RegressionNode(QString("%1").arg(mean));
				}
				node->addNode(val,child);
			}
			return node;
		} else {
			return NULL;
		}
	} else {
		assert(false);
		return NULL; //must not reach here
	}
}

RegressionTree * DecisionTreeRegression::buildTree(ATMProgressIFC * prg) {
	columnNames.clear();
	ConditionMap map;
	itrCnt=0;
	QList<int> workingfeatureColumns=featureColumns;
	RegressionNode * node=buildTreeNode(workingfeatureColumns,map,prg);
	return new RegressionTree(node);
}

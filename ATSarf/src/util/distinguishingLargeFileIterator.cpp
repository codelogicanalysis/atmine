#include <QStringList>
#include "distinguishingLargeFileIterator.h"

void DistinguishingLargeFileIterator::extractAdditionalInfo(const QStringList & list) {
	currDistiguishingValue=list.at(distinguishingColumn);
}

DistinguishingLargeFileIterator::DistinguishingLargeFileIterator(QString fileName, int valueColumn,
			int weightColumn, const ConditionMap & conditions, int distinguishingColumn, ATMProgressIFC *prg)
							:LargeFileIterator(fileName,valueColumn,weightColumn,conditions,prg) {
	this->distinguishingColumn=distinguishingColumn;
}

DistinguishingLargeFileIterator::DistinguishingLargeFileIterator(QString fileName, int valueColumn,
			int weightColumn,int distinguishingColumn, ATMProgressIFC *prg)
							:LargeFileIterator(fileName,valueColumn,weightColumn,prg) {
	this->distinguishingColumn=distinguishingColumn;
}


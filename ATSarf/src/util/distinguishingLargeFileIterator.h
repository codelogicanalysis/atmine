#ifndef DISTINGUISHINGLARGEFILEITERATOR_H
#define DISTINGUISHINGLARGEFILEITERATOR_H

#include "largeFileIterator.h"

class DistinguishingLargeFileIterator: public LargeFileIterator {
private:
	QString currDistiguishingValue;

	int distinguishingColumn;
protected:
	virtual void extractAdditionalInfo(const QStringList & list);
public:
	DistinguishingLargeFileIterator(QString fileName, int valueColumn, int weightColumn, const ConditionMap & conditions, int distinguishingColumn, ATMProgressIFC *prg=NULL);
	DistinguishingLargeFileIterator(QString fileName, int valueColumn, int weightColumn,int distinguishingColumn, ATMProgressIFC *prg=NULL);

	QString getDistinguishingValue() const {return currDistiguishingValue;}
};

#endif // DISTINGUISHINGLARGEFILEITERATOR_H

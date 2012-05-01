#ifndef LARGEFILEITERATOR_H
#define LARGEFILEITERATOR_H

#include <QString>
#include <QStringList>
#include <QMap>
#include "instanceIterator.h"
#include "ATMProgressIFC.h"


class QTextStream;
class QFile;

typedef QMap<int,QString> ConditionMap;

class LargeFileIterator : public  InstanceIterator {
private:
	//local values to be used by getInstance corresponding to current line
	double currValue;
	double currWeight;
	bool atEnd;

	//general fields
	int valueColumn;
	int weightColumn;
	ConditionMap conditions;

	//related to file
	QFile * inputFile;
	QTextStream * file;
	QStringList columns;
	long secondLinePos;

	ATMProgressIFC *prg; //TODO: not used yet
	long size;
	long pos;
private:
	void reportProgress();
	void processLine();
	void initialize(QString fileName, int valueColumn, int weightColumn, ATMProgressIFC *prg);
protected:
	virtual void extractAdditionalInfo(const QStringList & list);
public:
	LargeFileIterator(QString fileName, int valueColumn, int weightColumn, const ConditionMap & conditions, ATMProgressIFC *prg=NULL);
	LargeFileIterator(QString fileName, int valueColumn, int weightColumn,ATMProgressIFC *prg=NULL);
	virtual void start();
	virtual void next();
	virtual bool getInstance(double & value, double & weight) const;
	~LargeFileIterator();
	QStringList & getColumnNames() {return columns;}
	static long getSize(QString line);
};

#endif // LARGEFILEITERATOR_H

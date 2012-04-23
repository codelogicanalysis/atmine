#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <assert.h>
#include "largeFileIterator.h"
#include "logger.h"

LargeFileIterator::LargeFileIterator(QString fileName, int valueColumn, int weightColumn, const ConditionMap & conditions, ATMProgressIFC *prg){
	initialize(fileName,valueColumn,weightColumn,prg);
	this->conditions=conditions;
}

LargeFileIterator::LargeFileIterator(QString fileName, int valueColumn, int weightColumn, ATMProgressIFC *prg){
	initialize(fileName,valueColumn,weightColumn,prg);
}

void LargeFileIterator::initialize(QString fileName, int valueColumn, int weightColumn, ATMProgressIFC *prg) {
	inputFile=NULL;
	file=NULL;
	this->valueColumn=valueColumn;
	this->weightColumn=weightColumn;
	this->conditions=conditions;
	this->prg=prg;

	inputFile= new QFile(fileName);
	if (!inputFile->open(QIODevice::ReadOnly)) {
		out << "File not found\n";
		return;
	}
	file= new QTextStream(inputFile);
	file->setCodec("utf-8");
}


void LargeFileIterator::processLine() {
	atEnd=file->atEnd();
	if (!atEnd) {
		QString line=file->readLine();
		QStringList entries=line.split('\t');
		assert(entries.size()>valueColumn);
		currValue=entries.at(valueColumn).toDouble();
		currWeight=entries.at(weightColumn).toDouble();
		ConditionMap::iterator itr=conditions.begin();
		for (;itr!=conditions.end();itr++) {
			int col=itr.key();
			QString val=itr.value();
			QString curCondVal=entries.at(col);
			if (curCondVal!=val)
				return processLine();
		}
		extractAdditionalInfo(entries);
	}
}

void LargeFileIterator::extractAdditionalInfo(const QStringList &) { }

void LargeFileIterator::start() {
	file->seek(0);
	processLine();
}

void LargeFileIterator::next() {
	processLine();
}

bool LargeFileIterator::getInstance(double & value, double & weight) const{
	bool valid=!atEnd;
	if (valid) {
		value=currValue;
		weight=currWeight;
	}
	return valid;
}

LargeFileIterator::~LargeFileIterator() {
	if (inputFile!=NULL) {
		inputFile->close();
		delete inputFile;
	}
	if (file!=NULL)
		delete file;
}

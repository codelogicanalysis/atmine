#include <QFile>
#include <QDebug>
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

long LargeFileIterator::getSize(QString line){
	int s=line.toAscii().size()+1;
	return s;
}

void LargeFileIterator::reportProgress() {
	if (prg!=NULL)
		prg->report(((double)pos)/size*100+0.5);
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
	size=inputFile->size();
	pos=0;
	if (!file->atEnd()) {
		QString line=file->readLine();
		columns=line.split('\t');
		secondLinePos=file->pos();
		pos+=getSize(line);
	}
}


void LargeFileIterator::processLine() {
start: //to avoid deep recursion
	atEnd=file->atEnd();
	if (!atEnd) {
		QString line=file->readLine();
		pos+=getSize(line);
		QStringList entries=line.split('\t');
		//assert(entries.size()>valueColumn);
		if(entries.size()<valueColumn) {
			//qDebug()<<line;
			goto start;//return processLine();
		}
		currValue=entries.at(valueColumn).toDouble();
		if (weightColumn>=0)
			currWeight=entries.at(weightColumn).toDouble();
		else
			currWeight=1;
		ConditionMap::iterator itr=conditions.begin();
		for (;itr!=conditions.end();itr++) {
			int col=itr.key();
			QString val=itr.value();
			QString curCondVal=entries.at(col);
			if (curCondVal!=val)
				goto start;//return processLine();
		}
		extractAdditionalInfo(entries);
		reportProgress();
	}
}

void LargeFileIterator::extractAdditionalInfo(const QStringList &) { }

void LargeFileIterator::start() {
	file->seek(secondLinePos);
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
	reportProgress();
	if (inputFile!=NULL) {
		inputFile->close();
		delete inputFile;
	}
	if (file!=NULL)
		delete file;
}

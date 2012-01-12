#ifndef ATBEXPERIMENT_H
#define ATBEXPERIMENT_H

#include "atbExperiment.h"
#include <QString>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QRegExp>
#include <QStringList>
#include <assert.h>
#include "logger.h"
#include "stemmer.h"

#define TOKENIZE

class AtbStemmer: public Stemmer {
public:
	enum Status {EQ, D_GL, D_VOC, D_ALL};
private:
	const QStringList &voc;
	QStringList &gloss;
	const QStringList &pos;
	const QStringList &input_after;
	const QStringList &pos_after;
	bool found:1;
	Status stat:2;
	bool ignoreStem:1;
	QStringList similarGlosses;
	QStringList similarVoc;

	bool correctTokenize:1;
	bool skipTokenize:1;
	QStringList sarfTokenization;

private:
	Status updateSimilarFields(Status oldStat, Status currentStat, QString currGloss, QString currVoc, int old_pos);
	Status equal(int & index,minimal_item_info & item, Status currentStat, bool ignore=false);
public:
	AtbStemmer(QString & input,const QStringList &aVoc, QStringList &aGloss, const QStringList & aPOS, QStringList &aInputAfter, const QStringList & aPOSAfter, bool ignoreStem=false);
	virtual bool on_match();
	bool isFound() const {return found;}
	Status getStatus() const {return stat;}
	const QStringList & getMostSimilar() const {
		if (stat==D_VOC)
			return similarVoc;
		return similarGlosses;
	}
	bool isCorrectlyTokenized() const { return correctTokenize;}
	bool isSkipTokenize() const { return skipTokenize;}
	const QStringList & getTokenization() const { return sarfTokenization;}
};

int atb(QString inputString, ATMProgressIFC * prg);

#endif // ATBEXPERIMENT_H

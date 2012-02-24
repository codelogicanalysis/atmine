#ifndef AMBIGUITY_H
#define AMBIGUITY_H

#include "stemmer.h"

enum Ambiguity {Vocalization, Description, POS, Tokenization, Stem_Ambiguity, All_Ambiguity};
static const int ambiguitySize=(int)All_Ambiguity+1;

class AmbiguitySolution {
public:
	QString voc;
	QString desc;
	QString pos;
	bool featuresDefined:1;
	int stemStart:4;
	int suffStart:4;
	int stemIndex:4; //the morpheme number of the stem in the solution, so that we can identify it, can be deduced anyways from numPrefixes
	int numPrefixes:4;
	int numSuffixes:4;
	//TODO: other features
private:
	QString getFeatureIndex(const QString & feature, int index) const;
public:
	AmbiguitySolution(QString raw,QString des,QString POS);
	AmbiguitySolution(QString raw,QString des,QString POS,
			 int stemStart, int suffStart, int stemIndex, int numPrefixes, int numSuffixes);
	bool equal (const AmbiguitySolution & other, Ambiguity m) const;
private:
	int getTokenization() const;
};

typedef QList<AmbiguitySolution> AmbiguitySolutionList;
typedef QPair<QString, AmbiguitySolutionList > EntryAmbiguitySolutionList;

QString interpret(Ambiguity a);

item_types getDiacriticPosition(AmbiguitySolution & sol, int diacriticPos);

AmbiguitySolutionList getAmbiguityUnique(const AmbiguitySolutionList & list, Ambiguity m);

class AmbiguityStemmerBase: public Stemmer {
protected:
	virtual void store(QString entry, AmbiguitySolution & s)=0;
public:
	AmbiguityStemmerBase(QString & word): Stemmer(&word,0) { }
	virtual bool on_match();
};

class AmbiguityStemmer: public AmbiguityStemmerBase {
private:
	AmbiguitySolutionList list;
protected:
	virtual void store(QString entry, AmbiguitySolution & s);
public:
	AmbiguityStemmer(QString & word): AmbiguityStemmerBase(word) { }
	int getAmbiguity(Ambiguity amb) const;
};

#endif // AMBIGUITY_H

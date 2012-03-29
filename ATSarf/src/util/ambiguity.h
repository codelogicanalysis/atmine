#ifndef AMBIGUITY_H
#define AMBIGUITY_H

#include "stemmer.h"
#include "morphemes.h"

enum Ambiguity {Vocalization, Description, POS, Tokenization, Stem_Ambiguity, All_Ambiguity};
static const int ambiguitySize=(int)All_Ambiguity+1;


class AmbiguitySolution {
public:
	QString voc;
	QString desc;
	QString pos;
	bool featuresDefined:1;
	Morphemes morphemes;
	//TODO: other features
private:
	QString getFeatureIndex(const QString & feature, int index) const;
public:
	AmbiguitySolution(QString raw,QString des,QString POS);
	AmbiguitySolution(QString raw,QString des,QString POS, Morphemes morphemes);
	bool equal (const AmbiguitySolution & other, Ambiguity m) const;
	MorphemeType getMorphemeTypeAtPosition(int & diacriticPos, const QList<int> & diaPos,int & relativePos, int & morphemeSize);
private:
	int getTokenization() const;
};

typedef QList<AmbiguitySolution> AmbiguitySolutionList;
typedef QPair<QString, AmbiguitySolutionList > EntryAmbiguitySolutionList;

QString interpret(Ambiguity a);

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

inline unsigned int qHash(const Morpheme & m) {
	return qHash(m.start+m.end+(int)m.type);
}

inline unsigned int qHash(const Morphemes & m) {
	unsigned int h=0;
	for (int i=0;i<m.size();i++)
		h+=qHash(m[i]);
	return h;
}



#endif // AMBIGUITY_H

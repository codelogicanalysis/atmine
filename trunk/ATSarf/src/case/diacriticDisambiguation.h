#ifndef DIACRITICDISAMBIGUATION_H
#define DIACRITICDISAMBIGUATION_H

#include <QFile>
#include "affixTreeTraversal.h"
#include "stemmer.h"
#include "ambiguity.h"
#include "morphemes.h"

const static QChar diacritic_shadde_delimeter='\t';



class DiacriticDisambiguationBase {
	friend class DisambiguationStemmer;
public:
	typedef QHash<long, EntryAmbiguitySolutionList> Map;
protected:
	static const int maxDiacritics =15;
protected:
	bool mapBased:1;
	bool suppressOutput:1;
	int diacriticsCount:4;
	Map solutionMap; //TODO: better solution use unioin in terms of memory usage efficiency
	QString currEntry;
	long currId;
	AmbiguitySolutionList currSol;
	long long total[ambiguitySize];
	long double left[ambiguitySize];
	long long totalBranching[ambiguitySize];
	long long leftBranching[ambiguitySize];
	long long countAmbiguity[ambiguitySize];
	long long bestLeft[ambiguitySize];
	long long worstLeft[ambiguitySize];
	long long countReduced[ambiguitySize];
	long long reducingCombinations[ambiguitySize];
	long long totalCombinations[ambiguitySize];
	long long countWithoutDiacritics;
private:
        void printDiacriticDisplay(Diacritics d, QTextStream * o=&(theSarf->out));
        void printDiacritics(QString entry, int pos, QChar c, QTextStream * o=&(theSarf->out)); //for one diacritic
        void printDiacritics(QString voc,const QList<Diacritics> & d, AmbiguitySolution sol, QTextStream * o=&(theSarf->out)); //for multiple diacritcs
	void reset();
protected:
	DiacriticDisambiguationBase(bool mapBased, bool suppressOutput,int diacriticsCount=1);
	void store(long id, QString entry, AmbiguitySolution & s);
	void store(long id, QString entry, QString raw_data, QString description, QString POS);
	void store(long id, QString entry, QString raw_data, QString description, QString POS, Morphemes morphemes);
	void analyze();
	void analyzeOne(QString currAffix,const AmbiguitySolutionList & currSol);

	virtual ~DiacriticDisambiguationBase();
};

class AffixDisambiguation: public AffixTraversal, public DiacriticDisambiguationBase {
private:
	void visit(node *n, QString affix, QString raw_data, long category_id, QString description, QString POS);
public:
	AffixDisambiguation(item_types type, int numDiacritcs):AffixTraversal(type,true,true), DiacriticDisambiguationBase(true,false,numDiacritcs) {	}
};

class StemDisambiguation:public DiacriticDisambiguationBase {
private:
	StemNodesList * stemNodes;
	ItemCatRaw2AbsDescPosMapPtr map;
public:
	StemDisambiguation(int numDiacritics=1): DiacriticDisambiguationBase(true,false,numDiacritics) {
		stemNodes=database_info.trie_nodes;
		map=database_info.map_stem;
	}
	void operator()();
};


class FullFileDisambiguation: public DiacriticDisambiguationBase {
private:
	QString inputFileName, reducedFileName, allFileName;
	QFile reducedFile, allFile;
	QIODevice * oldDevice, * oldDeviceAll;
	ATMProgressIFC * prg;
public:
	FullFileDisambiguation(QString inputFileName, ATMProgressIFC * prg, int numDiacritcs=1, QString reducedFile="reducedOutput", QString allFile="fullOutput");
	void operator()();
	~FullFileDisambiguation();
};

class FullListDisambiguation: public DiacriticDisambiguationBase {
private:
	QStringList & list;
	ATMProgressIFC * prg;
public:
	FullListDisambiguation(QStringList & inputList, ATMProgressIFC * prg, int numDiacritcs=1);
	void operator()();
	~FullListDisambiguation();
};


class DisambiguationStemmer: public AmbiguityStemmerBase {
private:
	long id;
	DiacriticDisambiguationBase & storage;
protected:
	virtual void store(QString entry,AmbiguitySolution & s);
public:
	DisambiguationStemmer(long id,QString & word,DiacriticDisambiguationBase & t): AmbiguityStemmerBase(word),storage(t) {
		this->id=id;
	}
};



#endif // DIACRITICDISAMBIGUATION_H

#ifndef DIACRITICDISAMBIGUATION_H
#define DIACRITICDISAMBIGUATION_H

#include "affixTreeTraversal.h"
#include "stemmer.h"
#include <QFile>

class DiacriticDisambiguationBase {
public:
	enum Ambiguity {Vocalization, Description, POS, Tokenization, Stem_Ambiguity, All_Ambiguity};
	static QString interpret(Ambiguity a);
	class Solution {
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
		Solution(QString raw,QString des,QString POS);
		Solution(QString raw,QString des,QString POS,
				 int stemStart, int suffStart, int stemIndex, int numPrefixes, int numSuffixes);
		bool equal (const Solution & other, Ambiguity m) const;
	private:
		int getTokenization() const;
	};
	typedef QList<Solution> SolutionList;
	typedef QPair<QString, SolutionList > AffixSolutionList;
	typedef QHash<long, AffixSolutionList> Map;
protected:
	static const int ambiguitySize=(int)All_Ambiguity+1;
	static const int maxDiacritics =15;
protected:
	bool mapBased:1;
	bool suppressOutput:1;
	int diacriticsCount:4;
	Map solutionMap; //TODO: better solution use unioin in terms of memory usage efficiency
	QString currEntry;
	long currId;
	SolutionList currSol;
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
	void printDiacriticDisplay(Diacritics d);
	void printDiacritics(QString entry, int pos, QChar c); //for one diacritic
	void printDiacritics(const QList<Diacritics> & d); //for multiple diacritcs
	item_types getDiacriticPosition(Solution & sol, int diacriticPos) const;
	SolutionList getUnique(const SolutionList & list, Ambiguity m);
	void reset();
protected:
	DiacriticDisambiguationBase(bool mapBased, bool suppressOutput,int diacriticsCount=1);
	void store(long id, QString entry, Solution & s);
	void store(long id, QString entry, QString raw_data, QString description, QString POS);
	void store(long id, QString entry, QString raw_data, QString description, QString POS, int stemStart,
			   int suffStart, int stemIndex, int numPrefixes, int numSuffixes);
	void analyze();
	void analyzeOne(QString currAffix,const SolutionList & currSol);

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


class FullDisambiguation: public DiacriticDisambiguationBase {
private:
	QString inputFileName, outputFileName;
	QFile outFile;
	QIODevice * oldDevice;
	ATMProgressIFC * prg;

	friend class DisambiguationStemmer;
public:
	FullDisambiguation(QString inputFileName, ATMProgressIFC * prg, int numDiacritcs=1, QString outputFileName="fullOutput");
	void operator()();
	~FullDisambiguation();
};

class DisambiguationStemmer: public Stemmer {
private:
	long id;
	FullDisambiguation & storage;
public:
	DisambiguationStemmer(long id,QString & word,FullDisambiguation & t): Stemmer(&word,0),storage(t) {
		this->id=id;
	}
	virtual bool on_match();
};

#endif // DIACRITICDISAMBIGUATION_H

#ifndef DIACRITICDISAMBIGUATION_H
#define DIACRITICDISAMBIGUATION_H

#include "affixTreeTraversal.h"
#include "stemmer.h"

class DiacriticDisambiguationBase {
public:
	enum Ambiguity {Vocalization, Description, POS, Tokenization, All_Ambiguity};
	static QString interpret(Ambiguity a);
	class Solution {
	public:
		QString voc;
		QString desc;
		QString pos;
		Solution(QString raw,QString des,QString POS): voc(raw), desc(des),pos(POS) { }
		bool equal (const Solution & other, Ambiguity m) const;
	private:
		int getTokenization() const;
	};
	typedef QList<Solution> SolutionList;
	typedef QPair<QString, SolutionList > AffixSolutionList;
	typedef QMap<long, AffixSolutionList> Map;
protected:
	Map solutionMap;
	int total[(int)All_Ambiguity+1];
	double left[(int)All_Ambiguity+1];
	int countAmbiguity[(int)All_Ambiguity+1];
protected:
	DiacriticDisambiguationBase();
	void store(long id, QString entry, QString raw_data, QString description, QString POS);
	void analyze();
	SolutionList getUnique(const SolutionList & list, Ambiguity m);
	virtual ~DiacriticDisambiguationBase();
};

class AffixDisambiguation: public AffixTraversal, public DiacriticDisambiguationBase {
private:
	void visit(node *n, QString affix, QString raw_data, long category_id, QString description, QString POS);
public:
	AffixDisambiguation(item_types type):AffixTraversal(type,true,true) {	}
};

class StemDisambiguation:public DiacriticDisambiguationBase {
private:
	StemNodesList * stemNodes;
	ItemCatRaw2AbsDescPosMapPtr map;
public:
	StemDisambiguation() {
		stemNodes=database_info.trie_nodes;
		map=database_info.map_stem;
	}
	void operator()();
};


class FullDisambiguation: public DiacriticDisambiguationBase {
private:
	QString fileName;
	ATMProgressIFC * prg;

	friend class DisambiguationStemmer;
public:
	FullDisambiguation(QString fileName, ATMProgressIFC * prg) {
		this->fileName=fileName;
		this->prg=prg;
	}
	void operator()();
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

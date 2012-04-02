#ifndef DIACRITICSPOSTPROCESSING_H
#define DIACRITICSPOSTPROCESSING_H

#include <QHash>
#include <QList>
#include "ATMProgressIFC.h"
#include "diacritics.h"
#include "ambiguity.h"


class DiacriticsPostProcessing {
public:
	class Average {
	private:
		long double sum;
		int count;
	public:
		Average(){reset();}
		Average(double s): sum(s), count(1) {}
		void reset() {sum=0;count=0;}
		void add(double s) {sum+=s;count++;}
		double evaluate() {return sum/count;}
		double fractionOf(int total) {return ((double)count)/total;}
	};

	typedef QPair<QString,Average> StatPair;
	typedef QHash<QString,Average> StatTable;
	typedef QList<int> IndexList;
protected:
	IndexList testedFeatures;
	IndexList uniqueFeatures; //those refer to features whose accounting in sum must be done once
	int ambiguityIndex;
private:
	StatTable stats;
	int totalLines;

	QString lastUnique; //used for lumping
	QString lastFeature;
	Average lastAverage;
	//StatPair lastPair;

protected:
	virtual QString preProcessFeature(int index,QString feature);
	virtual QString displayFeature(int index, QString feature);
private:
	void restartLumpedEntries(QString newUnique="", QString newFeature="");
	QString getKey(QStringList list, IndexList keyConstituents);
	void processLine(QStringList list);
	void clearStats();
	void displayStats();
	void checkFeatureConsistency();
protected:
	DiacriticsPostProcessing(){ }
public:
	DiacriticsPostProcessing(const IndexList & keyfeatures,const IndexList & unique, int ambiguity)
		:testedFeatures(keyfeatures),uniqueFeatures(unique),ambiguityIndex(ambiguity) { }
	void operator()(QString fileName, ATMProgressIFC * prg);
};

class OneDiacriticEvaluation: public DiacriticsPostProcessing {
public:
	static const int	diacriticIndex, ambiguityIndexStart, letterIndex,
						morphemeIndex, morphemeActualPosIndex, morphemeRelPosIndex,
						longVowelIndex, shamsiIndex; //check .cpp for values
	static const QPair<int,int> uniqueFeatureRange;
	static const QPair<int,int> morphemeFeatureRange;
protected:
	virtual QString displayFeature(int index, QString feature);
public:
	OneDiacriticEvaluation(IndexList features, Ambiguity am);
};


#endif // DIACRITICSPOSTPROCESSING_H

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
		double count;
	public:
		Average(){reset();}
		Average(double s): sum(s), count(1) {}
		Average(double s, double weight): sum(s*weight), count(weight) {}
		void reset() {sum=0;count=0;}
		void add(double s) {sum+=s;count++;}
		void add(double s, double weight) {sum+=(s*weight);count+=weight;}
		double evaluate() const {return sum/count;}
		double fractionOf(int total) {return ((double)count)/total;}
		double getNumerator() const {return sum;}
		double getDenominator() const {return count;}
		QString toString() const {return QString("%1/%2=%3").arg((double)sum).arg(count).arg(evaluate());}
	};

	typedef QPair<QString,Average> StatPair;
	typedef QHash<QString,Average> StatTable;
	typedef QList<int> IndexList;
	typedef QList<IndexList> IndexLists;
	typedef QList<StatTable> StatPairList;
protected:
	IndexLists testedFeaturesList;
	IndexList uniqueFeatures; //those refer to features whose accounting in sum must be done once
	int ambiguityIndex;
	int totalLines;

	StatTable stats;

	QString lastUnique; //used for lumping
	StatPairList lastPairs;

protected:
	virtual QString preProcessFeature(int index,QString feature);
	virtual QString displayFeature(int index, QString feature);
	virtual QString getFeatureName(int index);
private:
	void restartLumpedEntries(QString newUnique="");
	QString getKey(QStringList list, IndexList keyConstituents);
	void processLine(QStringList list);
	void clearStats();
	void displayStats();
	void checkFeatureConsistency();
	void addToHash(StatTable & stats, QString feature, double amb, double weight=1.0);
protected:
	DiacriticsPostProcessing(){ }
public:
	DiacriticsPostProcessing(const IndexLists & keyfeatures,const IndexList & unique, int ambiguity)
		:testedFeaturesList(keyfeatures),uniqueFeatures(unique),ambiguityIndex(ambiguity) { }
	void operator()(QString fileName, ATMProgressIFC * prg);
};

class OneDiacriticEvaluation: public DiacriticsPostProcessing {
public:
	static const int	vocIndex, sizeIndex, diacriticCountIndex,
						diacriticPosIndex, diacriticIndex, letterIndex,
						morphemeIndex, morphemeActualPosIndex, morphemeRelPosIndex,
						longVowelIndex, shamsiIndex,
						ambiguityIndexStart; //check .cpp for values
	static const QPair<int,int> uniqueFeatureRange;
	static const QPair<int,int> morphemeFeatureRange;
protected:
	virtual QString displayFeature(int index, QString feature);
	virtual QString getFeatureName(int index);
public:
	OneDiacriticEvaluation(IndexList features, Ambiguity am);
};


#endif // DIACRITICSPOSTPROCESSING_H

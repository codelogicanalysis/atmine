#ifndef VOCALIZEDCOMBINATIONS_H
#define VOCALIZEDCOMBINATIONS_H

#include "diacritics.h"
#include "combinations.h"
#include <QHash>
#include <QSet>

class DiacriticsNposition {
public:
	int position;
	Diacritic diacritic;
public:
	DiacriticsNposition(int p, Diacritic dia): position(p), diacritic(dia) { }
};

typedef QList<DiacriticsNposition> DiacriticsPositionsList;

class VocalizedCombination {
private:
	friend class VocalizedCombinationsGenerator;
	QString voc;
	DiacriticsPositionsList shortList;
	QList<Diacritics> list;
private:
	VocalizedCombination(const QString & aVoc, const DiacriticsPositionsList & aShortList): voc(aVoc), shortList(aShortList) {	}
public:
	static VocalizedCombination deduceCombination(QString voc);
public:
	QString getString() const { return voc;}
	const DiacriticsPositionsList & getShortList() const { return shortList;}
	int getNumDiacritics() const { return shortList.size();}
	const QList<Diacritics> & getDiacritics();
	bool operator ==(const VocalizedCombination & other) const  { return voc==other.voc; }
	bool hasSelfInconsistency() const;
};

class VocCombIndexListPair {
public:
	VocalizedCombination comb;
	QSet<int> indicies;

	VocCombIndexListPair(VocalizedCombination & c):comb(c) {}
	bool operator ==(const VocCombIndexListPair & other) const  { return comb==other.comb; }
};

class VocalizedCombinationsGenerator {
private:
	class DiacPos {
	public:
		int pos;
		QChar diacritic;
	public:
		DiacPos(int p, QChar d):pos(p),diacritic(d) {}
	};

private:
	QString voc;
	QString unvoc;
	QList<DiacPos> diacritics;
	CombinationGenerator * generator;
public:
	VocalizedCombinationsGenerator(QString voc, int numVoc);
	bool isUnderVocalized() const {return generator->isUnderDefined();}
	VocalizedCombinationsGenerator & begin();
	VocalizedCombinationsGenerator & operator++();
	bool isFinished() const;
	QString getString() const;
	VocalizedCombination getCombination() const;
	VocalizedCombination operator *() const { return getCombination(); }
	~VocalizedCombinationsGenerator() {delete generator;}
};

inline unsigned int qHash(const VocalizedCombination & c) {
	return qHash(c.getString());
}

inline unsigned int qHash(const VocCombIndexListPair & c) {
	return qHash(c.comb);
}

#endif // VOCALIZEDCOMBINATIONS_H

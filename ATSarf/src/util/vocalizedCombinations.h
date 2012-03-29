#ifndef VOCALIZEDCOMBINATIONS_H
#define VOCALIZEDCOMBINATIONS_H

#include "diacritics.h"
#include <QHash>
#include <QSet>

class VocalizedCombination {
private:
	friend class VocalizedCombinationsGenerator;
	QString voc;
	QList<int> positions;
	QList<Diacritics> list;
private:
	VocalizedCombination(const QString & aVoc, const QList<int> & aPositions): voc(aVoc), positions(aPositions) {	}
public:
	static VocalizedCombination deduceCombination(QString voc);
public:
	QString getString() const { return voc;}
	const QList<int> & getPositions() const { return positions;}
	int getNumDiacritics() const { return positions.size();}
	const QList<Diacritics> & getDiacritics();
	bool operator ==(const VocalizedCombination & other) const  { return voc==other.voc; }
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
		DiacPos(int position,QChar dia): pos(position), diacritic(dia) { }
	};

private:
	QString voc;
	QString unvoc;
	QList<DiacPos> diacritics;
	QList<int> indicies;
	int numDiacritics;
private:
	bool initialize(int i, int index);
	void iterate(int i);
public:
	VocalizedCombinationsGenerator(QString voc, int numVoc);
	bool isUnderVocalized() const {return numDiacritics>diacritics.size();}
	VocalizedCombinationsGenerator & begin();
	VocalizedCombinationsGenerator & operator++();
	bool isFinished() const;
	QString getString() const;
	VocalizedCombination getCombination() const;
	VocalizedCombination operator *() const { return getCombination(); }
};

inline unsigned int qHash(const VocalizedCombination & c) {
	return qHash(c.getString());
}

inline unsigned int qHash(const VocCombIndexListPair & c) {
	return qHash(c.comb);
}

#endif // VOCALIZEDCOMBINATIONS_H

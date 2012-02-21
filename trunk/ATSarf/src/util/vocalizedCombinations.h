#ifndef VOCALIZEDCOMBINATIONS_H
#define VOCALIZEDCOMBINATIONS_H

#include "diacritics.h"
#include <QHash>

class VocalizedCombinations {
public:
	class Combination {
	private:
		friend class VocalizedCombinations;
		QString voc;
		QList<int> positions;
		QList<Diacritics> list;
	private:
		Combination(const QString & aVoc, const QList<int> & aPositions): voc(aVoc), positions(aPositions) {	}
	public:
		static Combination deduceCombination(QString voc);
	public:
		QString getString() const { return voc;}
		const QList<int> & getPositions() const { return positions;}
		int getNumDiacritics() const { return positions.size();}
		const QList<Diacritics> & getDiacritics();
		bool operator ==(const Combination & other) const  { return voc==other.voc; }
	};

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
	VocalizedCombinations(QString voc, int numVoc);
	bool isUnderVocalized() const {return numDiacritics>diacritics.size();}
	VocalizedCombinations & begin();
	VocalizedCombinations & operator++();
	bool isFinished() const;
	QString getString() const;
	Combination getCombination() const;
	Combination operator *() const { return getCombination(); }
};

inline unsigned int qHash(const VocalizedCombinations::Combination & c) {
	return qHash(c.getString());
}

#endif // VOCALIZEDCOMBINATIONS_H

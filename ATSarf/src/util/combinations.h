#ifndef COMBINATIONS_H
#define COMBINATIONS_H

#include <QList>

typedef QList<int> Combination;

class CombinationGenerator {

private:
	QList<int> indicies;
	int numIndicies;
	int maxSize;
private:
	bool initialize(int i, int index);
	void iterate(int i);
public:
	CombinationGenerator(int countIndicies, int max):numIndicies(countIndicies), maxSize(max) {}
	CombinationGenerator & begin();
	CombinationGenerator & operator++();
	bool isUnderDefined() const {return numIndicies>maxSize;}
	bool isFinished() const;
	const Combination & getCombination() const;
	template<class T> QList<T> getCombinationEquivalent(const QList<T> & list) const ;
};

template<class T>
QList<T> CombinationGenerator::getCombinationEquivalent(const QList<T> & list) const {
	assert(list.size()>=indicies.size());
	QList<T> listEq;
	for (int i=0;i<indicies.size();i++) {
		int index=indicies[i];
		listEq.append(list[index]);
	}
	return listEq;
}

#endif // COMBINATIONS_H

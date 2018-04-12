#ifndef ITERATIVEMATHFUNCTIONS_H
#define ITERATIVEMATHFUNCTIONS_H

#include <QStringList>
#include <QList>
#include "instanceIterator.h"
#include "distinguishingLargeFileIterator.h"
#include "iterativeStandardDeviation.h"

typedef IterativeStandardDeviation ItDev;
typedef QList<ItDev> ItDevList;


class IterativeMathFunctions{
private:
	static long double weightedSumHelper(InstanceIterator & itr, long double & totalWeight);
public:
	static long double weightedSum(InstanceIterator & itr);
	static double weightedMean(InstanceIterator & itr);
	static double weightedStandardDeviation(InstanceIterator & itr);
	static double weightedStandardDeviationReduction(DistinguishingLargeFileIterator & itr, QStringList & distingushingValues, ItDevList & devs, ItDev & totalDev);	//maybe use instead some interface just for distinguishing iterator which is another superclass for DistinguishingLargeFileIterator
};

#endif // ITERATIVEMATHFUNCTIONS_H

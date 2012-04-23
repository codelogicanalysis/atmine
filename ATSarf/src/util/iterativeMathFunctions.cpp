#include <math.h>
#include "iterativeMathFunctions.h"
#include "iterativeStandardDeviation.h"


long double IterativeMathFunctions::weightedSumHelper(InstanceIterator & itr, long double & totalWeight) {
	double value, weight;
	long double sum=0;
	totalWeight=0;
	for (itr.start();itr.getInstance(value,weight);itr.next()) {
		double weigthedValue=value*weight;
		sum+=weigthedValue;
		totalWeight+=weight;
	}
	return sum;
}

long double IterativeMathFunctions::weightedSum(InstanceIterator & itr) {
	long double totalWeight; //unused in current calculation
	long double sum=weightedSumHelper(itr,totalWeight);
	return sum;
}

double IterativeMathFunctions::weightedMean(InstanceIterator & itr) {
	long double totalWeight;
	long double sum=weightedSumHelper(itr,totalWeight);
	return sum/totalWeight;
}

double IterativeMathFunctions::weightedStandardDeviation(InstanceIterator & itr) {
	IterativeStandardDeviation dev;
	double xi,wi;
	for (itr.start();itr.getInstance(xi,wi);itr.next()) {
		dev.update(xi,wi);
	}
	return dev.getValue();
}

double IterativeMathFunctions::weightedStandardDeviationReduction(DistinguishingLargeFileIterator & itr, QStringList & distingushingValues, QList<double> & meanValues, QList<double> & standardDeviations) {
	distingushingValues.clear();
	standardDeviations.clear();
	meanValues.clear();
	IterativeStandardDeviation totalDeviation;
	typedef QMap<QString,IterativeStandardDeviation> DevMap;
	DevMap deviationMaps;
	double xi,wi;
	for (itr.start();itr.getInstance(xi,wi);itr.next()) {
		totalDeviation.update(xi,wi);
		QString distinguishingVal=itr.getDistinguishingValue();
		deviationMaps[distinguishingVal].update(xi,wi);
	}
	double totalDev=totalDeviation.getValue();
	int totalInstances=totalDeviation.getInstancesNum();
	double distTotalDev=0;
	for (DevMap::iterator i=deviationMaps.begin();i!=deviationMaps.end();i++) {
		distingushingValues.append(i.key());
		IterativeStandardDeviation & d=*i;
		double dev=d.getValue();
		double mean=d.getWeightedMean();
		standardDeviations.append(dev);
		meanValues.append(mean);
		int num=d.getInstancesNum();
		double probability=((double)num)/totalInstances;
		distTotalDev+=(dev*probability);

	}
	double SDR=totalDev-distTotalDev;
	return SDR;
}



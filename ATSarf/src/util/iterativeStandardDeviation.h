#ifndef ITERATIVESTANDARDDEVIATION_H
#define ITERATIVESTANDARDDEVIATION_H

#include <math.h>

/**can be computed on the fly as: (http://en.wikipedia.org/wiki/Mean_square_weighted_deviation)
	( [sum (wi * xi^2) ] * sum (wi) - [sum (wi * xi)]^2 )
	----------------------------------------------------
				( [sum(wi)]^2 - sum(wi^2) )

	in what follow we denote:

	sumWeighted_xi_2 * sum_wi - (weightedSum)^2
	--------------------------------------------
			(sum_wi)^2 - sum_wi_2
	**/

class IterativeStandardDeviation {
private:
	long double sumWeighted_xi_2;
	long double sum_wi;
	long double weightedSum;
	long double sum_wi_2;

	int numInstances; //not needed in calculation just to keep track
public:
	IterativeStandardDeviation() {
		clear();
	}
	void clear() {
		sumWeighted_xi_2=0;
		sum_wi=0;
		weightedSum=0;
		sum_wi_2=0;
		numInstances=0; //not used in main calculation
	}
	void update(double xi, double wi) {
		sumWeighted_xi_2+= wi*xi*xi;
		sum_wi+= wi;
		weightedSum+= wi*xi;
		sum_wi_2+=wi*wi;
		numInstances++;
	}
	double getValue() const {
		if (numInstances==1)
			return 0;
		double dev=pow((sumWeighted_xi_2 * sum_wi - weightedSum*weightedSum)/(sum_wi*sum_wi-sum_wi_2),0.5);
		return dev;
	}
	int getInstancesNum() const {return numInstances;}
	double getWeightedMean() const { return weightedSum/sum_wi;}
};

#endif // ITERATIVESTANDARDDEVIATION_H

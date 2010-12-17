#include "Math_functions.h"
#include <QtAlgorithms>
#include <math.h>

double average(QVector<int> & list)
{
	double avg=0;
	for (int i=0;i<list.size();i++)
		avg+=list[i];
	avg/=list.size();
	return avg;
}

int median(QVector<int> & list)
{
	qSort(list.begin(),list.end());
	return list[list.size()/2];
}

double standard_deviation(QVector<int> & list, double average)//must be optimized
{
	double dev=0;
	for (int i=0;i<list.size();i++)
		dev+=pow(list[i]-average,2);
	dev/=list.size()-1;
	dev=sqrt(dev);
	return dev;
}

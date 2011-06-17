#ifndef MATH_FUNCTIONS_H
#define MATH_FUNCTIONS_H

#include <QVector>
#include <QtAlgorithms>
#include <math.h>

template<class T>
double average(QVector<T> & list)
{
	double avg=0;
	for (int i=0;i<list.size();i++)
		avg+=list[i];
	avg/=list.size();
	return avg;
}

template<class T>
int median(QVector<T> & list)
{
	qSort(list.begin(),list.end());
	return list[list.size()/2];
}

template<class T>
double standard_deviation(QVector<T> & list, double average)//must be optimized
{
	double dev=0;
	for (int i=0;i<list.size();i++)
		dev+=pow(list[i]-average,2);
	dev/=list.size()-1;
	dev=sqrt(dev);
	return dev;
}


#endif // MATH_FUNCTIONS_H

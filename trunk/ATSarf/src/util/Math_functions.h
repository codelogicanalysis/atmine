/**
  * @file   Math_functions.h
  * @brief  this header file contains math functions defined for common use in the project
  * @author Jad Makhlouta
  */
#ifndef MATH_FUNCTIONS_H
#define MATH_FUNCTIONS_H

#include <QVector>
#include <QtAlgorithms>
#include <math.h>

/**
  * This function calculates the sum of the elements in an input vector
  * @author Jad Makhlouta
  * @param  list reference of a QVector list
  * @return a double representing the sum of the input
  */
template<class T>
double sum(QVector<T> & list) {
    /// initializing the sum to zero
    double sum=0;
    for (int i=0;i<list.size();i++)
        /// adding the elements in the list
        sum+=list[i];
    return sum;
}

/**
  * This function calculates the average of the elements in an input vector
  * @author Jad Makhlouta
  * @param  list reference of a QVector list
  * @return a double representing the average of the input
  */
template<class T>
double average(QVector<T> & list) {
    double avg=0;
    for (int i=0;i<list.size();i++)
        /// summing the elements of the list
        avg+=list[i];
    /// dividing by list size to get average
    avg/=list.size();
    return avg;
}

/**
  * This function calculates the median of the elements in an input vector
  * @author Jad Makhlouta
  * @param  list reference of a QVector list
  * @return an integer representing the median of the input
  */
template<class T>
int median(QVector<T> & list) {
    /// passing the list to a sorting function
    qSort(list.begin(),list.end());
    /// returning the middle element which is the median
    return list[list.size()/2];
}

/**
  * This function calculates the standard deviation of the elements in an input vector
  * @author Jad Makhlouta
  * @param  list reference of a QVector list
  * @param  average value of the average of input elements
  * @return an integer representing the median of the input
  */
template<class T>
double standard_deviation(QVector<T> & list, double average) {//must be optimized
    double dev=0;
    for (int i=0;i<list.size();i++)
        /// summing the squared value of each entiry minus the average
        dev+=pow(list[i]-average,2);
    /// dividing the previous value by the list value minus one
    dev/=list.size()-1;
    /// apply square root and return
    dev=sqrt(dev);
    return dev;
}


#endif // MATH_FUNCTIONS_H

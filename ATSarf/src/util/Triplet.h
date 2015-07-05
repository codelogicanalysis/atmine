#ifndef TRIPLET_H
#define TRIPLET_H

/**
  * @file   Triplet.h
  * @author Jad Makhlouta
  * @brief  This file defines a class containing a triplet with types defined using templates.
  *         It also implements special method for triplet manipulation and overloads some operators.
  */

#include <QtCore/qdatastream.h>
#include <QHash>
#include <QDataStream>

/**
  * @class  Triplet
  * @author Jad Makhlouta
  * @brief  This class defines a class of triplets with types of those triplets being templates
  */
template <class T1, class T2, class T3>
struct Triplet
{
        /**
          * This is the default constructor of the Triplet class
          */
        Triplet() : first(T1()), second(T2()), third(T3())  {}
        /**
          * This is a constructor of class taking references to the three types of the triplet.
          */
        Triplet(const T1 &t1, const T2 &t2, const T3 &t3) : first(t1), second(t2), third(t3) {}

        /**
          * This function overloads the '=' operator in order to be able to set a triplet class equal to another
          */
	Triplet<T1, T2, T3> &operator=(const Triplet<T1, T2, T3> &other)
	{ first = other.first; second = other.second; third=other.third; return *this; }

        /**
          * This is a constructor of the class taking a constant triplet and initializing a Triplet with same values
          */
	Triplet(const Triplet<T1, T2, T3> &other):first(other.first), second(other.second), third(other.third) {}

        T1 first;   //!< This is the first variable of the triplet with type T1
        T2 second;  //!< This is the second variable of the triplet with type T2
        T3 third;   //!< This is the third variable of the triplet with type T3
};

/**
  * This function overloads the '==' operator comparing the values of the 3 elements between 2 triplets
  * @return boolean reffering to comparison of the two triplets with operation p1 == p2
  * @param  p1 first triplet
  * @param  p2 second triplet
  */
template <class T1, class T2, class T3>
Q_INLINE_TEMPLATE bool operator==(const  Triplet<T1, T2, T3> &p1, const  Triplet<T1, T2, T3> &p2)
{ return p1.first == p2.first && p1.second == p2.second && p1.third == p2.third; }

/**
  * This function overloads the '!=' operator comparing the values of the 3 elements between 2 triplets
  * @return boolean reffering to the output of the inequality comparison p1 != p2
  * @param  p1 first triplet class reference
  * @param  p2 second triplet class reference
  */
template <class T1, class T2, class T3>
Q_INLINE_TEMPLATE bool operator!=(const  Triplet<T1, T2, T3> &p1, const  Triplet<T1, T2, T3> &p2)
{ return !(p1 == p2); }

template <class T1, class T2, class T3>
Q_INLINE_TEMPLATE bool operator<(const  Triplet<T1, T2, T3> &p1, const  Triplet<T1, T2, T3> &p2)
{
	return p1.first < p2.first || (!(p2.first < p1.first) && p1.second < p2.second) || (!(p2.first < p1.first) && !(p1.second < p2.second) && p1.third < p2.third);
}

/**
  * This method overloads the '>' operator comparing the values of the 3 elements between 2 triplets
  * @return boolean reffering to the output of the operation p1 > p2
  * @param  p1 first triplet class reference
  * @param  p2 second triplet class reference
  *
  */
template <class T1, class T2, class T3>
Q_INLINE_TEMPLATE bool operator>(const  Triplet<T1, T2, T3> &p1, const  Triplet<T1, T2, T3> &p2)
{
	return p2 < p1;
}

/**
  * This method overloads the '<=' operator comparing the values of the 3 elements between 2 triplets
  * @return boolean reffering to the output of the operation p1 <= p2
  * @param  p1 first triplet class reference
  * @param  p2 second triplet class reference
  *
  */
template <class T1, class T2, class T3>
Q_INLINE_TEMPLATE bool operator<=(const  Triplet<T1, T2, T3> &p1, const  Triplet<T1, T2, T3> &p2)
{
	return !(p2 < p1);
}

/**
  * This method overloads the '>=' operator comparing the values of the 3 elements between 2 triplets
  * @return boolean reffering to the output of the operation p1 >= p2
  * @param  p1 first triplet class reference
  * @param  p2 second triplet class reference
  *
  */
template <class T1, class T2, class T3>
Q_INLINE_TEMPLATE bool operator>=(const  Triplet<T1, T2, T3> &p1, const  Triplet<T1, T2, T3> &p2)
{
	return !(p1 < p2);
}

Q_INLINE_TEMPLATE uint qHash ( Triplet<long long, long, QString> t)
{
	return uint(qHash(t.first));//+qHash(t.second)+qHash(t.third));
}


template<class T1, class T2, class T3>
Q_INLINE_TEMPLATE uint qHash ( Triplet<T1, T2, T3> t)
{
	return uint(qHash(t.first)+qHash(t.second)+qHash(t.third));
}


template <class T1, class T2, class T3>
Q_INLINE_TEMPLATE QDataStream &operator<<(QDataStream &out, const Triplet<T1, T2, T3> &t)
{
	out<<t.first<<t.second<<t.third;
	return out;
}

template <class T1, class T2, class T3>
Q_INLINE_TEMPLATE QDataStream &operator>>(QDataStream &in, Triplet<T1, T2, T3> &t)
{
	in>>t.first>>t.second>>t.third;
	return in;
}


#endif // TRIPLET_H

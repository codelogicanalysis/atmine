#ifndef TRIPLET_H
#define TRIPLET_H

#include <QtCore/qdatastream.h>
#include <QHash>
#include <QDataStream>

template <class T1, class T2, class T3>
struct Triplet
{
	typedef T1 first_type;
	typedef T2 second_type;


	Triplet() : first(T1()), second(T2()), third(T2())  {}
	Triplet(const T1 &t1, const T2 &t2, const T3 &t3) : first(t1), second(t2), third(t3) {}

	Triplet<T1, T2, T3> &operator=(const Triplet<T1, T2, T3> &other)
	{ first = other.first; second = other.second; third=other.third; return *this; }

	Triplet(const Triplet<T1, T2, T3> &other):first(other.first), second(other.second), third(other.third) {}

	T1 first;
	T2 second;
	T3 third;
};

template <class T1, class T2, class T3>
Q_INLINE_TEMPLATE bool operator==(const  Triplet<T1, T2, T3> &p1, const  Triplet<T1, T2, T3> &p2)
{ return p1.first == p2.first && p1.second == p2.second && p1.third == p2.third; }

template <class T1, class T2, class T3>
Q_INLINE_TEMPLATE bool operator!=(const  Triplet<T1, T2, T3> &p1, const  Triplet<T1, T2, T3> &p2)
{ return !(p1 == p2); }

template <class T1, class T2, class T3>
Q_INLINE_TEMPLATE bool operator<(const  Triplet<T1, T2, T3> &p1, const  Triplet<T1, T2, T3> &p2)
{
	return p1.first < p2.first || (!(p2.first < p1.first) && p1.second < p2.second) || (!(p2.first < p1.first) && !(p1.second < p2.second) && p1.third < p2.third);
}

template <class T1, class T2, class T3>
Q_INLINE_TEMPLATE bool operator>(const  Triplet<T1, T2, T3> &p1, const  Triplet<T1, T2, T3> &p2)
{
	return p2 < p1;
}

template <class T1, class T2, class T3>
Q_INLINE_TEMPLATE bool operator<=(const  Triplet<T1, T2, T3> &p1, const  Triplet<T1, T2, T3> &p2)
{
	return !(p2 < p1);
}

template <class T1, class T2, class T3>
Q_INLINE_TEMPLATE bool operator>=(const  Triplet<T1, T2, T3> &p1, const  Triplet<T1, T2, T3> &p2)
{
	return !(p1 < p2);
}

Q_INLINE_TEMPLATE uint qHash ( Triplet<long long, long, QString> t)
{
	return uint(qHash(t.first));//+qHash(t.second)+qHash(t.third));
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

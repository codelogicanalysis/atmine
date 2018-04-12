#ifndef QUADRUPLET_H
#define QUADRUPLET_H


#include <QtCore/qdatastream.h>
#include <QHash>
#include <QDataStream>

template <class T1, class T2, class T3,class T4>
struct Quadruplet
{
	Quadruplet()  {}
	Quadruplet(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4) : first(t1), second(t2), third(t3),forth(t4) {}

	Quadruplet<T1, T2, T3, T4> &operator=(const Quadruplet<T1, T2, T3, T4> &other)
	{ first = other.first; second = other.second; third=other.third;forth=other.forth; return *this; }

	Quadruplet(const Quadruplet<T1, T2, T3, T4> &other):first(other.first), second(other.second), third(other.third),forth(other.forth) {}

	T1 first;
	T2 second;
	T3 third;
	T4 forth;
};

template <class T1, class T2, class T3,class T4>
Q_INLINE_TEMPLATE bool operator==(const  Quadruplet<T1, T2, T3, T4> &p1, const  Quadruplet<T1, T2, T3, T4> &p2)
{ return p1.first == p2.first && p1.second == p2.second && p1.third == p2.third && p1.forth == p2.forth; }

template <class T1, class T2, class T3,class T4>
Q_INLINE_TEMPLATE bool operator!=(const  Quadruplet<T1, T2, T3, T4> &p1, const  Quadruplet<T1, T2, T3, T4> &p2)
{ return !(p1 == p2); }

template <class T1, class T2, class T3,class T4>
Q_INLINE_TEMPLATE bool operator<(const Quadruplet<T1, T2, T3, T4>  &p1, const  Quadruplet<T1, T2, T3, T4> &p2)
{
	return p1.first < p2.first || ((p2.first == p1.first) && p1.second < p2.second) ||
			((p2.first == p1.first) && (p1.second == p2.second) && p1.third < p2.third) ||
			((p2.first == p1.first) && (p1.second == p2.second) && (p1.third == p2.third) && p1.forth < p2.forth);
}

template <class T1, class T2, class T3,class T4>
Q_INLINE_TEMPLATE bool operator>(const  Quadruplet<T1, T2, T3, T4> &p1, const  Quadruplet<T1, T2, T3, T4> &p2)
{
	return p2 < p1;
}

template <class T1, class T2, class T3,class T4>
Q_INLINE_TEMPLATE bool operator<=(const  Quadruplet<T1, T2, T3, T4> &p1, const  Quadruplet<T1, T2, T3, T4> &p2)
{
	return !(p2 < p1);
}

template <class T1, class T2, class T3,class T4>
Q_INLINE_TEMPLATE bool operator>=(const  Quadruplet<T1, T2, T3, T4> &p1, const  Quadruplet<T1, T2, T3, T4> &p2)
{
	return !(p1 < p2);
}

Q_INLINE_TEMPLATE uint qHash ( Quadruplet<long long, long, QString,bool> t)
{
	return uint(qHash(t.first));//+qHash(t.second)+qHash(t.third));
}

template <class T1, class T2, class T3,class T4>
Q_INLINE_TEMPLATE uint qHash ( Quadruplet<T1, T2, T3, T4> t)
{
	return uint(qHash(t.first)+qHash(t.second)+qHash(t.third)+qHash(t.forth));
}

template <class T1, class T2, class T3,class T4>
Q_INLINE_TEMPLATE QDataStream &operator<<(QDataStream &out, const Quadruplet<T1, T2, T3, T4> &t)
{
	out<<t.first<<t.second<<t.third<<t.forth;
	return out;
}

template <class T1, class T2, class T3,class T4>
Q_INLINE_TEMPLATE QDataStream &operator>>(QDataStream &in, Quadruplet<T1, T2, T3, T4> &t)
{
	in>>t.first>>t.second>>t.third>>t.forth;
	return in;
}



#endif // QUADRUPLET_H

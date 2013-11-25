#ifndef SARFTAGTYPE_H
#define SARFTAGTYPE_H
#include <QString>
#include <QVector>
#include <QPair>
#include "tagtype.h"

template <class T1, class T2, class T3, class T4>
struct Quadruple {
    typedef T1 first_type;
    typedef T2 second_type;
    typedef T3 third_type;
    typedef T4 fourth_type;

    Quadruple() : first(T1()), second(T2()), third(T3()), fourth(T4()) {}
    Quadruple(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4) : first(t1), second(t2), third(t3), fourth(t4) {}

    Quadruple<T1, T2, T3, T4> &operator=(const Quadruple<T1, T2, T3, T4> &other)
    { first = other.first; second = other.second; third = other.third; fourth = other.fourth; return *this; }

    T1 first;
    T2 second;
    T3 third;
    T4 fourth;
};

class SarfTagType : public TagType {

public:
    SarfTagType();
    SarfTagType( QString, QVector< Quadruple< QString , QString , QString , QString > >, QString, QString, QString, int, bool, bool, bool, Source);
    QVector< Quadruple< QString , QString , QString , QString > > tags;
};

#endif // SARFTAGTYPE_H

#include <QList>
#include <QTextStream>
#include <QFile>
#include "logger/logger.h"



class ChainNarratorPrim {
public:
    virtual void f()=0;

};

class NarratorPrim: public ChainNarratorPrim {
public:
    virtual void f()=0;
    long long m_start;
    long long m_end;

    NarratorPrim();
    NarratorPrim(long long m_start);


};

class ChainPrim :public ChainNarratorPrim {
public:
    virtual void f()=0;

};

class NamePrim :public NarratorPrim {
public:
    virtual void f(){}
    NamePrim();
    NamePrim(long long m_start);
};


class NameConnectorPrim :public NarratorPrim {
public:

    virtual void f(){}
    NameConnectorPrim();
    NameConnectorPrim(long long m_start);

};


class NarratorConnectorPrim :public ChainPrim {
public:
    virtual void f(){}
    long long m_start;
    long long m_end;
    NarratorConnectorPrim();
    NarratorConnectorPrim(long long m_start);


};

class Narrator :public ChainPrim {
public:
    virtual void f(){}
    QList <NarratorPrim *> m_narrator;
};

class Chain: public ChainNarratorPrim {
public:
    virtual void f(){}
    QList <ChainPrim *> m_chain;
    void serialize();


};

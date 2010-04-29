#include <QList>
#include <QTextStream>
#include <QFile>
#include "logger/logger.h"

class ChainNarratorPrim;

//typedef QList <ChainNarratorPrim *>::iterator CNPIterator;

//#enumerate the types

class ChainNarratorPrim {
public:

//    virtual CNPIterator first () const=0;
//    virtual CNPIterator end () const=0;
    virtual void serialize(QTextStream &chainOut,QStringList &wordList) const=0;
 //   virtual int startPosition() const=0;
 //   virtual int lastPosition() const=0;

};



class NarratorPrim: public ChainNarratorPrim {
public:
    long long m_start;
    long long m_end;

    NarratorPrim();
    NarratorPrim(long long m_start);

 //   virtual CNPIterator first () const=0;
 //   virtual CNPIterator end () const=0;
    virtual void serialize(QTextStream &chainOut,QStringList &wordList) const=0;

};

class ChainPrim :public ChainNarratorPrim {
public:
 //   virtual CNPIterator first () const=0;
//    virtual CNPIterator end () const=0;
    virtual void serialize(QTextStream &chainOut,QStringList &wordList) const=0;


};

class NamePrim :public NarratorPrim {
public:
    virtual void f(){}
    NamePrim();
    NamePrim(long long m_start);

   // virtual CNPIterator first () const{
   // return QList<ChainNarratorPrim*> ().begin();
  //      }

//    virtual CNPIterator end () const{
//    return QList<ChainNarratorPrim*> ().end();
//    }

void serialize(QTextStream &chainOut,QStringList &wordList) const;

};


class NameConnectorPrim :public NarratorPrim {
public:

    NameConnectorPrim();
    NameConnectorPrim(long long m_start);

//    virtual CNPIterator first () const{
//    return QList<ChainNarratorPrim*> ().begin();
//        }
//
//    virtual CNPIterator end () const{
//    return QList<ChainNarratorPrim*> ().end();
//    }

void serialize(QTextStream &chainOut,QStringList &wordList) const;
};


class NarratorConnectorPrim :public ChainPrim {
public:
    long long m_start;
    long long m_end;
    NarratorConnectorPrim();
    NarratorConnectorPrim(long long m_start);

//    virtual CNPIterator first () const{
//    return QList<ChainNarratorPrim*> ().begin();
//        }
//
//    virtual CNPIterator end () const{
//    return QList<ChainNarratorPrim*> ().end();
//    }
void serialize(QTextStream &chainOut,QStringList &wordList) const;

};

class Narrator :public ChainPrim {
public:
 //   QList <ChainNarratorPrim *> m_narrator; //modify back to here
   QList <NarratorPrim *> m_narrator;
//    virtual CNPIterator first () {
//    return m_narrator.begin();
//        }
//
//    virtual CNPIterator end () const{
//    //return m_narrator.end();
//    }

void serialize(QTextStream &chainOut,QStringList &wordList) const;
};

class Chain: public ChainNarratorPrim {
public:
    //QList <ChainNarratorPrim *> m_chain;//to be reverted to
    QList <ChainPrim *> m_chain;
    void serialize(QTextStream &chainOut,QStringList &wordList) const;


//    virtual CNPIterator first () const{
//    //return m_chain.begin();
//        }
//
//    virtual CNPIterator end () const{
//    //return m_chain.end();
//    }

};

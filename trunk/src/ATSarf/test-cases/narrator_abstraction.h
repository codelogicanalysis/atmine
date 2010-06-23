#include <QList>
#include <QTextStream>
#include <QFile>
#include "logger/logger.h"

#define SHOW_AS_TEXT


class ChainNarratorPrim;

//typedef QList <ChainNarratorPrim *>::iterator CNPIterator;

//#enumerate the types

class ChainNarratorPrim {
protected:
	QString * hadith_text;
public:
//    virtual CNPIterator first () const=0;
//    virtual CNPIterator end () const=0;
	virtual void serialize(QDataStream &chainOut) const=0;
	virtual void deserialize(QDataStream &chainOut)=0;
	virtual void serialize(QTextStream &chainOut) const=0;
 //   virtual int startPosition() const=0;
 //   virtual int lastPosition() const=0;

};
class NarratorPrim: public ChainNarratorPrim {
public:
    long long m_start;
    long long m_end;

	NarratorPrim(QString * hadith_text);
	NarratorPrim(QString * hadith_text,long long m_start);

 //   virtual CNPIterator first () const=0;
 //   virtual CNPIterator end () const=0;
	virtual void serialize(QDataStream &chainOut) const=0;
	virtual void deserialize(QDataStream &chainOut)=0;
	virtual void serialize(QTextStream &chainOut) const=0;
};
class ChainPrim :public ChainNarratorPrim {
public:
	ChainPrim(QString* hadith_text);
 //   virtual CNPIterator first () const=0;
//    virtual CNPIterator end () const=0;
	virtual void serialize(QDataStream &chainOut) const=0;
	virtual void deserialize(QDataStream &chainOut)=0;
	virtual void serialize(QTextStream &chainOut) const=0;
};
class NamePrim :public NarratorPrim {
public:
    virtual void f(){}
	NamePrim(QString * hadith_text);
	NamePrim(QString * hadith_text,long long m_start);

   // virtual CNPIterator first () const{
   // return QList<ChainNarratorPrim*> ().begin();
  //      }

//    virtual CNPIterator end () const{
//    return QList<ChainNarratorPrim*> ().end();
//    }
	virtual void serialize(QDataStream &chainOut) const;
	virtual void deserialize(QDataStream &chainOut);
	virtual void serialize(QTextStream &chainOut) const;
};
class NameConnectorPrim :public NarratorPrim {
public:

	NameConnectorPrim(QString * hadith_text);
	NameConnectorPrim(QString * hadith_text,long long m_start);

//    virtual CNPIterator first () const{
//    return QList<ChainNarratorPrim*> ().begin();
//        }
//
//    virtual CNPIterator end () const{
//    return QList<ChainNarratorPrim*> ().end();
//    }
	virtual void serialize(QDataStream &chainOut) const;
	virtual void deserialize(QDataStream &chainOut);
	virtual void serialize(QTextStream &chainOut) const;
};
class NarratorConnectorPrim :public ChainPrim {
public:
    long long m_start;
    long long m_end;
	NarratorConnectorPrim(QString * hadith_text);
	NarratorConnectorPrim(QString * hadith_text,long long m_start);

//    virtual CNPIterator first () const{
//    return QList<ChainNarratorPrim*> ().begin();
//        }
//
//    virtual CNPIterator end () const{
//    return QList<ChainNarratorPrim*> ().end();
//    }
	virtual void serialize(QDataStream &chainOut) const;
	virtual void deserialize(QDataStream &chainOut);
	virtual void serialize(QTextStream &chainOut) const;
};
class Narrator :public ChainPrim {
public:
	Narrator(QString * hadith_text);
 //   QList <ChainNarratorPrim *> m_narrator; //modify back to here
   QList <NarratorPrim *> m_narrator;
//    virtual CNPIterator first () {
//    return m_narrator.begin();
//        }
//
//    virtual CNPIterator end () const{
//    //return m_narrator.end();
//    }
	virtual void serialize(QDataStream &chainOut) const;
	virtual void deserialize(QDataStream &chainOut);
	virtual void serialize(QTextStream &chainOut) const;
};
class Chain: public ChainNarratorPrim {
public:
	Chain(QString * hadith_text);
	//QList <ChainNarratorPrim *> m_chain;//to be reverted to
    QList <ChainPrim *> m_chain;
	virtual void serialize(QDataStream &chainOut) const;
	virtual void deserialize(QDataStream &chainOut);
	virtual void serialize(QTextStream &chainOut) const;
//    virtual CNPIterator first () const{
//    //return m_chain.begin();
//        }
//
//    virtual CNPIterator end () const{
//    //return m_chain.end();
//    }

};

QDataStream &operator>>(QDataStream &in, ChainNarratorPrim &p);

QDataStream &operator<<(QDataStream &out, const ChainNarratorPrim &p);

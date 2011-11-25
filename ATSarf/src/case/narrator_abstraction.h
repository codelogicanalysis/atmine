#ifndef _NarrAbs_h
#define _NarrAbs_h

#include <QList>
#include <QTextStream>
#include <QDataStream>
#include "common.h"
#include <QFile>
#include <QDebug>
#include "logger.h"
#include "text_handling.h"

#define SHOW_AS_TEXT


class ChainNarratorPrim;
class Chain;
typedef QList<Chain *> ChainsContainer;

extern int segmentNarrators(QString * text,int (*functionUsingChains)(ChainsContainer &, ATMProgressIFC *, QString), ATMProgressIFC *prg);
extern int segmentNarrators(QString * text,int start, int end, int (*functionUsingChains)(ChainsContainer &, ATMProgressIFC *, QString), ATMProgressIFC *prg);


//typedef QList <ChainNarratorPrim *>::iterator CNPIterator;

//#enumerate the types

class ChainNarratorPrim {
public://protected:
	QString * hadith_text;
public:
//    virtual CNPIterator first () const=0;
//    virtual CNPIterator end () const=0;
	virtual void serialize(QDataStream &chainOut) const=0;
	virtual void deserialize(QDataStream &chainOut)=0;
	virtual void serialize(QTextStream &chainOut) const=0;

    virtual QString getString() const {
		int length=getLength();
		if (length<0)
			return "";
		return hadith_text->mid(getStart(), length);
    }
    virtual int getStart() const = 0;
    virtual int getLength() const = 0;
    virtual int getEnd() const = 0;
 //   virtual int startPosition() const=0;
 //   virtual int lastPosition() const=0;

};

class NarratorPrim: public ChainNarratorPrim {
public:
	int m_start;
	int m_end;

	NarratorPrim(QString * hadith_text);
	NarratorPrim(QString * hadith_text,int m_start);

 //   virtual CNPIterator first () const=0;
 //   virtual CNPIterator end () const=0;
	virtual bool isNamePrim() const =0;
	virtual void serialize(QDataStream &chainOut) const=0;
	virtual void deserialize(QDataStream &chainOut)=0;
	virtual void serialize(QTextStream &chainOut) const=0;
    virtual int getStart() const {
        return m_start;}
    virtual int getLength() const {
        return m_end - m_start + 1;}
    virtual int getEnd() const {
        return m_end;}

};
class ChainPrim :public ChainNarratorPrim {
public:
	ChainPrim(QString* hadith_text);
 //   virtual CNPIterator first () const=0;
//    virtual CNPIterator end () const=0;
	virtual bool isNarrator() const=0;
	virtual void serialize(QDataStream &chainOut) const=0;
	virtual void deserialize(QDataStream &chainOut)=0;
	virtual void serialize(QTextStream &chainOut) const=0;
    virtual int getStart() const = 0;
    virtual int getLength() const =0;
    virtual int getEnd() const = 0;
};
class NamePrim :public NarratorPrim {
public:
	bool learnedName;
	NamePrim(QString * hadith_text);
	NamePrim(QString * hadith_text,int m_start);

   // virtual CNPIterator first () const{
   // return QList<ChainNarratorPrim*> ().begin();
  //      }

//    virtual CNPIterator end () const{
//    return QList<ChainNarratorPrim*> ().end();
//    }
	virtual bool isNamePrim() const;
	virtual void serialize(QDataStream &chainOut) const;
	virtual void deserialize(QDataStream &chainOut);
	virtual void serialize(QTextStream &chainOut) const;
	bool operator ==(const NamePrim & name) const {
		return equal_withoutLastDiacritics(getString(),name.getString());
	}
};
inline uint qHash(const NamePrim & name) {
	return qHash(removeDiacritics(name.getString()));
}

class NameConnectorPrim :public NarratorPrim {
private:
	enum Type {POS,IBN,AB,OM,FAMILY_OTHER, OTHER};
	Type type;
public:
	void setPossessive(){type=POS;}
	bool isPossessive(){return type==POS;}
	void setIbn(){type=IBN;}
	bool isIbn(){return type==IBN;}
	void setAB(){type=AB;}
	bool isAB(){return type==AB;}
	void setOM(){type=OM;}
	bool isOM(){return type==OM;}
	void setFamilyConnector(){if (type!=IBN && type!=OM && type!=AB) type=FAMILY_OTHER;}
	bool isFamilyConnector(){return type==IBN || type==OM || type==AB || type==FAMILY_OTHER;}
	void setOther(){type=OTHER;}
	bool isOther(){return type==OTHER;}
	NameConnectorPrim(QString * hadith_text);
	NameConnectorPrim(QString * hadith_text,int m_start);

//    virtual CNPIterator first () const{
//    return QList<ChainNarratorPrim*> ().begin();
//        }
//
//    virtual CNPIterator end () const{
//    return QList<ChainNarratorPrim*> ().end();
//    }
	virtual bool isNamePrim() const;
	virtual void serialize(QDataStream &chainOut) const;
	virtual void deserialize(QDataStream &chainOut);
	virtual void serialize(QTextStream &chainOut) const;
};
class NarratorConnectorPrim :public ChainPrim {
public:
	int m_start;
	int m_end;
	NarratorConnectorPrim(QString * hadith_text);
	NarratorConnectorPrim(QString * hadith_text,int m_start);

//    virtual CNPIterator first () const{
//    return QList<ChainNarratorPrim*> ().begin();
//        }
//
//    virtual CNPIterator end () const{
//    return QList<ChainNarratorPrim*> ().end();
//    }
	virtual bool isNarrator() const;
	virtual void serialize(QDataStream &chainOut) const;
	virtual void deserialize(QDataStream &chainOut);
	virtual void serialize(QTextStream &chainOut) const;

    virtual int getStart() const {
        return m_start;}
    virtual int getLength() const {
        return m_end - m_start + 1;}
    virtual int getEnd() const {
        return m_end;}
};

inline bool possessivesCompare(const NameConnectorPrim * n1,const NameConnectorPrim * n2) {
	return n1->getString()<n2->getString();
}

class Narrator : public ChainPrim{
public:
	typedef QVector<NarratorPrim *> NarratorPrimList;
	typedef QVector<NarratorPrimList> NarratorPrimHierarchy;
	typedef QVector<NameConnectorPrim *> PossessiveList;
public:
	Narrator(QString * hadith_text);
 //   QList <ChainNarratorPrim *> m_narrator; //modify back to here
   QList <NarratorPrim *> m_narrator;
	bool isRasoul; //if it is known this node is rasoul node
//    virtual CNPIterator first () {
//    return m_narrator.begin();
//        }
//
//    virtual CNPIterator end () const{
//    //return m_narrator.end();
//    }
	virtual bool isNarrator() const;
	virtual void serialize(QDataStream &chainOut) const;
	virtual void deserialize(QDataStream &chainOut);
	virtual void serialize(QTextStream &chainOut) const;

    virtual int getStart() const {
        return m_narrator[0]->m_start;}

    virtual int getLength() const {
        int last = m_narrator.size() - 1;
		if (last==-1)
			return -1;
        int start = getStart();
        return m_narrator[last]->m_end - start + 1;}

    virtual int getEnd() const {
        int last = m_narrator.size() - 1;
        return m_narrator[last]->m_end;}

    virtual bool operator == (Narrator & rhs) const {
        return getString() == rhs.getString(); }

	virtual double equals(const Narrator & rhs) const;

	void preProcessForEquality(NarratorPrimHierarchy & names,PossessiveList & possessives) const{
		int j=0; //index of names entry defined by bin
		names.append(NarratorPrimList());
		//qDebug()<< names.size();
		for (int i=0;i<m_narrator.count();i++) {
			if (m_narrator[i]->getString().isEmpty())
				continue;
			if (m_narrator[i]->isNamePrim()){
				names[j].append(m_narrator[i]);
			} else {
				NameConnectorPrim * c=(NameConnectorPrim*)m_narrator[i];
				if (c->isPossessive()) {
					possessives.append(c);
				} else if (c->isIbn()){
					names.append(NarratorPrimList());
					//qDebug()<<"\t"<<c->getString();
					j++;
				} else if (c->isFamilyConnector()) {
					names[j].append(c);
				}
				//if (c->isOther()) do nothing
			}
		}
		//TODO: add sorting of names inside same level (pay attention to those after family connector) and possessives
		qSort(possessives.begin(),possessives.end(),possessivesCompare);
		//TODO: allow for skipping in the selection if total number is conserved
	}

	QString getKey() const;
};

class Chain: public ChainNarratorPrim {
public:
	Chain(QString * hadith_text);
	//QList <ChainNarratorPrim *> m_chain;//to be reverted to
    QList <ChainPrim *> m_chain;
	virtual void serialize(QDataStream &chainOut) const;
	virtual void deserialize(QDataStream &chainOut);
	virtual void serialize(QTextStream &chainOut) const;

    virtual int getStart() const {
        return m_chain[0]->getStart(); }

    virtual int getLength() const {
        int last = m_chain.size() - 1;
        int start = getStart();
        return m_chain[last]->getEnd() - start + 1;}

    virtual int getEnd() const {
        int last = m_chain.size() - 1;
        return m_chain[last]->getEnd();}

    virtual bool operator == (Chain & rhs) const {
        return getString() == rhs.getString(); }

//    virtual CNPIterator first () const{
//    //return m_chain.begin();
//        }
//
//    virtual CNPIterator end () const{
//    //return m_chain.end();
//    }

};
class NarratorNodeIfc;
class NarratorGraph;
class Biography {
public:
	class BiographyNarrator{
	public:
		BiographyNarrator(){
			narrator=NULL;
			isRealNarrator=false;
		}
		BiographyNarrator(Narrator * narrator) {
			this->narrator=narrator;
			isRealNarrator=false;
		}
		Narrator * narrator;
		bool isRealNarrator; //i.e. found in POR
		~BiographyNarrator() {
			if (narrator!=NULL)
				delete narrator;
		}
	};
	typedef QVector<BiographyNarrator *> NarratorVector;
private:

	QString * text;
	int start, end;

	NarratorVector narrators;
#ifdef SEGMENT_BIOGRAPHY_USING_POR
private:
	NarratorGraph * graph;
	bool isRealNarrator(Narrator * n);
public:

	class MatchingNode {
	public:
		NarratorNodeIfc * node;
		double similarity;
		Narrator * bioNarrator;
		MatchingNode(NarratorNodeIfc * node,double similarity,Narrator * bioNarrator=NULL) {
			this->node=node;
			this->similarity=similarity;
			this->bioNarrator=bioNarrator;
		}
		static bool contains(const QList<MatchingNode> & list, NarratorNodeIfc * node, double similarity, int & index) {
			//if found with a larger similarity will return true and index=-1,
			//if found with a smaller similarity will return true and index = index of the found
			//if not found return false and index=-1
			int size=list.size();
			for (int i=0;i<size;i++) {
				if (list[i].node==node) {
					if (list[i].similarity>=similarity) {
						index=-1;
						return true;
					} else {
						index=i;
						return true;
					}
				}
			}
			index=-1;
			return false;
		}
		bool operator <(const MatchingNode & rhs) const {
			return similarity>rhs.similarity; //to get descending order
		}
	};
	typedef QList<MatchingNode> NarratorNodeList;
	typedef QList<NarratorNodeList> NarratorNodeGroups;
	NarratorNodeGroups nodeGroups;
#endif
public:
	//Biography() { this->text=NULL; start=0;end=0;}
	Biography(NarratorGraph * graph,QString * text) {
	#ifdef SEGMENT_BIOGRAPHY_USING_POR
		this->graph=graph;
	#endif
		this->text=text;
		start=0;
		end=0;
	}
	Biography(NarratorGraph * graph,QString * text,long start) {
	#ifdef SEGMENT_BIOGRAPHY_USING_POR
		this->graph=graph;
	#endif
		this->text=text;
		this->start=start;
		end=start;
	}
	Biography(NarratorGraph * graph,QString * text,long start, long end) {
	#ifdef SEGMENT_BIOGRAPHY_USING_POR
		this->graph=graph;
	#endif
		this->text=text;
		this->start=start;
		this->end=end;
	}
	void setStart(long s) { start=s;}
	void setEnd(long e) { end=e; }
	int getStart() const {return start;}
	int getLength() const {	return end - start + 1;}
	virtual int getEnd() const { return end;}
	QString getString() const {
		int length=getLength();
		if (length<0)
			return "";
		return text->mid(getStart(), length);
	}
private:
	bool addNarratorHelper(Narrator * n) {
		bool real=false;
		BiographyNarrator * b =new BiographyNarrator(n);
	#ifdef SEGMENT_BIOGRAPHY_USING_POR
		b->isRealNarrator=isRealNarrator(n);
		real=b->isRealNarrator;
	#endif
		narrators.append(b);
		return real;
	}
	void modifyBoundaries(Narrator * n) {
		int start=n->getStart();
		int end=n->getEnd();
		if (start<this->start)
			this->start=start;
		if (end>this->end)
			this->end=end;
	}
public:
	bool addNarrator(Narrator & n) {
		if (n.getStart()>=this->start) {// && n.getEnd()<=this->end)
			return addNarratorHelper(&n);
		}
		return false;
	}
	bool addNarratorAndUpdateBoundaries(Narrator * n) {
		if (n==NULL)
			return false;
		bool real=addNarratorHelper(n);
		modifyBoundaries(n);
		return real;
	}

	bool addNarrator(Narrator * n) {
		if (n!=NULL)
			return addNarrator(*n);
		return false;
	}
	void addRealNarrator(Narrator * n) {
		assert(n!=NULL);
		BiographyNarrator * b =new BiographyNarrator(n);
	#ifdef SEGMENT_BIOGRAPHY_USING_POR
		b->isRealNarrator=true;
	#endif
		modifyBoundaries(n);
		narrators.append(b);
	}

	int size() {
		return narrators.size();
	}
	Narrator * operator [] (int i) {
		assert(i>=0 && i<narrators.size());
		return narrators[i]->narrator;
	}
	bool isReal(int i) {
		assert(i>=0 && i<narrators.size());
		return narrators[i]->isRealNarrator;
	}

	//const NarratorVector & getNarrators() const { return narrators;}
	Narrator * getLastNarrator() {
		if (narrators.size()>0)
			return narrators[narrators.size()-1]->narrator;
		else
			return NULL;
	}
	void removeNarrator(int i) {
		narrators.remove(i);
	}
	void serialize(QDataStream &chainOut) const;
	void deserialize(QDataStream &chainIn);
	void serialize(QTextStream &chainOut) const;
	~Biography(){ //TODO: check there are no memory leaks
		/*for (int i=0;i<narrators.size();i++) {
			delete narrators[i];
		}*/
	}
};


double equal(const Narrator & n1,const Narrator & n2);

QDataStream &operator>>(QDataStream &in, ChainNarratorPrim &p);
QDataStream &operator<<(QDataStream &out, const ChainNarratorPrim &p);
#endif

#ifndef CHAIN_GRAPH_H
#define CHAIN_GRAPH_H

#include "Triplet.h"
#include <assert.h>
#include <QList>

class NarratorNodeIfc;
class ChainNarratorNode;
class ChainNarratorNodeIterator;
class GraphNarratorNode;

class Chain;
class Narrator;
class ChainPrim;

typedef QList<Chain *> ChainsContainer;
typedef QPair<NarratorNodeIfc &, ChainNarratorNodeIterator &> NodeAddress;


#define nullNodeAddress NodeAddress(nullNarratorNodeIfc,nullChainNarratorNodeIterator)
extern ChainsContainer chains;

void fillRank(Narrator &n, int index, bool last);
void fillRanks();

class NarratorNodeIfc //abstract interface
{
public:
	virtual NarratorNodeIfc & firstChild()=0; //TODO: change to return a reference
	virtual NarratorNodeIfc & nextChild(NarratorNodeIfc & current)=0;
	virtual NarratorNodeIfc & firstParent()=0;
	virtual NarratorNodeIfc & nextParent(NarratorNodeIfc & current)=0;

	virtual ChainNarratorNodeIterator & firstNarrator()=0;
	virtual ChainNarratorNodeIterator & nextNarrator(ChainNarratorNodeIterator & current)=0;

	virtual NodeAddress prevInChain(ChainNarratorNodeIterator &)=0;
	virtual NodeAddress nextInChain(ChainNarratorNodeIterator &)=0;

	virtual QString CanonicalName()=0;

	virtual bool isNull(){return false;}

	NarratorNodeIfc & operator*()
	{
		return *this;
	}
};

class NULLNarratorNodeIfc: public NarratorNodeIfc
{
	NarratorNodeIfc & firstChild();
	NarratorNodeIfc & nextChild(NarratorNodeIfc & current);
	NarratorNodeIfc & firstParent();
	NarratorNodeIfc & nextParent(NarratorNodeIfc & current);

	ChainNarratorNodeIterator & firstNarrator();
	ChainNarratorNodeIterator & nextNarrator(ChainNarratorNodeIterator & current);

	NodeAddress prevInChain(ChainNarratorNodeIterator &);
	NodeAddress nextInChain(ChainNarratorNodeIterator &);

	QString CanonicalName(){return QString::null;}

	virtual bool isNull(){return true;}
};

extern NULLNarratorNodeIfc nullNarratorNodeIfc;

class ChainNarratorNode
{
private:
	GraphNarratorNode * narrNode;
public:
	NarratorNodeIfc & getCorrespondingNarratorNode()
	{
		return *(NarratorNodeIfc *)narrNode;
	}
	void  setCorrespondingNarratorNode(GraphNarratorNode * narrNode)
	{
		this->narrNode=narrNode;
	}
	QString CanonicalName();
};

class ChainNarratorNodeIterator:public QList<ChainPrim *>::iterator, public NarratorNodeIfc //Maybe later form by composition instead of inheretence
{
private:
	ChainPrim* getChainPrimPtr()
	{
		return (ChainPrim*)(*(QList<ChainPrim *>::iterator)*this);
	}
	ChainPrim & getChainPrim()
	{
		return *(ChainPrim*)(*(QList<ChainPrim *>::iterator)*this);
	}
public:
	ChainNarratorNodeIterator(){}
	ChainNarratorNodeIterator(Chain *ch, Narrator * n);
	ChainNarratorNodeIterator(QList<ChainPrim *>::iterator it);
	ChainNarratorNodeIterator & firstNarratorInChain();//TODO: create firstNarrator() equivalent to begin
	Narrator & getNarrator()
	{
		return (Narrator &)getChainPrim();
	}
	ChainNarratorNode & operator*();
	ChainNarratorNode * operator->()
	{
		return &this->operator *();
	}
	ChainNarratorNodeIterator & operator++();
	ChainNarratorNodeIterator & operator--();
	ChainNarratorNodeIterator & operator+(int);
	ChainNarratorNodeIterator & operator-(int);
	ChainNarratorNodeIterator & prevInChain()
	{
		return (this->operator --());
	}
	NarratorNodeIfc & firstChild();
	NarratorNodeIfc & nextChild(NarratorNodeIfc & )
	{
		return nullNarratorNodeIfc;
	}
	NarratorNodeIfc & firstParent();
	NarratorNodeIfc & nextParent(NarratorNodeIfc & )
	{
		return nullNarratorNodeIfc;
	}
	ChainNarratorNodeIterator & firstNarrator();
	ChainNarratorNodeIterator & nextNarrator(ChainNarratorNodeIterator &);
	NodeAddress prevInChain(ChainNarratorNodeIterator & node);
	NodeAddress nextInChain(ChainNarratorNodeIterator & node);
	ChainNarratorNodeIterator & nextInChain()
	{
		return (this->operator ++());
	}
	QString CanonicalName()
	{
		return this->operator *().CanonicalName();
	}
	NarratorNodeIfc &  getCorrespondingNarratorNode()
	{
		NarratorNodeIfc & narrNode=this->operator *().getCorrespondingNarratorNode();
		if ( narrNode.isNull())
			return *this;
		else
			return *narrNode;
	}
	bool isFirst();
	bool isLast();
	int getIndex();
	int getChainNum();
	virtual bool isNULL()
	{
		return false;
	}
};

class NULLChainNarratorNodeIterator: public ChainNarratorNodeIterator
{
public:
	virtual bool isNULL()
	{
		return true;
	}
};

extern NULLChainNarratorNodeIterator nullChainNarratorNodeIterator;

class GraphNarratorNode: public NarratorNodeIfc
{
private:
	QList<ChainNarratorNodeIterator>  equalnarrators;
public:
	GraphNarratorNode(ChainNarratorNodeIterator & nar1,ChainNarratorNodeIterator & nar2);
	GraphNarratorNode(Chain * chain1, Narrator * nar1,Chain * chain2, Narrator * nar2);
	void addNarrator(Chain * chain1, Narrator * nar1);
	void addNarrator(ChainNarratorNodeIterator & nar1);
	NarratorNodeIfc & firstChild() //return iterator instead of NarratorNode
	{
		return (*((*(equalnarrators.begin())).nextInChain())).getCorrespondingNarratorNode();
	}
	NarratorNodeIfc & nextChild(NarratorNodeIfc & current)
	{
		assert(current.firstNarrator().isNULL());//=>  is ChainNarratorNodePtr
		//check if end reached
		if (!(((ChainNarratorNodeIterator &)current).isLast()))
			return current.nextNarrator((ChainNarratorNodeIterator&)current).nextInChain().getCorrespondingNarratorNode();
		else
			return nullNarratorNodeIfc;
	}

	NarratorNodeIfc & firstParent()
	{
		return (*((*(equalnarrators.begin())).prevInChain())).getCorrespondingNarratorNode();
	}
	NarratorNodeIfc & nextParent(NarratorNodeIfc & current)
	{
		assert(current.firstNarrator().isNULL());//=>  is ChainNarratorNodePtr
		//check if end reached
		if (!(((ChainNarratorNodeIterator &)current).isFirst()))
			return current.nextNarrator((ChainNarratorNodeIterator &)current).prevInChain().getCorrespondingNarratorNode();
		else
			return nullNarratorNodeIfc;
	}
	ChainNarratorNodeIterator & firstNarrator()
	{
		return *equalnarrators.begin();
	}
	ChainNarratorNodeIterator & nextNarrator(ChainNarratorNodeIterator & current)
	{
		if (!(current.isLast()))
			return current.nextNarrator(current);
		else
			return nullChainNarratorNodeIterator;
	}

	NodeAddress prevInChain(ChainNarratorNodeIterator & node)
	{
		ChainNarratorNodeIterator prev=node.prevInChain();
		return NodeAddress(prev->getCorrespondingNarratorNode(), prev);
	}
	NodeAddress nextInChain(ChainNarratorNodeIterator & node)
	{
		ChainNarratorNodeIterator next=node.nextInChain();
		return NodeAddress(next->getCorrespondingNarratorNode (), next);
	}

	QString CanonicalName()
	{
		int smallestsize=0, index=-1;
		for (int i=0;i<equalnarrators.size();i++)
		{
			int size=equalnarrators[i].CanonicalName().size();
			if (smallestsize>size)
			{
				smallestsize=size;
				index=i;
			}
		}
		if (index>0)
			return equalnarrators[index].CanonicalName();
		else
			return "";
	}

};

int test_NarratorEquality(QString input_str);//just for testing purposes

#endif // CHAIN_GRAPH_H

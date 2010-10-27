#ifndef CHAIN_GRAPH_H
#define CHAIN_GRAPH_H

#include "Triplet.h"
#include <assert.h>
#include <QList>


/*
GHGNarratorNode
{list of equal narrators pointer, cannonical_name, functions for:{(parent,children)}
use this node structure when merging the 2 hadith chains
hash them according to wither elements or string stripped and so on (for testing)
  */

class NarratorNode;
class ChainNarratorNode;
class ChainNarratorNodePtr;
class GraphNarratorNode;

class Chain;
class Narrator;
class ChainPrim;

typedef QPair<NarratorNode *, ChainNarratorNodePtr> NodeAddress;

class NarratorNode //abstract interface
{
public:
	virtual NarratorNode * firstChild()=0;
	virtual NarratorNode * nextChild(NarratorNode * current)=0;
	virtual NarratorNode * firstParent()=0;
	virtual NarratorNode * nextParent(NarratorNode * current)=0;

	virtual ChainNarratorNodePtr firstNarrator()=0;
	virtual ChainNarratorNodePtr nextNarrator(ChainNarratorNodePtr current)=0;

	virtual NodeAddress prevInChain(ChainNarratorNodePtr)=0;
	virtual NodeAddress nextInChain(ChainNarratorNodePtr)=0;

	virtual QString CanonicalName()=0;
};

class ChainNarratorNode //TODO: merge this with Narrator class
{
private:
	GraphNarratorNode * narrNode;
public:
	NarratorNode * getCorrespondingNarratorNode()
	{
		return (NarratorNode *)narrNode;
	}
	void  setCorrespondingNarratorNode(GraphNarratorNode * narrNode)
	{
		this->narrNode=narrNode;
	}
	QString CanonicalName();
};

class ChainNarratorNodePtr:public QList<ChainPrim *>::iterator, public NarratorNode
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
	ChainNarratorNodePtr(){}
	ChainNarratorNodePtr(Chain *ch, Narrator * n);
	ChainNarratorNode & operator*();
	ChainNarratorNode * operator->()
	{
		return &this->operator *();
	}
	ChainNarratorNodePtr & operator++();
	ChainNarratorNodePtr & operator--();
	ChainNarratorNodePtr & prevInChain()
	{
		return (this->operator --());
	}
	NarratorNode * firstChild();
	NarratorNode * nextChild(NarratorNode * )
	{
		return NULL;
	}
	NarratorNode * firstParent();
	NarratorNode * nextParent(NarratorNode * )
	{
		return NULL;
	}
	ChainNarratorNodePtr firstNarrator();
	ChainNarratorNodePtr nextNarrator(ChainNarratorNodePtr);
	NodeAddress prevInChain(ChainNarratorNodePtr node);
	NodeAddress nextInChain(ChainNarratorNodePtr node);
	ChainNarratorNodePtr & nextInChain()
	{
		return (this->operator ++());
	}
	QString CanonicalName()
	{
		return this->operator *().CanonicalName();
	}
	NarratorNode *  getCorrespondingNarratorNode()
	{
		NarratorNode * narrNode=this->operator *().getCorrespondingNarratorNode();
		if ( narrNode==NULL)
			return this;
		else
			return narrNode;
	}
	bool isFirst();
	bool isLast();
	int getIndex();
	virtual bool isNULL()
	{
		return false;
	}
};

class NULLChainNarratorNodePtr: public ChainNarratorNodePtr
{
public:
	virtual bool isNULL()
	{
		return true;
	}
};

class GraphNarratorNode: public NarratorNode
{
private:
	QList<ChainNarratorNodePtr>  equalnarrators;
public:
	GraphNarratorNode(ChainNarratorNodePtr nar1,ChainNarratorNodePtr nar2);
	GraphNarratorNode(Chain * chain1, Narrator * nar1,Chain * chain2, Narrator * nar2);
	void addNarrator(Chain * chain1, Narrator * nar1);
	void addNarrator(ChainNarratorNodePtr nar1);
	NarratorNode * firstChild() //return iterator instead of NarratorNode
	{
		return (*((*(equalnarrators.begin())).nextInChain())).getCorrespondingNarratorNode();
	}
	NarratorNode * nextChild(NarratorNode * current)
	{
		assert(current->firstNarrator().isNULL());//=>  is ChainNarratorNodePtr
		//check if end reached
		if (!(((ChainNarratorNodePtr *)current)->isLast()))
			return current->nextNarrator(*(ChainNarratorNodePtr*)current).nextInChain().getCorrespondingNarratorNode();
		else
			return NULL;
	}

	NarratorNode * firstParent()
	{
		return (*((*(equalnarrators.begin())).prevInChain())).getCorrespondingNarratorNode();
	}
	NarratorNode * nextParent(NarratorNode * current)
	{
		assert(current->firstNarrator().isNULL());//=>  is ChainNarratorNodePtr
		//check if end reached
		if (!(((ChainNarratorNodePtr *)current)->isFirst()))
			return current->nextNarrator(*(ChainNarratorNodePtr*)current).prevInChain().getCorrespondingNarratorNode();
		else
			return NULL;
	}
	ChainNarratorNodePtr firstNarrator()
	{
		return *equalnarrators.begin();
	}
	ChainNarratorNodePtr nextNarrator(ChainNarratorNodePtr current)
	{
		if (!(current.isLast()))
			return current.nextNarrator(current);
		else
			return NULLChainNarratorNodePtr();
	}

	NodeAddress prevInChain(ChainNarratorNodePtr node)
	{
		ChainNarratorNodePtr prev=node.prevInChain();
		return NodeAddress(prev->getCorrespondingNarratorNode(), prev);
	}
	NodeAddress nextInChain(ChainNarratorNodePtr node)
	{
		ChainNarratorNodePtr next=node.nextInChain();
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

#endif // CHAIN_GRAPH_H

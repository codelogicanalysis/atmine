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

class ChainNarratorNode: public NarratorNode //TODO: merge this with Narrator class
{
private:
	GraphNarratorNode * narrNode;
public:

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
	virtual ChainNarratorNodePtr & prevInChain()=0;
	virtual ChainNarratorNodePtr & nextInChain()=0;
	NarratorNode *  getCorrespondingNarratorNode();
	QString CanonicalName();
};

class ChainPrim;

class ChainNarratorNodePtr:public QList<ChainPrim *>::iterator, public ChainNarratorNode
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
	ChainNarratorNode & operator*();
	ChainNarratorNode * operator->()
	{
		return &this->operator *();
	}
	ChainNarratorNodePtr & operator++();
	ChainNarratorNodePtr & operator--();
	virtual ChainNarratorNodePtr & prevInChain()
	{
		return (this->operator --());
	}
	virtual ChainNarratorNodePtr & nextInChain()
	{
		return (this->operator ++());
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

	NarratorNode * firstChild() //return iterator instead of NarratorNode
	{
		return (*(equalnarrators.begin()))->prevInChain()->getCorrespondingNarratorNode();
	}
	NarratorNode * nextChild(NarratorNode * current)
	{
		/*return qualnarrators[equalnarrators.find(current).getIndex()+1]
				.getPrevInChain().getCorrespondingNarratorNode(); //inefficient*/
	}

	NarratorNode * firstParent();
	NarratorNode * nextParent(NarratorNode * current);//similar to above

	ChainNarratorNodePtr firstNarrator(); //similar to above
	ChainNarratorNodePtr nextNarrator(ChainNarratorNode * current);

	NodeAddress prevInChain(ChainNarratorNode * node)
	{
		ChainNarratorNodePtr prev=node->prevInChain();
		return NodeAddress(prev->getCorrespondingNarratorNode(), prev);
	}
	NodeAddress nextInChain(ChainNarratorNode * node)
	{
		ChainNarratorNodePtr next=node->nextInChain();
		return NodeAddress(next->getCorrespondingNarratorNode (), next);
	}

	QString CanonicalName()
	{
	//return smallest among names
	}

};

#endif // CHAIN_GRAPH_H

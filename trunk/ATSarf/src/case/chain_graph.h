#ifndef CHAIN_GRAPH_H
#define CHAIN_GRAPH_H

#include "narrator_abstraction.h"
#include "Triplet.h"

class CNarratorNodePtr:public QList<ChainPrim *>::iterator
{
private:
	Chain * chain;
public:
	CNarratorNodePtr(Chain * chain):QList<ChainPrim *>::iterator(chain->m_chain.begin())
	{
		this->chain=chain;
	}
	ChainPrim * begin()//isFirst
	{
		return (chain->m_chain.count()>0?chain->m_chain[0]:NULL);
	}
	ChainPrim * end()//isLast
	{
		return (chain->m_chain.count()>0?chain->m_chain[chain->m_chain.count()-1]:NULL);
	}
	Narrator * nextNarrator()
	{
		ChainPrim * n;
		while (!( n=next())->isNarrator() && n!=end())
			;
		return (Narrator*)n;
	}
	Narrator * previousNarrator()
	{
		ChainPrim * n;
		while (!( n=previous())->isNarrator() && n!=begin())
			;
		return (Narrator*)n;
	}
	ChainPrim * next()
	{
		if ((ChainPrim*)(*(QList<ChainPrim *>::iterator)*this)!=end())
			return (ChainPrim*)(*(QList<ChainPrim *>::iterator)(++(*this)));
		else
			return (ChainPrim*)(*(QList<ChainPrim *>::iterator)*this);

	}
	ChainPrim * previous()
	{
		if ((ChainPrim*)(*(QList<ChainPrim *>::iterator)*this)!=begin())
			return (ChainPrim*)(*(QList<ChainPrim *>::iterator)(--(*this)));
		else
			return ((ChainPrim*)(*(QList<ChainPrim *>::iterator)*this));
	}
};


/*
GHGNarratorNode
{list of equal narrators pointer, cannonical_name, functions for:{(parent,children)}
use this node structure when merging the 2 hadith chains
hash them according to wither elements or string stripped and so on (for testing)
  */

class NarratorNode;
class ChainNarratorNode;

typedef QPair<NarratorNode *, ChainNarratorNode *> NodeAddress;

NarratorNode //abstract interface
{
	NarratorNode * firstChild()=0;
	NarratorNode * nextChild(NarratorNode * current)=0;
	NarratorNode * firstParent()=0;
	NarratorNode * nextParent(NarratorNode * current)=0;

	ChainNarratorNode * firstNarrator()=0;
	ChainNarratorNode * nextNarrator(ChainNarratorNode * current)=0;

	<NarratorNode *, ChainNarratorNode *> prevInChain(ChainNarratorNode *)=0;
	<NarratorNode *, ChainNarratorNode *> nextInChain(ChainNarratorNode *)=0;

	QString CanonicalName()=0;
};

ChainNarratorNode: public NarratorNode //TODO: merge this with Narrator class
{
private:
	NarratorNode * narrNode;
public:

	NarratorNode * firstChild()
	{
		return  nextInChain();
	}
	NarratorNode * nextChild(NarratorNode * current)
	{
		return null;
	}

	NarratorNode * firstParent()
	{
		return  prevInChain();
	}
	NarratorNode * nextParent(NarratorNode * current)
	{
		return null;
	}

	ChainNarratorNode * firstNarrator()
	{
		return this;
	}
	ChainNarratorNode * nextNarrator(ChainNarratorNode * current)
	{
		return null;
	}

	<NarratorNode *, ChainNarratorNode *> prevInChain(ChainNarratorNode * node)
	{
		assert (node==this);
		ChainNarratorNode * prev=node.prevInChain();
		return NodeAddress(prev.getCorrespondingNarratorNode(), prev);
	}
	<NarratorNode *, ChainNarratorNode *> nextInChain(ChainNarratorNode * node)
	{
		assert (node==this);
		ChainNarratorNode * next=node.nextInChain();
		return NodeAddress(next.getCorrespondingNarratorNode (), next);
	}

	<ChainNarratorNode *> prevInChain();
	<ChainNarratorNode *> nextInChain();

	NarratorNode *  getCorrespondingNarratorNode()
	{
		if ( narrNode==NULL)
			return this;
		else
			return narrNode;
	}

	QString CanonicalName()
	{
		return this.getText();//change to get Narrator text
	}
};

	GraphNarratorNode: public NarratorNode
{
private:
	Qlist<ChainNarratorNode> * equalnarrators;
public:

	NarratorNode * firstChild() //return iterator instead of NarratorNode
	{
		return  equalnarrators[0].getPrevInChain().getCorrespondingNarratorNode();
	}
	NarratorNode * nextChild(NarratorNode * current)
	{
		return qualnarrators[equalnarrators.find(current).getIndex()+1]
				.getPrevInChain().getCorrespondingNarratorNode(); //inefficient
	}

	NarratorNode * firstParent();
	NarratorNode * nextParent(NarratorNode * current);//similar to above

	ChainNarratorNode * firstNarrator(); //similar to above
	ChainNarratorNode * nextNarrator(ChainNarratorNode * current);

	<NarratorNode *, ChainNarratorNode *> prevInChain(ChainNarratorNode * node)
	{
		ChainNarratorNode * prev=node.prevInChain();
		return NodeAddress(prev.getCorrespondingNarratorNode(), prev);
	}
	<NarratorNode *, ChainNarratorNode *> nextInChain(ChainNarratorNode * node)
	{
		ChainNarratorNode * next=node.nextInChain();
		return NodeAddress(next.getCorrespondingNarratorNode (), next);
	}

		QString CanonicalName()
	{
	//return smallest among names
	}

}

#endif // CHAIN_GRAPH_H

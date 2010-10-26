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

class GHGNarratorNode;

typedef QPair<GHGNarratorNode*,CNarratorNodePtr*> NodeAddress;

class GHGNarratorNode
{
private:
	QList<CNarratorNodePtr*> equalNarrators;
public:
	GHGNarratorNode();
	int addNarrator(Narrator * n);
	QList<NodeAddress> getParents();
	QList<NodeAddress> getChildren();
	QList<CNarratorNodePtr *> getEqualNarrators();
	QString getCanonicalName();
	NodeAddress getParent(Narrator* narrator);
	NodeAddress getChild(Narrator* narrator);
	Narrator* getNarrator (int index);
	NodeAddress getParentAddress(int index);
	NodeAddress getChildAddress (int index);
	~GHGNarratorNode();
};

#endif // CHAIN_GRAPH_H

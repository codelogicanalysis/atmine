#include "chain_graph.h"
#include "narrator_abstraction.h"

QString ChainNarratorNode::CanonicalName()
{
	return ((Narrator*)this)->getString();
}

ChainNarratorNode & ChainNarratorNodePtr::operator*()
{
	ChainPrim & curr=getChainPrim();
	assert (curr.isNarrator());
	return (ChainNarratorNode &)curr;
}
ChainNarratorNodePtr & ChainNarratorNodePtr::operator++()
{
	while (getChainPrimPtr()->isNarrator() && !isLast())
		++(*this);
	return *this;//check if this calls operator *() or not
}

ChainNarratorNodePtr & ChainNarratorNodePtr::operator--()
{
	while (getChainPrimPtr()->isNarrator() && !isLast())
		--(*this);
	return *this;//check if this calls operator *() or not
}
bool ChainNarratorNodePtr::isFirst()
{
	return ((Narrator*)getChainPrimPtr())->getRank().first;
}

bool ChainNarratorNodePtr::isLast()
{
	return ((Narrator*)getChainPrimPtr())->getRank().last;
}
int ChainNarratorNodePtr::getIndex()
{
	return ((Narrator*)getChainPrimPtr())->getRank().index;
}

ChainNarratorNodePtr ChainNarratorNode::firstNarrator()
{
	return NULLChainNarratorNodePtr();
}
ChainNarratorNodePtr ChainNarratorNode::nextNarrator(ChainNarratorNodePtr current)
{
	return NULLChainNarratorNodePtr();
}

NarratorNode * ChainNarratorNode::firstChild()
{
	return  &(nextInChain().operator *());
}
NarratorNode * ChainNarratorNode::firstParent()
{
	return  &(prevInChain().operator *());
}
NodeAddress ChainNarratorNode::prevInChain(ChainNarratorNodePtr node)
{
	assert (&(*node)==this);
	ChainNarratorNodePtr prev=--node;
	if (!prev.isNULL())
		return NodeAddress(prev->getCorrespondingNarratorNode(), prev);
	else
		return NodeAddress(NULL,NULLChainNarratorNodePtr());
}
NodeAddress ChainNarratorNode::nextInChain(ChainNarratorNodePtr node)
{
	assert (&(*node)==this);
	ChainNarratorNodePtr next=--node;
	if (!next.isNULL())
		return NodeAddress(next->getCorrespondingNarratorNode (), next);
	else
		return NodeAddress(NULL,NULLChainNarratorNodePtr());
}

NarratorNode *  ChainNarratorNode::getCorrespondingNarratorNode()
{
	if ( narrNode==NULL)
		return this;
	else
		return narrNode;
}

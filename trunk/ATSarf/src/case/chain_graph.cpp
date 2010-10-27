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

ChainNarratorNodePtr ChainNarratorNodePtr::firstNarrator()
{
	return NULLChainNarratorNodePtr();
}
ChainNarratorNodePtr ChainNarratorNodePtr::nextNarrator(ChainNarratorNodePtr)
{
	return NULLChainNarratorNodePtr();
}

NarratorNode * ChainNarratorNodePtr::firstChild()
{
	return  &nextInChain();
}
NarratorNode * ChainNarratorNodePtr::firstParent()
{
	return  &prevInChain();
}
NodeAddress ChainNarratorNodePtr::prevInChain(ChainNarratorNodePtr node)
{
	assert (node==*this);
	ChainNarratorNodePtr prev=--node;
	if (!prev.isNULL())
		return NodeAddress(prev->getCorrespondingNarratorNode(), prev);
	else
		return NodeAddress(NULL,NULLChainNarratorNodePtr());
}
NodeAddress ChainNarratorNodePtr::nextInChain(ChainNarratorNodePtr node)
{
	assert (node==*this);
	ChainNarratorNodePtr next=--node;
	if (!next.isNULL())
		return NodeAddress(next->getCorrespondingNarratorNode (), next);
	else
		return NodeAddress(NULL,NULLChainNarratorNodePtr());
}
ChainNarratorNodePtr::ChainNarratorNodePtr(Chain *ch, Narrator * n):QList<ChainPrim *>::iterator(ch->m_chain.begin()+n->getRank().index)
{

}
GraphNarratorNode::GraphNarratorNode(ChainNarratorNodePtr nar1,ChainNarratorNodePtr nar2)
{
	equalnarrators.append(nar1);
	equalnarrators.append(nar2);
}
GraphNarratorNode::GraphNarratorNode(Chain * chain1, Narrator * nar1,Chain * chain2, Narrator * nar2)
{
	ChainNarratorNodePtr c1(chain1,nar1);
	ChainNarratorNodePtr c2(chain2,nar2);
	equalnarrators.append(c1);
	equalnarrators.append(c2);
}
void GraphNarratorNode::addNarrator(Chain * chain, Narrator * nar)
{
	ChainNarratorNodePtr c(chain,nar);
	equalnarrators.append(c);
}
void GraphNarratorNode::addNarrator(ChainNarratorNodePtr nar)
{
	equalnarrators.append(nar);
}

#include "chain_graph.h"
#include "narrator_abstraction.h"

NULLChainNarratorNodeIterator nullChainNarratorNodeIterator;
NULLNarratorNodeIfc nullNarratorNodeIfc;
ChainsContainer chains;

NarratorNodeIfc & NULLNarratorNodeIfc::firstChild()
{
	return nullNarratorNodeIfc;
}
NarratorNodeIfc & NULLNarratorNodeIfc::nextChild(NarratorNodeIfc &)
{
	return nullNarratorNodeIfc;
}
NarratorNodeIfc & NULLNarratorNodeIfc::firstParent()
{
	return nullNarratorNodeIfc;
}
NarratorNodeIfc & NULLNarratorNodeIfc::nextParent(NarratorNodeIfc & )
{
	return nullNarratorNodeIfc;
}
ChainNarratorNodeIterator & NULLNarratorNodeIfc::firstNarrator()
{
	return nullChainNarratorNodeIterator;
}
ChainNarratorNodeIterator & NULLNarratorNodeIfc::nextNarrator(ChainNarratorNodeIterator & )
{
	return nullChainNarratorNodeIterator;
}
NodeAddress NULLNarratorNodeIfc::prevInChain(ChainNarratorNodeIterator &)
{
	return nullNodeAddress;
}
NodeAddress NULLNarratorNodeIfc::nextInChain(ChainNarratorNodeIterator &)
{
	return nullNodeAddress;
}
QString ChainNarratorNode::CanonicalName()
{
	return ((Narrator*)this)->getString();
}
ChainNarratorNode & ChainNarratorNodeIterator::operator*()
{
	ChainPrim & curr=getChainPrim();
	assert (curr.isNarrator());
	return (ChainNarratorNode &)curr;
}
ChainNarratorNodeIterator & ChainNarratorNodeIterator::operator++()
{
	while (getChainPrimPtr()->isNarrator() && !isLast())
		++(*this);
	return *this;//check if this calls operator *() or not
}

ChainNarratorNodeIterator & ChainNarratorNodeIterator::operator--()
{
	while (getChainPrimPtr()->isNarrator() && !isLast())
		--(*this);
	return *this;//check if this calls operator *() or not
}
bool ChainNarratorNodeIterator::isFirst()
{
	return ((Narrator*)getChainPrimPtr())->getRank().first;
}

bool ChainNarratorNodeIterator::isLast()
{
	return ((Narrator*)getChainPrimPtr())->getRank().last;
}
int ChainNarratorNodeIterator::getIndex()
{
	return ((Narrator*)getChainPrimPtr())->getRank().index;
}

ChainNarratorNodeIterator & ChainNarratorNodeIterator::firstNarrator()
{
	return nullChainNarratorNodeIterator;
}
ChainNarratorNodeIterator & ChainNarratorNodeIterator::nextNarrator(ChainNarratorNodeIterator &)
{
	return nullChainNarratorNodeIterator;
}

NarratorNodeIfc & ChainNarratorNodeIterator::firstChild()
{
	return  nextInChain();
}
NarratorNodeIfc & ChainNarratorNodeIterator::firstParent()
{
	return  prevInChain();
}
NodeAddress ChainNarratorNodeIterator::prevInChain(ChainNarratorNodeIterator & node)
{
	assert (node==*this);
	ChainNarratorNodeIterator prev=--node;
	if (!prev.isNULL())
		return NodeAddress(prev->getCorrespondingNarratorNode(), prev);
	else
		return nullNodeAddress;
}
NodeAddress ChainNarratorNodeIterator::nextInChain(ChainNarratorNodeIterator & node)
{
	assert (node==*this);
	ChainNarratorNodeIterator next=--node;
	if (!next.isNULL())
		return NodeAddress(next->getCorrespondingNarratorNode (), next);
	else
		return nullNodeAddress;
}
ChainNarratorNodeIterator::ChainNarratorNodeIterator(Chain *ch, Narrator * n)//change to refernce
{
	QList<ChainPrim *>::iterator (ch->m_chain.begin()+n->getRank().index);
}
ChainNarratorNodeIterator::ChainNarratorNodeIterator(QList<ChainPrim *>::iterator itr):QList<ChainPrim *>::iterator (itr)
{
}
ChainNarratorNodeIterator & ChainNarratorNodeIterator::firstNarratorInChain()
{
	if (getChainPrim().isNarrator())
		return *this;
	else
		return this->operator ++();
}
GraphNarratorNode::GraphNarratorNode(ChainNarratorNodeIterator nar1,ChainNarratorNodeIterator nar2)
{
	equalnarrators.append(nar1);
	equalnarrators.append(nar2);
}
GraphNarratorNode::GraphNarratorNode(Chain * chain1, Narrator * nar1,Chain * chain2, Narrator * nar2)
{
	ChainNarratorNodeIterator c1(chain1,nar1);
	ChainNarratorNodeIterator c2(chain2,nar2);
	equalnarrators.append(c1);
	equalnarrators.append(c2);
}
void GraphNarratorNode::addNarrator(Chain * chain, Narrator * nar)
{
	ChainNarratorNodeIterator c(chain,nar);
	equalnarrators.append(c);
}
void GraphNarratorNode::addNarrator(ChainNarratorNodeIterator nar)
{
	equalnarrators.append(nar);
}

void fillRank(Narrator &n, int index, bool last)
{
	n.rank.index=index;
	n.rank.last=last;
	n.rank.first=(index==0);
}
void fillRanks()
{
	for (int i=0;i<chains.count();i++)
	{
		int size=chains.at(i)->m_chain.count(),index=0;
		Narrator* n;
		for (int j=0;j<size;j++)
		{
			if (chains.at(i)->m_chain[j]->isNarrator())
			{
				//qDebug()<<chains.at(i)->m_chain[j]->getString()<<"\n";
				n=(Narrator *)(chains.at(i)->m_chain[j]);
				fillRank(*n,index,j==size-1);
				n->getRank().printRank();
				index++;
			}
		}
		fillRank(*n,index-1,true);
	}
}

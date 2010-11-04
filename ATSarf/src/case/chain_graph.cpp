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

ChainNarratorNodeIterator & ChainNarratorNodeIterator::operator+(int n)
{
	for (int i=0;i<n;i++)
		this->operator ++();
	return *this;
}
ChainNarratorNodeIterator & ChainNarratorNodeIterator::operator-(int n)
{
	for (int i=0;i<n;i++)
		this->operator --();
	return *this;
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
int ChainNarratorNodeIterator::getChainNum()
{
	return ((Narrator*)getChainPrimPtr())->getRank().chain_num;
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
GraphNarratorNode::GraphNarratorNode(ChainNarratorNodeIterator & nar1,ChainNarratorNodeIterator & nar2)
{
	equalnarrators.append(nar1);
	equalnarrators.append(nar2);
	nar1->setCorrespondingNarratorNode(this);
	nar2->setCorrespondingNarratorNode(this);
}
GraphNarratorNode::GraphNarratorNode(Chain * chain1, Narrator * nar1,Chain * chain2, Narrator * nar2)
{
	ChainNarratorNodeIterator c1(chain1,nar1);
	ChainNarratorNodeIterator c2(chain2,nar2);
	equalnarrators.append(c1);
	equalnarrators.append(c2);
	c1->setCorrespondingNarratorNode(this);
	c2->setCorrespondingNarratorNode(this);
}
void GraphNarratorNode::addNarrator(Chain * chain, Narrator * nar)
{
	ChainNarratorNodeIterator c(chain,nar);
	equalnarrators.append(c);
	c->setCorrespondingNarratorNode(this);
}
void GraphNarratorNode::addNarrator(ChainNarratorNodeIterator & nar)
{
	equalnarrators.append(nar);
	nar->setCorrespondingNarratorNode(this);
}

void fillRank(Narrator &n, int index, bool last, int chain_num)
{
	n.rank.index=index;
	n.rank.last=last;
	n.rank.first=(index==0);
	n.rank.chain_num=chain_num;
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
				fillRank(*n,index,j==size-1,i);
				n->getRank().printRank();
				index++;
			}
		}
		fillRank(*n,index-1,true,i);
	}
}

const int radius=3;
const double threshold=0.7;

class TmpChainInfo
{
public:
	ChainNarratorNodeIterator curr_itr;
	//int traversal_num;
	bool checkStepEqualChains(ChainNarratorNodeIterator & it)
	{
		for (int j=0;j<radius && j<chains.at(curr_itr.getChainNum())->m_chain.size();j++)
		{
			if (equal((curr_itr+j).getNarrator(),it.getNarrator())>=threshold)
			{
				curr_itr+=j;

				if (it.getCorrespondingNarratorNode().isNull() && curr_itr.getCorrespondingNarratorNode().isNull())
					new GraphNarratorNode(curr_itr,it); //dont delete
				else if (!it.getCorrespondingNarratorNode().isNull())
					((GraphNarratorNode &)(it.getCorrespondingNarratorNode())).addNarrator(curr_itr);
				else if (!curr_itr.getCorrespondingNarratorNode().isNull())
					((GraphNarratorNode &)(curr_itr.getCorrespondingNarratorNode())).addNarrator(it);
				else if (&curr_itr.getCorrespondingNarratorNode()!=&it.getCorrespondingNarratorNode())
				{
					GraphNarratorNode & g_node=(GraphNarratorNode &)(curr_itr.getCorrespondingNarratorNode());
					GraphNarratorNode * dlt_g_node= &(GraphNarratorNode &)(it.getCorrespondingNarratorNode());
					for (ChainNarratorNodeIterator & c_node=it.firstNarrator();!c_node.isNULL();c_node=it.nextNarrator(c_node))
						g_node.addNarrator(c_node);
					delete dlt_g_node;
				}
				//traversal_num++;
				return true;
			}
		}
		if (curr_itr+1!=chains.at(curr_itr.getChainNum())->m_chain.end())
			++curr_itr;
		//traversal_num++;
		return false;
	}
	bool finished()
	{
		return (curr_itr==chains.at(curr_itr.getChainNum())->m_chain.end());
	}
};

int test_NarratorEquality(QString)
{
	//find the max_size of chains
	int max_size=0;
	for (int i=0;i<chains.size();i++)
	{
		int curr_size=chains.at(i)->m_chain.size();
		if (curr_size>max_size)
			max_size=curr_size;
	}
	//fill info with begin()
	QVector<QVector<TmpChainInfo> > info;
	QVector<TmpChainInfo> traversal_leader_pos;
	QVector<int> chains_left;
	QVector<TmpChainInfo> temp;
	for (int i=0;i<chains.size();i++)
	{
		TmpChainInfo t;
		t.curr_itr=chains.at(i)->m_chain.begin();
		temp.push_back(t);
		chains_left.push_back(i);
	}
	traversal_leader_pos=temp;
	info.push_back(temp);
	//since temp has all at first position push to entry of 0, the rest must be moved one pos then pushed to all rest
	for (int j=0;j<chains.size();j++)
		++temp[j].curr_itr;
	for (int i=1;i<chains.size();i++)
		info.push_back(temp);

	while (chains_left.count()>1)
	{
		for (int i=0;i<chains_left.count();i++)
		{
			int c1=chains_left[i];
			ChainNarratorNodeIterator & it=traversal_leader_pos[c1].curr_itr;
			for (int j=0;j<chains_left.count();j++)
			{
				int c2=chains_left[j];
				if (c1!=c2)
				{
					if (!info[c1][c2].finished())
						info[c1][c2].checkStepEqualChains(it);
				}
			}
			++it;
			if (traversal_leader_pos[c1].finished())
				chains_left.remove(c1);
		}
	}


	/*
	for (int c1=0;c1<chains.count();c1++)
	{
		ChainNarratorNodeIterator it1(chains.at(c1)->m_chain.begin());
		for (int c2=0;c2<chains.count();c2++)
		{
			ChainNarratorNodeIterator it2(chains.at(c2)->m_chain.begin());//if it is not the first check, then already checked
			if (c1!=0)
				++it2;
			for (int j=(c1==0?0:1);j<radius && j<chains.at(c2)->m_chain.size();j++)
			{
				assert(it2.getIndex()==j);
				if (equal(it1.getNarrator(),it2.getNarrator())>=threshold)
				{
					new GraphNarratorNode(it1,it2); //dont delete
					break;
				}
				++it2;
			}
		}
	}
	for (int n=1;n<max_size;n++)
	{
		for (int c1=0;c1<chains.count();c1++)
		{
			ChainNarratorNodeIterator it1(chains.at(c1)->m_chain.begin()+n);
			for (int c2=n;c2<chains.count();c2++)
			{
				ChainNarratorNodeIterator it2(chains.at(c2)->m_chain.begin());//if it is not the first check, then already checked
				if (c1!=0)
					++it2;
				for (int j=(c1==0?0:1);j<radius;j++)
				{
					assert(it2.getIndex()==j);
					if (equal(it1.getNarrator(),it2.getNarrator())>=threshold)
					{
						new GraphNarratorNode(it1,it2); //dont delete
						break;
					}
					++it2;
				}
			}
		}
	}*/
	return 0;
}

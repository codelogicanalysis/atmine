#ifndef GRAPH_STRUCTURE_H
#define GRAPH_STRUCTURE_H

#include "Triplet.h"
#include <assert.h>
#include <QList>
#include <QMap>
#include <QPair>
#include <QFile>
#include "logger.h"
#include <QDebug>
#include <QQueue>
#include "reference.h"
#include "narrator_abstraction.h"

#include "hadith.h"

#define SMALLEST_CANONICAL

class NarratorNodeIfc;
class ChainNarratorNode;
class ChainNarratorNode;
class GraphNarratorNode;
class NULLGraphNarratorNode;
class NULLNarratorNodeIfc;
class NULLChainNarratorNode;

typedef QList<Chain *> ChainsContainer;
typedef QPair<NarratorNodeIfc &, ChainNarratorNode &> NodeAddress;

#define nullNodeAddress NodeAddress((NarratorNodeIfc &)nullNarratorNodeIfc,(ChainNarratorNode &)nullChainNarratorNode)
extern NULLNarratorNodeIfc nullNarratorNodeIfc;
extern NULLGraphNarratorNode nullGraphNarratorNode;
extern NULLChainNarratorNode nullChainNarratorNode;

class RankCorrectorNodeVisitor;
class LoopBreakingVisitor;
double equal(const Narrator & n1,const Narrator & n2);


class NarratorNodeIfc //abstract interface
{
protected:
	virtual void setRank(int rank)=0;
	virtual int getSavedRank()=0;
	virtual int getAutomaticRank()=0;
	friend class RankCorrectorNodeVisitor;
public:
	virtual NarratorNodeIfc & getCorrespondingNarratorNode()=0; //if used on a graphNode returns null

	virtual int size()=0;
	virtual NarratorNodeIfc & getChild(int index)=0;
	virtual NarratorNodeIfc & getParent(int index)=0;
	virtual ChainNarratorNode & operator [](int index)=0;

	virtual NodeAddress prevInChain(ChainNarratorNode &)=0;
	virtual NodeAddress nextInChain(ChainNarratorNode &)=0;

	virtual QString CanonicalName()=0;
	virtual bool isNull()=0;
	virtual bool isGraphNode()=0;

	virtual int getRank()
	{
		int savedRank=getSavedRank();
		return (savedRank>=0?savedRank:getAutomaticRank());
	}
	virtual QString rank2String()=0;

	virtual ChainNarratorNode & getChainNodeInChain(int chain_num)=0;
	virtual QString toString() =0;
};

class NULLNarratorNodeIfc: public NarratorNodeIfc
{
	NarratorNodeIfc & getCorrespondingNarratorNode(){throw 1;}

	int size(){throw 1;}
	NarratorNodeIfc & getChild(int ){throw 1;}
	NarratorNodeIfc & getParent(int){throw 1;}
	ChainNarratorNode & operator [](int){throw 1;}

	ChainNarratorNode & firstChainNode(){throw 1;}
	ChainNarratorNode & nextChainNode(ChainNarratorNode & ){throw 1;}

	NodeAddress prevInChain(ChainNarratorNode &){throw 1;}
	NodeAddress nextInChain(ChainNarratorNode &){throw 1;}

	QString CanonicalName(){throw 1;}

	virtual bool isNull(){throw 1;}
	void setRank(int ){throw 1;}
	int getAutomaticRank(){throw 1;}
	int getRank() {throw 1;}
	int getSavedRank(){throw 1;}
	virtual QString rank2String(){throw 1;}
	virtual ChainNarratorNode & getChainNodeInChain(int){throw 1;}
	virtual bool isGraphNode() {return false;}
	virtual QString toString()   {return "NULLNarratorNodeIfc";}
};

class ChainContext
{
private:
	bool valid:1; //to indicate if info in the variable is valid
	int index:8;
	int chain_num:23;
public:
	ChainContext()
	{
		valid=false;
	}
	ChainContext(int index, int chain_num)
	{
		set(index,chain_num);
	}
	void set(int index, int chain_num)
	{
		this->index=index;
		this->chain_num=chain_num;
		valid=true;
	}
	int getIndex()
	{
		assert(valid);
		return index;
	}
	int getChainNum()
	{
		assert(valid);
		return chain_num;
	}
	void printChainContext() //for debugging purposes only
	{
		out<<chain_num<<"["<<index<<"]\n";
		//qDebug()<<chain_num<<"["<<index<<"]\n";;
	}
};

class ChainNarratorNode:public NarratorNodeIfc
{
private:
	ChainNarratorNode * previous, * next;
	Narrator * narrator;

	GraphNarratorNode * graphNode;
	int savedRank;
	friend class NarratorGraph;

	ChainContext chainContext;
	friend void buildChainNodes(ChainsContainer & chains);
protected:
	ChainNarratorNode() {} //to be used by NULLChainNarratorNode
	virtual int getSavedRank(){	return savedRank;}
	virtual int getAutomaticRank(){return getIndex();}
	virtual void setRank(int rank) {savedRank=rank;}
	friend class GraphNarratorNode;
public:
	ChainNarratorNode(Narrator * n,int index, int chain_num)
	{
		previous=next=NULL;
		graphNode=NULL;
		savedRank=-1;
		narrator=n;
		graphNode=NULL;
		chainContext.set(index, chain_num);
	}
	virtual NarratorNodeIfc & getCorrespondingNarratorNode();
	virtual void  setCorrespondingNarratorNode(GraphNarratorNode * graphNode) {this->graphNode=graphNode;}
	virtual QString toString(){	return "("+CanonicalName()+")";}
	virtual Narrator & getNarrator() {return (Narrator &)*narrator;}
	virtual ChainNarratorNode & operator+(int n) const
	{
		if (n>=0)
		{
			ChainNarratorNode * c=(ChainNarratorNode *)this;
			for (int i=0;i<n && !c->isNull();i++)
				c=&(c->nextInChain());
			return *c;
		}
		else
		{
			ChainNarratorNode * c=(ChainNarratorNode *)this;
			for (int i=0;i>n && !c->isNull();i--)
				c=&(c->prevInChain());
			return *c;
		}
	}
	virtual ChainNarratorNode & operator-(int n) {return this->operator +(-n);}
	virtual ChainNarratorNode & prevInChain();
	virtual ChainNarratorNode & nextInChain();
	virtual NarratorNodeIfc & nextChild(NarratorNodeIfc & ) {return nullNarratorNodeIfc;}
	virtual int size() {return (isLast()?0:1);}
	virtual NarratorNodeIfc & getChild(int index)
	{
		assert(index==0);
		return nextInChain().getCorrespondingNarratorNode();
	}
	virtual NarratorNodeIfc & getParent(int index)
	{
		assert(index==0);
		return prevInChain().getCorrespondingNarratorNode();
	}
	virtual ChainNarratorNode & operator [](int index)
	{
		assert(index==0);
		return *this;
	}
	virtual NodeAddress prevInChain(ChainNarratorNode & node)
	{
		assert (&node==this);
		ChainNarratorNode & prev=node.prevInChain();
		if (!prev.isNull())
			return NodeAddress(prev.getCorrespondingNarratorNode(), prev);
		else
			return nullNodeAddress;
	}
	virtual NodeAddress nextInChain(ChainNarratorNode & node)
	{
		assert (&node==this);
		ChainNarratorNode & next=node.nextInChain();
		if (!next.isNull())
			return NodeAddress(next.getCorrespondingNarratorNode(), next);
		else
			return nullNodeAddress;
	}
	virtual QString CanonicalName(){return narrator->getString();}
	virtual bool isFirst() {return previous==NULL;}
	virtual bool isLast() {return next==NULL;}
	virtual int getIndex() {return chainContext.getIndex();}
	virtual QString rank2String()
	{
	#ifdef SHOW_VERBOSE_RANKS
		return QString("[%1](%2)").arg(getAutomaticRank()).arg(getSavedRank());
	#else
		return QString("(%1)").arg(getSavedRank());
	#endif

	}
	virtual int getChainNum() {return chainContext.getChainNum();}
	virtual bool isNull() {	return false;}
	virtual bool isGraphNode() { return false;}
	virtual ChainNarratorNode & getChainNodeInChain(int chain_num)
	{
		if (getChainNum()==chain_num)
			return *this;
		else
			return (ChainNarratorNode &)nullChainNarratorNode;
	}
};

class NULLChainNarratorNode: public ChainNarratorNode
{
protected:
	int getSavedRank(){throw 1;}
	int getAutomaticRank(){throw 1;}
	void setRank(int ){throw 1;}
public:
	NULLChainNarratorNode() {}
	bool isNull() {	return true;}
	NarratorNodeIfc & getCorrespondingNarratorNode() {throw 1;}
	Narrator & getNarrator(){throw 1;}
	ChainNarratorNode & operator+(int){throw 1;}
	ChainNarratorNode & operator-(int){throw 1;}
	int size() {return 0;}
	NarratorNodeIfc & getChild(int){throw 1;}
	NarratorNodeIfc & getParent(int){throw 1;}
	ChainNarratorNode & operator [](int){throw 1;}
	NodeAddress prevInChain(ChainNarratorNode & ){throw 1;}
	NodeAddress nextInChain(ChainNarratorNode & ){throw 1;}
	ChainNarratorNode & prevInChain(){throw 1;}
	ChainNarratorNode & nextInChain(){throw 1;}
	bool isFirst(){throw 1;}
	bool isLast(){throw 1;}
	int getIndex(){throw 1;}
	QString rank2String(){throw 1;}
	QString CanonicalName(){throw 1;}
	int getChainNum(){throw 1;}
	ChainNarratorNode & getChainNodeInChain(int ){throw 1;}
	QString toString(){	return "NULLChainNarratorNodeIterator";}
};

class GraphNarratorNode: public NarratorNodeIfc
{
protected:
	QList<ChainNarratorNode *>  equalChainNodes;
	int savedRank;
	virtual void setRank(int rank){savedRank=rank;}
	virtual int getAutomaticRank()
	{
		int largest_rank=equalChainNodes[0]->getAutomaticRank();
		for (int i=1;i<equalChainNodes.size();i++)
		{
			int rank=equalChainNodes[i]->getAutomaticRank();
			if (largest_rank<rank)
				largest_rank=rank;
		}
		return largest_rank;
	}
	virtual int getSavedRank(){return savedRank;}
	friend class LoopBreakingVisitor;

	GraphNarratorNode(){} //to be used by NULLGraphNarratorNode
public:
	//GraphNarratorNode(){savedRank=-1;}
	GraphNarratorNode(ChainNarratorNode & nar1,ChainNarratorNode & nar2)
	{
		assert(&nar1!=&nar2); //make sure these are not just the same node
		equalChainNodes.append(&nar1);
		nar1.setCorrespondingNarratorNode(this);
		equalChainNodes.append(&nar2);
		nar2.setCorrespondingNarratorNode(this);
		savedRank=-1;
	}
	virtual void addNarrator(ChainNarratorNode & nar) //we dont check for duplicates here
	{
		equalChainNodes.append(&nar);
		nar.setCorrespondingNarratorNode(this);
	}
	virtual NarratorNodeIfc & getCorrespondingNarratorNode() {return nullNarratorNodeIfc;}
	virtual int size(){ return equalChainNodes.size(); }
	virtual NarratorNodeIfc & getChild(int index)
	{
		assert(index>=0 && index<size());//check redundant will be done inside [] for QList
		ChainNarratorNode & c =equalChainNodes[index]->nextInChain();
		return (c.isNull()
					?nullChainNarratorNode
					:c.getCorrespondingNarratorNode());
	}
	virtual NarratorNodeIfc & getParent(int index)
	{
		assert(index>=0 && index<size());//check redundant will be done inside [] for QList
		ChainNarratorNode & c =equalChainNodes[index]->prevInChain();
		return (c.isNull()
					?nullChainNarratorNode
					:c.getCorrespondingNarratorNode());
	}
	virtual ChainNarratorNode & operator [](int index)
	{
		assert(index>=0 && index<size());//check redundant will be done inside [] for QList
		return *equalChainNodes[index];
	}
	virtual NodeAddress prevInChain(ChainNarratorNode & node) //does not check if node belongs to graph node, since will work in all cases
	{
		assert(!node.isNull());
		ChainNarratorNode & prev=node.prevInChain();
		return (prev.isNull()
					? nullNodeAddress
					:NodeAddress(prev.getCorrespondingNarratorNode(), prev));
	}
	virtual NodeAddress nextInChain(ChainNarratorNode & node)//does not check if node belongs to graph node, since will work in all cases
	{
		assert(!node.isNull());
		ChainNarratorNode & next=node.nextInChain();
		return (next.isNull()
					? nullNodeAddress
					:NodeAddress(next.getCorrespondingNarratorNode(), next));
	}
	virtual QString rank2String()
	{
		QString ranks;
	#ifdef SHOW_VERBOSE_RANKS
		ranks="[";
		for (int i=0;i<equalnarrators.size();i++)
			ranks+=QString("%1,").arg(equalnarrators[i].getAutomaticRank());
		ranks+="]";
	#endif
		ranks+=QString("(%1)").arg(getSavedRank());
		return ranks;
	}
	virtual QString CanonicalName()
	{
	#ifdef REFINEMENTS
		if (rasoul_words.contains(equalChainNodes[0]->CanonicalName()))
			return alrasoul;
	#endif
	#ifdef SMALLEST_CANONICAL
		//qDebug()<<"---";
		int smallestsize=equalChainNodes[0]->CanonicalName().size(), index=0;
		//qDebug()<<"("<<equalnarrators[0].CanonicalName();
		for (int i=1;i<equalChainNodes.size();i++)
		{
			int size=equalChainNodes[i]->CanonicalName().size();
			//qDebug()<<equalnarrators[i].CanonicalName();
			if (smallestsize>size)
			{
				smallestsize=size;
				index=i;
			}
		}
	#else
		//qDebug()<<"---";
		int largestsize=equalnarrators[0].CanonicalName().size(), index=0;
		//qDebug()<<"("<<equalnarrators[0].CanonicalName();
		for (int i=1;i<equalnarrators.size();i++)
		{
			int size=equalnarrators[i].CanonicalName().size();
			//qDebug()<<equalnarrators[i].CanonicalName();
			if (largestsize<size)
			{
				largestsize=size;
				index=i;
			}
		}
	#endif
		if (index>=0)
		{
			//qDebug()<<")=>{"<<equalnarrators[index].CanonicalName()<<"}";
			return equalChainNodes[index]->CanonicalName();
		}
		else
			return "";
	}
	virtual bool isGraphNode() {return true;}
	virtual ChainNarratorNode & getChainNodeInChain(int chain_num)
	{
		for (int i=0;i<equalChainNodes.size();i++)
			if (equalChainNodes[i]->getChainNum()==chain_num)
				return *equalChainNodes[i];
		return nullChainNarratorNode;
	}
	virtual QString toString()
	{
		QString s=QString("[");
		for (int i=0;i<equalChainNodes.size();i++)
			s+=equalChainNodes[i]->toString();
		s+="]";
		return s;
	}
	virtual bool isNull(){return false;}
};

class NULLGraphNarratorNode: public GraphNarratorNode
{
protected:
	int getSavedRank(){throw 1;}
	int getAutomaticRank(){throw 1;}
	void setRank(int ){throw 1;}
public:
	NULLGraphNarratorNode(){}
	bool isNull() {return true;}
	void addNarrator(ChainNarratorNode &) {throw -1;}
	NarratorNodeIfc & getCorrespondingNarratorNode() {throw 1;}
	int size() {return 0;}
	NarratorNodeIfc & getChild(int){throw 1;}
	NarratorNodeIfc & getParent(int){throw 1;}
	ChainNarratorNode & operator [](int){throw 1;}
	NodeAddress prevInChain(ChainNarratorNode & ){throw 1;}
	NodeAddress nextInChain(ChainNarratorNode & ){throw 1;}
	QString rank2String(){throw 1;}
	QString CanonicalName(){throw 1;}
	ChainNarratorNode & getChainNodeInChain(int ){throw 1;}
	QString toString(){	return "NULLChainNarratorNodeIterator";}
};



#endif // GRAPH_STRUCTURE_H

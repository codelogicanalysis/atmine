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
#include <QDataStream>
#include "reference.h"
#include "narrator_abstraction.h"
#include "hadith_utilities.h"

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
typedef QList<int> BiographyIndicies;

extern NodeAddress nullNodeAddress;
extern NULLNarratorNodeIfc nullNarratorNodeIfc;
extern NULLGraphNarratorNode nullGraphNarratorNode;
extern NULLChainNarratorNode nullChainNarratorNode;

class RankCorrectorNodeVisitor;
class NarratorGraph;
class GraphVisitorController;
class ColorIndices;


class NarratorNodeIfc { //abstract interface
private:
	BiographyIndicies indicies;
	unsigned int color;
	friend class LoopBreakingVisitor;
protected:
	virtual void setRank(int rank)=0;
	virtual int getSavedRank() const=0;
	virtual int getAutomaticRank() const=0;
	friend class RankCorrectorNodeVisitor;

	virtual void setVisited(unsigned int bit) {	color |= 1 << bit; }
	virtual void resetVisited(unsigned int bit) {color &= (~(1 << bit)); }
	virtual void resetColor() {	color = 0; }
	NarratorNodeIfc(){resetColor();}
	friend class NarratorGraph;
	friend class ColorIndices;
	friend class GraphVisitorController;

	virtual void serializeHelper(QDataStream &chainOut, NarratorGraph & graph) const=0;
	virtual void deserializeHelper(QDataStream &chainIn,NarratorGraph & graph) =0;
public:
	virtual NarratorNodeIfc & getCorrespondingNarratorNode()=0; //if used on a graphNode returns null

	virtual int size() const=0;
	virtual NarratorNodeIfc & getChild(int index)=0;
	virtual NarratorNodeIfc & getParent(int index)=0;
	virtual ChainNarratorNode & operator [](int index)=0;

	virtual NodeAddress prevInChain(ChainNarratorNode &)=0;
	virtual NodeAddress nextInChain(ChainNarratorNode &) =0;

	virtual QString CanonicalName() const=0;
	virtual bool isNull() const=0;
	virtual bool isGraphNode() const=0;

	virtual int getRank() const {
		int savedRank=getSavedRank();
		return (savedRank>=0?savedRank:getAutomaticRank());
	}
	virtual QString rank2String() const=0;

	virtual ChainNarratorNode & getChainNodeInChain(int chain_num)=0;
	virtual QString toString() const =0;
	virtual bool isVisited(unsigned int bit) const {return (color & (1 << bit)) != 0; }

	void addBiographyIndex(int i) {
		NarratorNodeIfc & n=getCorrespondingNarratorNode();
		if (!n.indicies.contains(i)){
			n.indicies.append(i);
			/*qDebug()<<(long)this<<"-"<<(long)&n
					<<"<"<<n.CanonicalName()<<">:"<<i;*/
		}
	}
	bool hasBiographyIndex(int i) {
		NarratorNodeIfc & n=getCorrespondingNarratorNode();
		/*qDebug()<<"---"<<(long)&n<<"<"<<CanonicalName()<<">---";
		for (int j=0;j<n.indicies.size();j++)
			qDebug()<<n.indicies[j];*/
		return (n.indicies.contains(i));
	}

	void serialize(QDataStream &chainOut,NarratorGraph & graph) const {
		chainOut<<isNull();
		chainOut<<isGraphNode();
		if (!isNull()) {
			chainOut<<indicies;
			serializeHelper(chainOut,graph);
		}
	}
	static NarratorNodeIfc * deserialize(QDataStream &chainIn,NarratorGraph & graph) ;
};

class NULLNarratorNodeIfc: public NarratorNodeIfc {
	NarratorNodeIfc & getCorrespondingNarratorNode() {assert(false);}

	int size() const{assert(false);}
	NarratorNodeIfc & getChild(int ) {assert(false);}
	NarratorNodeIfc & getParent(int) {assert(false);}
	ChainNarratorNode & operator [](int){assert(false);}

	ChainNarratorNode & firstChainNode() {assert(false);}
	ChainNarratorNode & nextChainNode(ChainNarratorNode & ) {assert(false);}

	NodeAddress prevInChain(ChainNarratorNode &) {assert(false);}
	NodeAddress nextInChain(ChainNarratorNode &) {assert(false);}

	QString CanonicalName() const{assert(false);}

	virtual bool isNull() const {return true;}
	void setRank(int ){assert(false);}
	int getAutomaticRank() const {assert(false);}
	int getRank() const {assert(false);}
	int getSavedRank() const{assert(false);}
	virtual QString rank2String() const {assert(false);}
	virtual ChainNarratorNode & getChainNodeInChain(int) {assert(false);}
	virtual bool isGraphNode() const  {return false;}
	virtual QString toString() const   {return "NULLNarratorNodeIfc";}
	void setVisited(unsigned int ) { assert(false);}
	void resetVisited(unsigned int ) { assert(false); }
	bool isVisited(unsigned int ) const{ assert(false); }
	void resetColor() { assert(false); }

	virtual void serializeHelper(QDataStream &,NarratorGraph & ) const {}
	virtual void deserializeHelper(QDataStream &,NarratorGraph & ) {}
};

class ChainContext
{
private:
	bool valid:1; //to indicate if info in the variable is valid
	int index:8;
	int chain_num:23;
public:
	ChainContext()	{valid=false;}
	ChainContext(int index, int chain_num){	set(index,chain_num);}
	void set(int index, int chain_num) {
		//assert(index<20);
		this->index=index;
		this->chain_num=chain_num;
		valid=true;
	}
	int getIndex() const {
		assert(valid);
		return index;
	}
	int getChainNum() const {
		assert(valid);
		return chain_num;
	}
	void printChainContext() { //for debugging purposes only
		out<<chain_num<<"["<<index<<"]\n";
		//qDebug()<<chain_num<<"["<<index<<"]\n";;
	}
};

class ChainNarratorNode:public NarratorNodeIfc {
private:
	ChainNarratorNode * previous, * next;
	Narrator * narrator;

	GraphNarratorNode * graphNode;
	int savedRank;
	friend class NarratorGraph;

	ChainContext chainContext;
	friend void buildChainNodes(ChainsContainer & chains);
protected:
	ChainNarratorNode():NarratorNodeIfc() {} //to be used by NULLChainNarratorNode
	friend class NarratorNodeIfc;

	virtual int getSavedRank() const{	return savedRank;}
	virtual int getAutomaticRank() const {
	#if 1
		return (savedRank>=0?savedRank:getIndex());
	#else
		int r=getIndex();
		if (savedRank<r)
			return savedRank;
		else
			return r;
	#endif
	}
	virtual void setRank(int rank) {savedRank=rank;}
	virtual void setIndex(int index){chainContext.set(index, chainContext.getChainNum());}
	virtual void setChainNum(int num){chainContext.set(chainContext.getIndex(), num);}
	friend class GraphNarratorNode;

	virtual void serializeHelper(QDataStream &chainOut, NarratorGraph & graph) const;
	virtual void deserializeHelper(QDataStream &chainIn,NarratorGraph & graph) ;
public:
	ChainNarratorNode(Narrator * n,int index, int chain_num):NarratorNodeIfc()
	{
		previous=next=NULL;
		graphNode=NULL;
		savedRank=-1;
		narrator=n;
		graphNode=NULL;
		chainContext.set(index, chain_num);
	}
	virtual NarratorNodeIfc & getCorrespondingNarratorNode() ;
	virtual void  setCorrespondingNarratorNode(GraphNarratorNode * graphNode) {this->graphNode=graphNode;}
	virtual QString toString() const{	return "("+CanonicalName()+")";}
	virtual Narrator & getNarrator() const {return (Narrator &)*narrator;}
	virtual ChainNarratorNode & operator+(int n)
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
	virtual ChainNarratorNode & operator-(int n)  {return this->operator +(-n);}
	virtual ChainNarratorNode & prevInChain() ;
	virtual ChainNarratorNode & nextInChain() ;
	virtual NarratorNodeIfc & nextChild(NarratorNodeIfc & )  {return nullNarratorNodeIfc;}
	virtual int size() const{return 1;}
	virtual NarratorNodeIfc & getChild(int index)
	{
		assert(index==0);
		if (isLast())
			return (NarratorNodeIfc &)nullChainNarratorNode;
		else
			return nextInChain().getCorrespondingNarratorNode();
	}
	virtual NarratorNodeIfc & getParent(int index)
	{
		assert(index==0);
		if (isFirst())
			return (NarratorNodeIfc &)nullChainNarratorNode;
		else
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
	virtual QString CanonicalName() const{return narrator->getString();}
	virtual bool isFirst() const {return previous==NULL;}
	virtual bool isLast() const {return next==NULL;}
	virtual int getIndex() const {return chainContext.getIndex();}
	virtual QString rank2String() const
	{
	#ifdef SHOW_VERBOSE_RANKS
		return QString("[%1](%2)").arg(getAutomaticRank()).arg(getSavedRank());
	#else
		return QString("(%1)").arg(getSavedRank());
	#endif

	}
	virtual int getChainNum() const {return chainContext.getChainNum();}
	virtual bool isNull() const {	return false;}
	virtual bool isGraphNode() const { return false;}
	virtual ChainNarratorNode & getChainNodeInChain(int chain_num) 	{
		if (getChainNum()==chain_num)
			return *this;
		else
			return (ChainNarratorNode &)nullChainNarratorNode;
	}
};

class NULLChainNarratorNode: public ChainNarratorNode
{
protected:
	int getSavedRank() const{assert(false);}
	int getAutomaticRank() const{assert(false);}
	void setRank(int ){assert(false);}
	void setIndex(int ){assert(false);}

	virtual void serializeHelper(QDataStream &, NarratorGraph & ) const {}
	virtual void deserializeHelper(QDataStream &,NarratorGraph & ) {}
public:
	NULLChainNarratorNode() {}
	bool isNull() const {	return true;}
	NarratorNodeIfc & getCorrespondingNarratorNode()  {assert(false);}
	Narrator & getNarrator() const{assert(false);}
	ChainNarratorNode & operator+(int) {assert(false);}
	ChainNarratorNode & operator-(int) {assert(false);}
	int size() const {return 0;}
	NarratorNodeIfc & getChild(int) {assert(false);}
	NarratorNodeIfc & getParent(int) {assert(false);}
	ChainNarratorNode & operator [](int) {assert(false);}
	NodeAddress prevInChain(ChainNarratorNode & ) {assert(false);}
	NodeAddress nextInChain(ChainNarratorNode & ) {assert(false);}
	ChainNarratorNode & prevInChain() {assert(false);}
	ChainNarratorNode & nextInChain() {assert(false);}
	bool isFirst() const{assert(false);}
	bool isLast() const{assert(false);}
	int getIndex() const{assert(false);}
	QString rank2String() const{assert(false);}
	QString CanonicalName() const{assert(false);}
	int getChainNum() const{assert(false);}
	ChainNarratorNode & getChainNodeInChain(int ) {assert(false);}
	QString toString() const{	return "NULLChainNarratorNodeIterator";}
	void setVisited(unsigned int ) { assert(false);}
	void resetVisited(unsigned int ) { assert(false); }
	bool isVisited(unsigned int )  const{ assert(false); }
	void resetColor() { assert(false); }
};

class GraphNarratorNode: public NarratorNodeIfc
{
protected:
	QList<ChainNarratorNode *>  equalChainNodes;
	int savedRank;
	virtual void setRank(int rank){
		savedRank=rank;
		for (int i=0;i<equalChainNodes.size();i++)
			equalChainNodes[i]->setRank(rank);
	}
	virtual int getAutomaticRank() const
	{
		int smallest_rank=equalChainNodes[0]->getAutomaticRank();
		for (int i=1;i<equalChainNodes.size();i++)
		{
			int rank=equalChainNodes[i]->getAutomaticRank();
			if (smallest_rank<rank)
				smallest_rank=rank;
		}
		return smallest_rank;
	}
	virtual int getSavedRank() const{return savedRank;}
	friend class LoopBreakingVisitor;

	virtual void serializeHelper(QDataStream &chainOut, NarratorGraph & graph) const;
	virtual void deserializeHelper(QDataStream &chainIn,NarratorGraph & graph) ;

	GraphNarratorNode():NarratorNodeIfc(){} //to be used by NULLGraphNarratorNode
	friend class NarratorNodeIfc;
public:
	//GraphNarratorNode(){savedRank=-1;}
	GraphNarratorNode(ChainNarratorNode & nar1,ChainNarratorNode & nar2):NarratorNodeIfc()
	{
		assert(nar1.graphNode==NULL);
		assert(nar2.graphNode==NULL);
		assert(&nar1!=&nar2); //make sure these are not just the same node
		equalChainNodes.append(&nar1);
		nar1.setCorrespondingNarratorNode(this);
		equalChainNodes.append(&nar2);
		nar2.setCorrespondingNarratorNode(this);
		savedRank=-1;
	}
	virtual void addNarrator(ChainNarratorNode & nar) //we dont check for duplicates here
	{
	#if 0
		for (int i=0;i<size();i++)
		{
			ChainNarratorNode * n2=&(*this)[i];
			ChainNarratorNode * n1=&nar;
			if (n1==n2)
				return;
			//assert (n1!=n2); //TODO...
		}
	#endif
		//assert(nar.graphNode==NULL);
		equalChainNodes.append(&nar);
		nar.setCorrespondingNarratorNode(this);
	}
	virtual NarratorNodeIfc & getCorrespondingNarratorNode() {return *this;}
	virtual int size() const{ return equalChainNodes.size(); }
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
	virtual QString rank2String() const
	{
		QString ranks;
	#ifdef SHOW_VERBOSE_RANKS
		ranks="[";
		for (int i=0;i<equalChainNodes.size();i++)
			ranks+=QString("%1,").arg(equalChainNodes[i]->getAutomaticRank());
		ranks+="]";
	#endif
		ranks+=QString("(%1)").arg(getSavedRank());
		return ranks;
	}
	virtual QString CanonicalName() const
	{
	#ifdef REFINEMENTS
		if (isRasoul(equalChainNodes[0]->CanonicalName()))
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
	virtual bool isGraphNode() const {return true;}
	virtual ChainNarratorNode & getChainNodeInChain(int chain_num)
	{
		for (int i=0;i<equalChainNodes.size();i++)
			if (equalChainNodes[i]->getChainNum()==chain_num)
				return *equalChainNodes[i];
		return nullChainNarratorNode;
	}
	virtual QString toString() const
	{
		QString s=QString("[");
		for (int i=0;i<equalChainNodes.size();i++)
			s+=equalChainNodes[i]->toString();
		s+="]";
		return s;
	}
	virtual bool isNull() const{return false;}
};

class NULLGraphNarratorNode: public GraphNarratorNode
{
protected:
	int getSavedRank() const{assert(false);}
	int getAutomaticRank() const{assert(false);}
	void setRank(int ){assert(false);}
	virtual void serializeHelper(QDataStream &,NarratorGraph & ) const {}
	virtual void deserializeHelper(QDataStream &,NarratorGraph & ) {}
public:
	NULLGraphNarratorNode(){}
	bool isNull() const {return true;}
	void addNarrator(ChainNarratorNode &) {assert(false);}
	NarratorNodeIfc & getCorrespondingNarratorNode() const {assert(false);}
	int size() const {return 0;}
	NarratorNodeIfc & getChild(int) const{assert(false);}
	NarratorNodeIfc & getParent(int) const{assert(false);}
	ChainNarratorNode & operator [](int) const{assert(false);}
	NodeAddress prevInChain(ChainNarratorNode & ) const{assert(false);}
	NodeAddress nextInChain(ChainNarratorNode & ) const{assert(false);}
	QString rank2String() const{assert(false);}
	QString CanonicalName() const{assert(false);}
	ChainNarratorNode & getChainNodeInChain(int ) const{assert(false);}
	QString toString() const{	return "NULLChainNarratorNodeIterator";}
	void setVisited(unsigned int ) { assert(false);}
	void resetVisited(unsigned int ) { assert(false); }
	bool isVisited(unsigned int ) const{ assert(false); }
	void resetColor() { assert(false); }
};

#endif // GRAPH_STRUCTURE_H

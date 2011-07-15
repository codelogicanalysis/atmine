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
class GroupNode;
class NULLGraphNarratorNode;
class NULLNarratorNodeIfc;
class NULLChainNarratorNode;
class ChainNodeIterator;
class GraphNodeItem;
class NarratorGraph;

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
	unsigned int id;
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
	NarratorNodeIfc(NarratorGraph & g);
	friend class NarratorGraph;
	friend class ColorIndices;
	friend class GraphVisitorController;

	virtual void serializeHelper(QDataStream &chainOut, NarratorGraph & graph) const=0;
	virtual void deserializeHelper(QDataStream &chainIn,NarratorGraph & graph) =0;


	friend class ChainNodeIterator;
	virtual NarratorNodeIfc & getChild(int index1,int index2)=0;
	virtual NarratorNodeIfc & getParent(int index1,int index2)=0;
	virtual NarratorNodeIfc & getChild(const QPair<int,int> & p){
		return getChild(p.first,p.second);
	}
	virtual NarratorNodeIfc & getParent(const QPair<int,int> & p){
		return getParent(p.first,p.second);
	}

	NarratorNodeIfc() {resetColor();}
public:
	virtual NarratorNodeIfc & getCorrespondingNarratorNode()=0; //if used on a graphNode returns null

	virtual int size() const=0;
	virtual GraphNodeItem & operator [](int index)=0;
	virtual ChainNodeIterator begin();
	void setId(unsigned int aId) { id = aId;}
	unsigned int  getId() const {
		if (this==NULL)
			return 0;
		return id;
	}
	//virtual NodeAddress prevInChain(ChainNarratorNode &)=0;
	//virtual NodeAddress nextInChain(ChainNarratorNode &) =0;

	virtual QString CanonicalName() const=0;
	virtual bool isNull() const=0;
	virtual bool isGraphNode() const{return false;}
	virtual bool isChainNode() const{return false;}
	virtual bool isGroupNode() const{return false;}
	virtual bool isActualNode() const {return false;}

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
		chainOut<<isChainNode();
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
	NarratorNodeIfc & getChild(int,int ) {assert(false);}
	NarratorNodeIfc & getParent(int,int) {assert(false);}
	GraphNodeItem & operator [](int){assert(false);}

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
	virtual QString toString() const   {return "NULLNarratorNodeIfc";}
	void setVisited(unsigned int ) { assert(false);}
	void resetVisited(unsigned int ) { assert(false); }
	bool isVisited(unsigned int ) const{ assert(false); }
	void resetColor() { assert(false); }

	virtual void serializeHelper(QDataStream &,NarratorGraph & ) const {}
	virtual void deserializeHelper(QDataStream &,NarratorGraph & ) {}
};

class GraphNodeItem: public NarratorNodeIfc { //abstract class
protected:
	GraphNodeItem(){}
public:
	GraphNodeItem(NarratorGraph&g):NarratorNodeIfc (g){}
};

class ChainNarratorNode:public GraphNodeItem {
private:
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

private:
	ChainNarratorNode * previous, * next;
	Narrator * narrator;

	GroupNode * group;
	int savedRank;
	friend class NarratorGraph;
	friend class GroupNode;

	ChainContext chainContext;
	friend void buildChainNodes(ChainsContainer & chains);
protected:

	ChainNarratorNode():GraphNodeItem() { //to be used by NULLChainNarratorNode
		previous=next=NULL;
		group=NULL;
		narrator=NULL;
		savedRank=-1;
		group=NULL;
		chainContext.set(-1, -1);
	}
	friend class NarratorNodeIfc;
	friend class NarratorHash;

	virtual int getSavedRank() const ;
	virtual int getAutomaticRank() const {
		int savedRank=getSavedRank();
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
	virtual void setRank(int rank);
	virtual void setIndex(int index){chainContext.set(index, chainContext.getChainNum());}
	virtual void setChainNum(int num){chainContext.set(chainContext.getIndex(), num);}
	friend class GraphNarratorNode;

	virtual void serializeHelper(QDataStream &chainOut, NarratorGraph & graph) const;
	virtual void deserializeHelper(QDataStream &chainIn,NarratorGraph & graph) ;
public:

	ChainNarratorNode(NarratorGraph&g, Narrator * n,int index, int chain_num)
		:GraphNodeItem(g)
	{
		previous=next=NULL;
		group=NULL;
		narrator=n;
		savedRank=-1;
		group=NULL;
		chainContext.set(index, chain_num);
	}
	virtual NarratorNodeIfc & getCorrespondingNarratorNode() ;
	virtual void  setCorrespondingNarratorNodeGroup(GroupNode * group) {
		this->group=group;
	}
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
	virtual NarratorNodeIfc & getChild (int index1,int index2);
	virtual NarratorNodeIfc & getParent(int index1,int index2);
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
	virtual bool isNull() const {return false;}
	virtual bool isChainNode() const{return true;}
	virtual bool isActualNode() const { return group==NULL;	}
	virtual ChainNarratorNode & getChainNodeInChain(int chain_num);
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
	void  setCorrespondingNarratorNodeGroup(GroupNode * ) {assert(false);}
	Narrator & getNarrator() const{assert(false);}
	ChainNarratorNode & operator+(int) {assert(false);}
	ChainNarratorNode & operator-(int) {assert(false);}
	int size() const {return 0;}
	NarratorNodeIfc & getChild(int,int) {assert(false);}
	NarratorNodeIfc & getParent(int,int) {assert(false);}
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

class GroupNode:public GraphNodeItem {
private:
	GraphNarratorNode * graphNode;
	QString key;
	QList<ChainNarratorNode *> list;
private:
	friend class GraphNarratorNode;
	friend class ChainNarratorNode;
	friend class NarratorNodeIfc;
	void setRank(int rank);
protected:
	friend class NarratorHash;
	GroupNode():GraphNodeItem() {
		graphNode=NULL;
	}
	GroupNode(NarratorGraph&g,GraphNarratorNode * gNode, ChainNarratorNode * cNode,QString key)
		:GraphNodeItem(g),graphNode(gNode) {
		assert(gNode==NULL || ((NarratorNodeIfc*)gNode)->isGraphNode());
		if (gNode!=NULL && ((NarratorNodeIfc*)gNode)->getId()==1173)
			qDebug()<<"check";
		list.append(cNode);
		this->key=key;
		cNode->setCorrespondingNarratorNodeGroup(this);
	}
	GroupNode(NarratorGraph&g,GraphNarratorNode * gNode, ChainNarratorNode * cNode)
		:GraphNodeItem(g),graphNode(gNode) {
		assert(gNode==NULL || ((NarratorNodeIfc*)gNode)->isGraphNode());
		if (gNode!=NULL && ((NarratorNodeIfc*)gNode)->getId()==1173)
			qDebug()<<"check";
		list.append(cNode);
		this->key=cNode->getNarrator().getKey();
		cNode->setCorrespondingNarratorNodeGroup(this);
	}

	virtual NarratorNodeIfc & getChild(int index1,int ) {
		assert(false);
		return list[index1]->getChild(0,0);
	}
	virtual NarratorNodeIfc & getParent(int index1,int ){
		return list[index1]->getParent(0,0);
	}
	virtual NarratorNodeIfc & getChild(const QPair<int,int> & p){
		return getChild(p.first,p.second);
	}
	virtual NarratorNodeIfc & getParent(const QPair<int,int> & p){
		return getParent(p.first,p.second);
	}

public:
	ChainNarratorNode & operator [](int index) {
		assert(index>=0 && index<size());//check redundant will be done inside [] for QList
		return *list[index];
	}
	void addChainNode(NarratorGraph */*g*/,ChainNarratorNode & nar) { //we dont check for duplicates here
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
		list.append(&nar);
		nar.setCorrespondingNarratorNodeGroup(this);
	}
	virtual NarratorNodeIfc & getCorrespondingNarratorNode() ;
	int size() const{ return list.size(); }
	QString CanonicalName() const {
	#ifdef REFINEMENTS
		if (isRasoul(list[0]->CanonicalName()))
			return alrasoul;
	#endif
	#ifdef SMALLEST_CANONICAL
		int smallestsize=list[0]->CanonicalName().size(), index=0;
		for (int i=1;i<list.size();i++) {
			int size=list[i]->CanonicalName().size();
			if (smallestsize>size) {
				smallestsize=size;
				index=i;
			}
		}
	#else
		int largestsize=list[0].CanonicalName().size(), index=0;
		for (int i=1;i<list.size();i++) {
			int size=list[i].CanonicalName().size();
			if (largestsize<size) {
				largestsize=size;
				index=i;
			}
		}
	#endif
		if (index>=0) {
			return list[index]->CanonicalName();
		}
		else
			return "";
	}
	ChainNarratorNode & getChainNodeInChain(int chain_num) {
		for (int i=0;i<list.size();i++)
			if (list[i]->getChainNum()==chain_num)
				return *list[i];
		return nullChainNarratorNode;
	}
	QString toString() const {
		QString s=QString("<");
		for (int i=0;i<list.size();i++)
			s+=list[i]->toString();
		s+=">";
		return s;
	}
	QString getKey() { return key; }
	int getSavedRank() const;
	int getAutomaticRank() const
	{
		int smallest_rank=list[0]->getAutomaticRank();
		for (int i=1;i<list.size();i++)
		{
			int rank=list[i]->getAutomaticRank();
			if (smallest_rank<rank)
				smallest_rank=rank;
		}
		return smallest_rank;
	}
	void setGraphNode(GraphNarratorNode * node) {
		assert(node==NULL || ((NarratorNodeIfc*)node)->isGraphNode());
		if (node!=NULL && ((NarratorNodeIfc*)node)->getId()==1173)
			qDebug()<<"check";
		this->graphNode=node;
	}
	virtual bool isNull() const{ return false;}

	virtual void serializeHelper(QDataStream &chainOut, NarratorGraph & graph) const;
	virtual void deserializeHelper(QDataStream &chainIn,NarratorGraph & graph);

	virtual QString rank2String() const	{
		QString ranks;
	#ifdef SHOW_VERBOSE_RANKS
		ranks="[";
		for (int i=0;i<list.size();i++)
			ranks+=QString("%1,").arg(list[i]->getAutomaticRank());
		ranks+="]";
	#endif
		ranks+=QString("(%1)").arg(getSavedRank());
		return ranks;
	}
	virtual bool isGroupNode() const{return true;}
};

class GraphNarratorNode: public NarratorNodeIfc
{
protected:
	QList<GroupNode *>  groupList;
	int savedRank;
	virtual void setRank(int rank){
		savedRank=rank;
	#if 0
		for (int i=0;i<size();i++) {
			for (int j=0;j<groupList[i]->size();j++)
				(*groupList[i])[j].setRank(rank);
		}
	#endif
	}
	virtual int getAutomaticRank() const
	{
		int smallest_rank=groupList[0]->getAutomaticRank();
		for (int i=1;i<groupList.size();i++)
		{
			int rank=groupList[i]->getAutomaticRank();
			if (smallest_rank<rank)
				smallest_rank=rank;
		}
		return smallest_rank;
	}
	virtual int getSavedRank() const{return savedRank;}
	friend class LoopBreakingVisitor;

	virtual void serializeHelper(QDataStream &chainOut, NarratorGraph & graph) const;
	virtual void deserializeHelper(QDataStream &chainIn,NarratorGraph & graph) ;

	GraphNarratorNode():NarratorNodeIfc(){savedRank=-1;} //to be used by NULLGraphNarratorNode
	friend class NarratorNodeIfc;
	friend class GroupNode;
public:
	GraphNarratorNode(NarratorGraph &g,ChainNarratorNode & nar1,ChainNarratorNode & nar2)
		:NarratorNodeIfc(g)
	{
		assert(nar1.group==NULL);
		assert(nar2.group==NULL);
		assert(&nar1!=&nar2); //make sure these are not just the same node
		addChainNode(&g,nar1);
		addChainNode(&g,nar2);
		savedRank=-1;
	}
	GraphNarratorNode(NarratorGraph &graph,GroupNode & g)
		:NarratorNodeIfc(graph)
	{
		assert(g.graphNode==NULL);
		assert(g.size()>1); //not allowed to have just one chain node
		groupList.append(&g);
		g.setGraphNode(this);
		savedRank=-1;
	}
	GraphNarratorNode(NarratorGraph &graph,GroupNode & g1,GroupNode & g2)
		:NarratorNodeIfc(graph)
	{
		assert(g1.graphNode==NULL);
		assert(g2.graphNode==NULL);
		assert(&g1!=&g2); //make sure these are not just the same node
		groupList.append(&g1);
		groupList.append(&g2);
		g1.setGraphNode(this);
		g2.setGraphNode(this);
		savedRank=-1;
	}
	virtual void addChainNode(NarratorGraph *g, ChainNarratorNode & nar) { //we dont check for duplicates here
		QString key=nar.getNarrator().getKey();
		for (int i=0;i<size();i++) {
			GroupNode & group=(*this)[i];
			if (group.getKey()==key) {
				group.addChainNode(g,nar);
				return;
			}
		}
		GroupNode * newGoup=new GroupNode(*g,this,&nar,key);
		groupList.append(newGoup);
	}
	virtual NarratorNodeIfc & getCorrespondingNarratorNode() {return *this;}
	virtual int size() const{ return groupList.size(); }
	virtual NarratorNodeIfc & getChild(int index1,int index2)
	{
		assert(index1>=0 && index1<size());//check redundant will be done inside [] for QList
		assert(index2>=0 && index2<groupList[index1]->size());
		ChainNarratorNode & c =(*groupList[index1])[index2].nextInChain();
		return (c.isNull()
					?nullChainNarratorNode
					:c.getCorrespondingNarratorNode());
	}
	virtual NarratorNodeIfc & getParent(int index1,int index2)
	{
		assert(index1>=0 && index1<size());//check redundant will be done inside [] for QList
		assert(index2>=0 && index2<groupList[index1]->size());
		ChainNarratorNode & c =(*groupList[index1])[index2].prevInChain();
		return (c.isNull()
					?nullChainNarratorNode
					:c.getCorrespondingNarratorNode());
	}
	virtual GroupNode & operator [](int index)
	{
		assert(index>=0 && index<size());//check redundant will be done inside [] for QList
		return *groupList[index];
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
		for (int i=0;i<groupList.size();i++)
			ranks+=QString("%1,").arg(groupList[i]->getAutomaticRank());
		ranks+="]";
	#endif
		ranks+=QString("(%1)").arg(getSavedRank());
		return ranks;
	}
	virtual QString CanonicalName() const
	{
	#ifdef REFINEMENTS
		if (isRasoul(groupList[0]->CanonicalName()))
			return alrasoul;
	#endif
	#ifdef SMALLEST_CANONICAL
		//qDebug()<<"---";
		int smallestsize=groupList[0]->CanonicalName().size(), index=0;
		//qDebug()<<"("<<equalnarrators[0].CanonicalName();
		for (int i=1;i<groupList.size();i++)
		{
			int size=groupList[i]->CanonicalName().size();
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
			return groupList[index]->CanonicalName();
		}
		else
			return "";
	}
	virtual bool isGraphNode() const {return true;}
	virtual bool isActualNode() const { return true;}
	virtual ChainNarratorNode & getChainNodeInChain(int chain_num)
	{
		for (int i=0;i<groupList.size();i++) {
			for (int j=0;j<groupList[i]->size();j++) {
				ChainNarratorNode & c=(*groupList[i])[j];
				if (c.getChainNum()==chain_num)
					return c;
			}
		}
		return nullChainNarratorNode;
	}
	virtual QString toString() const
	{
		QString s=QString("[");
		for (int i=0;i<groupList.size();i++)
			s+=groupList[i]->toString();
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
	void addChainNode(NarratorGraph* ,ChainNarratorNode &) {assert(false);}
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

class ChainNodeIterator {
private:
	NarratorNodeIfc * node;
	int i,j;
private:

	friend class NarratorNodeIfc;
	ChainNodeIterator(NarratorNodeIfc * n,int index1,int index2):node(n),i(index1),j(index2) {}
public:
	typedef QPair<int,int> IndiciesPair;
public:
	ChainNodeIterator & operator++() {
		if(i>=node->size())
			return *this; //force reaching end to be last iteration
		j++;
		if (j==(*node)[i].size()) {
			j=0;
			i++;
		}
		return *this;
	}
	bool isFinished() const {
		return (i>= node->size());
	}
	ChainNarratorNode & operator * () {
		return (ChainNarratorNode &)(*node)[i][j];
	}
	ChainNarratorNode * getChainNode() {
		return (ChainNarratorNode *)&(*node)[i][j];
	}
	NarratorNodeIfc & getChild() {
		return node->getChild(i,j);
	}
	NarratorNodeIfc & getParent() {
		return node->getParent(i,j);
	}
	int getGroupIndex() const {
		return i;
	}
	int getChainIndex() const {
		return j;
	}
	NarratorNodeIfc * getBaseNode() const {
		return node;
	}
	IndiciesPair getIndicies() const {
		return IndiciesPair(i,j);
	}
	bool operator ==( const ChainNodeIterator & rhs) const {
		return (node==rhs.node && i==rhs.i && j==rhs.j);
	}

	bool isNull() const{
	#if 0
		return this==&null;
	#else
		return operator ==(null);
	#endif
	}

	static ChainNodeIterator null;
};


#endif // GRAPH_STRUCTURE_H

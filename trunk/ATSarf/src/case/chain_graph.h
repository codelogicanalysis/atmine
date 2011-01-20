#ifndef CHAIN_GRAPH_H
#define CHAIN_GRAPH_H

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

#include "hadith.h"

#define SMALLEST_CANONICAL

class NarratorNodeIfc;
class ChainNarratorNode;
class ChainNarratorNodeIterator;
class GraphNarratorNode;
class NULLGraphNarratorNode;
class NULLNarratorNodeIfc;
class NULLChainNarratorNodeIterator;
class RankCorrectorNodeVisitor;

class Chain;
class Narrator;
class ChainPrim;


typedef QList<Chain *> ChainsContainer;
typedef QPair<NarratorNodeIfc &, ChainNarratorNodeIterator &> NodeAddress;
typedef QList<GraphNarratorNode *> NarratorNodesList;
typedef Reference<NarratorNodeIfc> NarratorNodeIfcRfc;


#define nullNodeAddress NodeAddress(nullNarratorNodeIfc,nullChainNarratorNodeIterator)
extern ChainsContainer chains;

extern NULLNarratorNodeIfc nullNarratorNodeIfc;
extern NULLGraphNarratorNode nullGraphNarratorNode;
extern NULLChainNarratorNodeIterator nullChainNarratorNodeIterator;

void fillRank(Narrator &n, int index, bool last);
void fillRanks();
void buildGraph(ChainsContainer & chs);

class NarratorNodeIfc //abstract interface
{
protected:
	virtual void setSavedRank(int rank)=0;
	virtual int getSavedRank()=0;
	virtual int getAutomaticRank()=0;
	friend class RankCorrectorNodeVisitor;
public:
	virtual NarratorNodeIfc & firstChild()=0;
	virtual NarratorNodeIfc & nextChild(NarratorNodeIfc & current)=0;
	virtual NarratorNodeIfc & firstParent()=0;
	virtual NarratorNodeIfc & nextParent(NarratorNodeIfc & current)=0;

	virtual int getNumChildren()=0;
	virtual NarratorNodeIfcRfc getChild(int index)=0;

	virtual ChainNarratorNodeIterator & firstNarrator()=0;
	virtual ChainNarratorNodeIterator & nextNarrator(ChainNarratorNodeIterator & current)=0;

	virtual NodeAddress prevInChain(ChainNarratorNodeIterator &)=0;
	virtual NodeAddress nextInChain(ChainNarratorNodeIterator &)=0;

	virtual QString CanonicalName()=0;
	virtual bool isNull()=0;
	virtual bool isGraphNode()=0;

	virtual int getGraphRank()
	{
		int savedRank=getSavedRank();
		return (savedRank>=0?savedRank:getAutomaticRank());
	}
	virtual QString getRanks()=0;

	virtual ChainNarratorNodeIterator & getChainNodeItrInChain(int chain_num)=0;
	virtual QString toString() =0;
};

class NULLNarratorNodeIfc: public NarratorNodeIfc
{
	NarratorNodeIfc & firstChild(){throw 1;}
	NarratorNodeIfc & nextChild(NarratorNodeIfc & ){throw 1;}
	NarratorNodeIfc & firstParent(){throw 1;}
	NarratorNodeIfc & nextParent(NarratorNodeIfc & ){throw 1;}

	int getNumChildren(){throw 1;}
	NarratorNodeIfcRfc getChild(int ){throw 1;}

	ChainNarratorNodeIterator & firstNarrator(){throw 1;}
	ChainNarratorNodeIterator & nextNarrator(ChainNarratorNodeIterator & ){throw 1;}

	NodeAddress prevInChain(ChainNarratorNodeIterator &){throw 1;}
	NodeAddress nextInChain(ChainNarratorNodeIterator &){throw 1;}

	QString CanonicalName(){throw 1;}

	virtual bool isNull(){throw 1;}
	void setSavedRank(int ){throw 1;}
	int getAutomaticRank(){throw 1;}
	int getGraphRank() {throw 1;}
	int getSavedRank(){throw 1;}
	virtual QString getRanks(){throw 1;}
	virtual ChainNarratorNodeIterator & getChainNodeItrInChain(int){throw 1;}
	virtual bool isGraphNode() {return false;}
	virtual QString toString()   {return "NULLNarratorNodeIfc";}
};

class ChainNarratorNode
{
protected:
	GraphNarratorNode * narrNode;
	int savedRank;
public:
	ChainNarratorNode()
	{
		narrNode=NULL;
		savedRank=-1;
	}
	GraphNarratorNode & getCorrespondingNarratorNode();
	void  setCorrespondingNarratorNode(GraphNarratorNode * narrNode)
	{
		this->narrNode=narrNode;
	}
	int getSavedRank()
	{
		return savedRank;
	}
	void setSavedRank(int rank)
	{
		savedRank=rank;
	}
	QString CanonicalName() ;
	virtual QString toString()
	{
		return "("+CanonicalName()+")";
	}
};

class ChainNarratorNodeIterator:public QList<ChainPrim *>::iterator, public NarratorNodeIfc //Maybe later form by composition instead of inheretence
{
private:
	ChainPrim* getChainPrimPtr()
	{
		QList<ChainPrim *>::iterator * s=this;
		return *(*s);
	}
	ChainPrim & getChainPrim()
	{
		return *getChainPrimPtr();
	}
	friend class GraphNarratorNode;
public:
	ChainNarratorNodeIterator(){}
	ChainNarratorNodeIterator(Chain *ch, Narrator * n);
	ChainNarratorNodeIterator(QList<ChainPrim *>::iterator it); //Note is provided an iterator that is at end may result in unexpected behaviour
	ChainNarratorNodeIterator & nearestNarratorInChain(bool next=true);//TODO: create firstNarrator() equivalent to begin
	Narrator & getNarrator()
	{
		return (Narrator &)getChainPrim();
	}
	ChainNarratorNode & operator*();
	ChainNarratorNode * operator->()
	{
		return &this->operator *();
	}
	ChainNarratorNodeIterator & operator++();
	ChainNarratorNodeIterator & operator--();
	ChainNarratorNodeIterator & operator+(int);
	ChainNarratorNodeIterator & operator-(int);
	ChainNarratorNodeIterator & prevInChain()
	{
		return (this->operator --());
	}
	NarratorNodeIfc & firstChild();
	NarratorNodeIfc & nextChild(NarratorNodeIfc & )
	{
		return nullNarratorNodeIfc;
	}

	int getNumChildren()
	{
		if (!isLast())
			return 1;
		else
			return 0;
	}
	NarratorNodeIfcRfc getChild(int index);

	NarratorNodeIfc & firstParent();
	NarratorNodeIfc & nextParent(NarratorNodeIfc & )
	{
		return nullNarratorNodeIfc;
	}
	ChainNarratorNodeIterator & firstNarrator();
	ChainNarratorNodeIterator & nextNarrator(ChainNarratorNodeIterator &);
	NodeAddress prevInChain(ChainNarratorNodeIterator & node);
	NodeAddress nextInChain(ChainNarratorNodeIterator & node);
	ChainNarratorNodeIterator & nextInChain();
	/*{
		return (this->operator ++());
	}*/
	QString CanonicalName()
	{
		return (*this)->CanonicalName();
	}
	NarratorNodeIfc &  getCorrespondingNarratorNode();
	bool isFirst();
	bool isLast();
	int getIndex();
	int getAutomaticRank(){return getIndex();}
	void setSavedRank(int rank)
	{
		(*this)->setSavedRank(rank);
	}
	int getSavedRank(){return (*this)->getSavedRank();}
	QString getRanks()
	{
	#ifdef SHOW_VERBOSE_RANKS
		return QString("[%1](%2)").arg(getAutomaticRank()).arg(getSavedRank());
	#else
		return QString("(%1)").arg(getSavedRank());
	#endif

	}
	int getChainNum();
	virtual bool isNull()
	{
		return false;
	}
	virtual bool isGraphNode() { return false;}
	virtual ChainNarratorNodeIterator & getChainNodeItrInChain(int chain_num);
	virtual QString toString()
	{
		return (*this)->toString();
	}
};

class NULLChainNarratorNodeIterator: public ChainNarratorNodeIterator
{
public:
	virtual bool isNull() {	return true;}
	ChainNarratorNodeIterator & nearestNarratorInChain(bool =true){throw 1;}
	Narrator & getNarrator(){throw 1;}
	ChainNarratorNode & operator*(){throw 1;}
	ChainNarratorNode * operator->(){throw 1;}
	ChainNarratorNodeIterator & operator++(){throw 1;}
	ChainNarratorNodeIterator & operator--(){throw 1;}
	ChainNarratorNodeIterator & operator+(int){throw 1;}
	ChainNarratorNodeIterator & operator-(int){throw 1;}
	ChainNarratorNodeIterator & prevInChain(){throw 1;}
	NarratorNodeIfc & firstChild(){throw 1;}
	NarratorNodeIfc & nextChild(NarratorNodeIfc & ){throw 1;}
	int getNumChildren() {return 0;}
	NarratorNodeIfcRfc getChild(int){throw 1;}
	NarratorNodeIfc & firstParent(){throw 1;}
	NarratorNodeIfc & nextParent(NarratorNodeIfc & ){throw 1;}
	ChainNarratorNodeIterator & firstNarrator(){throw 1;}
	ChainNarratorNodeIterator & nextNarrator(ChainNarratorNodeIterator &){throw 1;}
	NodeAddress prevInChain(ChainNarratorNodeIterator & ){throw 1;}
	NodeAddress nextInChain(ChainNarratorNodeIterator & ){throw 1;}
	ChainNarratorNodeIterator & nextInChain(){throw 1;}
	bool isFirst(){throw 1;}
	bool isLast(){throw 1;}
	int getIndex(){throw 1;}
	QString getRanks(){throw 1;}
	void setSavedRank(int ){throw 1;}
	int getSavedRank(){throw 1;}
	QString CanonicalName(){throw 1;}
	int getChainNum(){throw 1;}
	virtual ChainNarratorNodeIterator & getChainNodeItrInChain(int ){throw 1;}
	virtual QString toString(){	return "NULLChainNarratorNodeIterator";}
};

class LoopBreakingVisitor;

class GraphNarratorNode: public NarratorNodeIfc
{
protected:
	QList<ChainNarratorNodeIterator>  equalnarrators;
	int savedRank;
	void setSavedRank(int rank){savedRank=rank;}
	friend class LoopBreakingVisitor;
public:
	GraphNarratorNode(){savedRank=-1;}
	GraphNarratorNode(ChainNarratorNodeIterator & nar1,ChainNarratorNodeIterator & nar2);
	GraphNarratorNode(Chain * chain1, Narrator * nar1,Chain * chain2, Narrator * nar2);
	void addNarrator(Chain * chain1, Narrator * nar1);
	void addNarrator(ChainNarratorNodeIterator & nar1);
	NarratorNodeIfc & firstChild() //return iterator instead of NarratorNode
	{
		return (*((*(equalnarrators.begin())).nextInChain())).getCorrespondingNarratorNode();
	}
	NarratorNodeIfc & nextChild(NarratorNodeIfc & current)
	{
		assert(current.firstNarrator().isNull());//=>  is ChainNarratorNodePtr
		//check if end reached
		if (!(((ChainNarratorNodeIterator &)current).isLast()))
			return current.nextNarrator((ChainNarratorNodeIterator&)current).nextInChain().getCorrespondingNarratorNode();
		else
			return nullNarratorNodeIfc;
	}
	int getNumChildren()
	{
		return equalnarrators.size();
	}
	NarratorNodeIfcRfc getChild(int index)
	{
		assert(index>=0 && index<getNumChildren());
		ChainNarratorNodeIterator & c=*(new ChainNarratorNodeIterator(equalnarrators[index]));
		if((++c).isNull())
		{
			delete &c;
			return NarratorNodeIfcRfc(nullChainNarratorNodeIterator,false);
		}
		else
		{
			NarratorNodeIfc & n=c.getCorrespondingNarratorNode();
			if (n.isGraphNode())
				delete &c;
			return NarratorNodeIfcRfc(n,!n.isGraphNode());
		}
	}
	NarratorNodeIfc & firstParent()
	{
		return (*((*(equalnarrators.begin())).prevInChain())).getCorrespondingNarratorNode();
	}
	NarratorNodeIfc & nextParent(NarratorNodeIfc & current)
	{
		assert(current.firstNarrator().isNull());//=>  is ChainNarratorNodePtr
		//check if end reached
		if (!(((ChainNarratorNodeIterator &)current).isFirst()))
			return current.nextNarrator((ChainNarratorNodeIterator &)current).prevInChain().getCorrespondingNarratorNode();
		else
			return nullNarratorNodeIfc;
	}
	ChainNarratorNodeIterator & firstNarrator()
	{
		return *equalnarrators.begin();
	}
	ChainNarratorNodeIterator & nextNarrator(ChainNarratorNodeIterator & current)
	{
		if (!(current.isLast()))
			return current.nextNarrator(current);
		else
			return nullChainNarratorNodeIterator;
	}
	NodeAddress prevInChain(ChainNarratorNodeIterator & node)
	{
		ChainNarratorNodeIterator prev=node.prevInChain();
		return NodeAddress(prev.getCorrespondingNarratorNode(), prev);
	}
	NodeAddress nextInChain(ChainNarratorNodeIterator & node)
	{
		ChainNarratorNodeIterator next=node.nextInChain();
		return NodeAddress(next.getCorrespondingNarratorNode (), next);
	}
	int getAutomaticRank()
	{
		int largest_rank=equalnarrators[0].getAutomaticRank();
		for (int i=1;i<equalnarrators.size();i++)
		{
			int rank=equalnarrators[i].getAutomaticRank();
			if (largest_rank<rank)
				largest_rank=rank;
		}
		return largest_rank;
	}
	int getSavedRank(){return savedRank;}
	QString getRanks()
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
	QString CanonicalName()
	{
	#ifdef REFINEMENTS
		if (rasoul_words.contains(equalnarrators[0].CanonicalName()))
			return alrasoul;
	#endif
	#ifdef SMALLEST_CANONICAL
		//qDebug()<<"---";
		int smallestsize=equalnarrators[0].CanonicalName().size(), index=0;
		//qDebug()<<"("<<equalnarrators[0].CanonicalName();
		for (int i=1;i<equalnarrators.size();i++)
		{
			int size=equalnarrators[i].CanonicalName().size();
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
			return equalnarrators[index].CanonicalName();
		}
		else
			return "";
	}
	virtual bool isGraphNode() {return true;}
	virtual ChainNarratorNodeIterator & getChainNodeItrInChain(int chain_num)
	{
		for (int i=0;i<equalnarrators.size();i++)
			if (equalnarrators[i].getChainNum()==chain_num)
				return equalnarrators[i];
		return nullChainNarratorNodeIterator;
	}
	virtual QString toString()
	{
		QString s=QString("[");
		for (int i=0;i<equalnarrators.size();i++)
			s+=equalnarrators[i].toString();
		s+="]";
		return s;
	}
	virtual bool isNull(){return false;}
};

class NULLGraphNarratorNode: public GraphNarratorNode
{
public:
	bool isNull()
	{
		return true;
	}
};

void mergeNodes(ChainNarratorNodeIterator & n1,ChainNarratorNodeIterator & n2);
double equal(const Narrator & n1,const Narrator & n2);

class GraphVisitorController;

class NodeVisitor
{
protected:
	GraphVisitorController  * controller;
	friend class GraphVisitorController;
public:
	virtual void initialize()=0;
	virtual void visit(NarratorNodeIfc & n)=0;
	virtual void visit(NarratorNodeIfc & n1,NarratorNodeIfc & n2, int child_num)=0;
	virtual void finish()=0;

	virtual void cycle_detected(){}//any cycle of length larger than 1 or 2 detected
	virtual void cycle_detected(NarratorNodeIfc &){}//any cycle of length larger than 1 or 2 detected
	virtual void cycle_detected(NarratorNodeIfc & ,NarratorNodeIfc &){}//any cycle of length larger than 1 or 2 detected
};

class NarratorGraph;
class LoopBreakingVisitor;

class GraphVisitorController
{
protected:
	typedef QMap<GraphNarratorNode*,int> G_IDMap;
	typedef QMap<ChainNarratorNode*,int> C_IDMap;
	typedef Triplet<ChainNarratorNode *,ChainNarratorNode*,int> CCEdge; //Triplet: not needed but for consistency purposes, Pair is enough
	typedef Triplet<ChainNarratorNode *,GraphNarratorNode*,int> CGEdge;
	typedef Triplet<GraphNarratorNode *,GraphNarratorNode*,int> GGEdge;
	typedef QMap<CGEdge, bool> CGEdgeMap; //(chain, graph) , will be used for (graph,chain) also
	typedef QMap<GGEdge, bool> GGEdgeMap; //(graph, graph)
	typedef QMap<CCEdge, bool> CCEdgeMap; //(graph, graph)
	G_IDMap GraphNodesID;
	C_IDMap ChainNodesID;
	CGEdgeMap cgEdgeMap;
	CGEdgeMap gcEdgeMap;
	GGEdgeMap ggEdgeMap;
	CCEdgeMap ccEdgeMap;
	bool keep_track_of_edges, keep_track_of_nodes,merged_edges_as_one, detect_cycles;
	int last_id;

	bool stop_searching_for_cycles;

	NodeVisitor * visitor;
	void init()
	{

		cgEdgeMap.clear();
		gcEdgeMap.clear();
		ggEdgeMap.clear();
		ccEdgeMap.clear();
		GraphNodesID.clear();
		ChainNodesID.clear();
		last_id=0;
	}
	//friend NarratorGraph::DFS_traverse(GraphVisitorController &visitor, bool detect_cycles);
	//friend NarratorGraph::DFS_traverse(NarratorNodeIfc &n, GraphVisitorController &visitor);
	friend class NarratorGraph;
	friend class LoopBreakingVisitor;

public:
	GraphVisitorController(NodeVisitor * visitor,bool keep_track_of_edges=true,bool keep_track_of_nodes=true, bool merged_edges_as_one=true/*, bool detect_cycles=false*/) //removed bc cotrol will be in the DFS algorithm code
	{
		this->visitor=visitor;
		assert(visitor!=NULL);
		this->visitor->controller=this;
		this->keep_track_of_edges=keep_track_of_edges;
		this->keep_track_of_nodes=keep_track_of_nodes;
		this->merged_edges_as_one=merged_edges_as_one;

		this->detect_cycles=false;
		stop_searching_for_cycles=false;
		//this->detect_cycles=detect_cycles;
		init();
	}
	int getUniqueNodeID(NarratorNodeIfc & n)//if not there returns -1
	{
		int curr_id;
		if (n.isGraphNode())
		{
			G_IDMap::iterator it=GraphNodesID.find((GraphNarratorNode*)&n);
			if (it==GraphNodesID.end())
				return -1;
			else
				curr_id=it.value();
		}
		else
		{
			C_IDMap::iterator it=ChainNodesID.find(&*(ChainNarratorNodeIterator&)n);
			if (it==ChainNodesID.end())
				return -1;
			else
				curr_id=it.value();
		}
		return curr_id;
	}
	int generateUniqueNodeID(NarratorNodeIfc & n) //if not there generates one and returns it
	{
		int curr_id;
		if (n.isGraphNode())
		{
			G_IDMap::iterator it=GraphNodesID.find((GraphNarratorNode*)&n);
			if (it==GraphNodesID.end())
			{
				curr_id=++last_id;
				GraphNodesID.insert((GraphNarratorNode*)&n,curr_id);
			}
			else
				curr_id=it.value();
		}
		else
		{
			C_IDMap::iterator it=ChainNodesID.find(&*(ChainNarratorNodeIterator&)n);
			if (it==ChainNodesID.end())
			{
				curr_id=++last_id;
				ChainNodesID.insert(&*(ChainNarratorNodeIterator&)n,curr_id);
			}
			else
				curr_id=it.value();
		}
		return curr_id;
	}
	bool isPreviouslyVisited( NarratorNodeIfc * node)//only for graph nodes
	{
		if (!keep_track_of_nodes)
			return false;
		if (!node->isGraphNode())
		{
			C_IDMap::iterator it=ChainNodesID.find(&**(ChainNarratorNodeIterator*)node);
			return !(it==ChainNodesID.end());
		}
		else
		{
			G_IDMap::iterator it=GraphNodesID.find((GraphNarratorNode*)node);
			return !(it==GraphNodesID.end());
		}
	}
	bool isPreviouslyVisited( NarratorNodeIfc * n1, NarratorNodeIfc * n2,int child_num)
	{
		if (!keep_track_of_edges)
			return false;
		if (merged_edges_as_one)
			child_num=1; //so that they will all be treated equally
		if (!n1->isGraphNode() && !n2->isGraphNode())
		{
			CCEdgeMap::iterator it=ccEdgeMap.find(CCEdge(&**((ChainNarratorNodeIterator*)n1),&**((ChainNarratorNodeIterator*)n2),child_num));
			return !(it==ccEdgeMap.end());
			return false;
		}
		else if (n1->isGraphNode() && !n2->isGraphNode() )
		{
			CGEdgeMap::iterator it=gcEdgeMap.find(CGEdge(&**((ChainNarratorNodeIterator*)n2),(GraphNarratorNode*)n1,child_num));
			return !(it==gcEdgeMap.end());
			return false;
		}
		else if (!n1->isGraphNode() && n2->isGraphNode())
		{
			CGEdgeMap::iterator it=cgEdgeMap.find(CGEdge(&**((ChainNarratorNodeIterator*)n1),(GraphNarratorNode*)n2,child_num));
			return !(it==cgEdgeMap.end());
			return false;
		}
		else
		{
			GGEdgeMap::iterator it=ggEdgeMap.find(GGEdge((GraphNarratorNode*)n1,(GraphNarratorNode*)n2,child_num));
			return !(it==ggEdgeMap.end());
		}
	}
	void initialize()
	{
		init();
		visitor->initialize();
	}
	void visit(NarratorNodeIfc & n1,NarratorNodeIfc & n2, int child_num)
	{
		if (merged_edges_as_one)
			child_num=1; //so that they will all be treated equally
		if (!isPreviouslyVisited(&n1,&n2,child_num))
			visitor->visit(n1,n2,child_num);

		if (!keep_track_of_edges)
			return;
		if (n1.isGraphNode())
		{
			if (!n2.isGraphNode())
				gcEdgeMap.insert(CGEdge(&*((ChainNarratorNodeIterator &)n2),&(GraphNarratorNode &)n1,child_num),true);
			else
				ggEdgeMap.insert(GGEdge(&(GraphNarratorNode &)n1,&(GraphNarratorNode &)n2,child_num),true);
		}
		else
		{
			if (n2.isGraphNode())
				cgEdgeMap.insert(CGEdge(&*((ChainNarratorNodeIterator &)n1),&(GraphNarratorNode &)n2,child_num),true);
			else
				ccEdgeMap.insert(CCEdge(&*((ChainNarratorNodeIterator &)n1),&*((ChainNarratorNodeIterator &)n2),child_num),true);
		}
	}
	void visit(NarratorNodeIfc & n)
	{
		if (!isPreviouslyVisited(&n))
			visitor->visit(n);

		if (!keep_track_of_nodes)
			return ;
		generateUniqueNodeID(n);
	}
	void finish()
	{
		visitor->finish();
	}
};

class NarratorNodeVisitor: public NodeVisitor
{
protected:
	QFile * file;
	QTextStream * dot_out;
	#define d_out (*dot_out)
	QList<QStringList> RanksList;
	int highest_rank;

	void setGraphRank(int rank, QString s)
	{
		while(rank>=RanksList.size())
			RanksList.append(QStringList());
		RanksList[rank].append(s);
	}
	QString getID(NarratorNodeIfc & n)
	{
		int curr_id=controller->getUniqueNodeID(n);
		QString name;
		if (curr_id==-1)
		{
			curr_id=controller->generateUniqueNodeID(n);
			if (n.isGraphNode())
			{
				d_out<<QString("g")<<curr_id<<" [label=\""<<n.CanonicalName().replace('\n',"");
			#ifdef SHOW_RANKS
				d_out<<n.getRanks();
			#endif
				d_out<<"\", shape=box];\n";
				name=QString("g%1").arg(curr_id);
			}
			else
			{
				d_out<<QString("c")<<curr_id<<" [label=\""<<n.CanonicalName().replace('\n',"");
			#ifdef SHOW_RANKS
				d_out<<n.getRanks();
			#endif
				d_out<<"\"]"<<";\n";
				name=QString("c%1").arg(curr_id);
			}
			if (n.getNumChildren()==0)
				setGraphRank(highest_rank,name);
			else
				setGraphRank(n.getGraphRank(),name);
			return name;
		}
		else
			return QString("%1%2").arg((n.isGraphNode()?"g":"c")).arg(curr_id);
	}
public:
	NarratorNodeVisitor(int deapest_rank)
	{
		this->highest_rank=deapest_rank;
	}
	virtual void initialize()
	{
		RanksList.clear();
		file=new QFile("graph.dot");
		file->remove();
		if (!file->open(QIODevice::ReadWrite))
		{
			out<<"Error openning file\n";
			return;
		}

		dot_out=new QTextStream(file);
		d_out.setCodec("utf-8");
		d_out<<"digraph hadith_graph {\n";
	}
	virtual void visit(NarratorNodeIfc & n1,NarratorNodeIfc & n2, int /*child_num*/)
	{
		QString s1=getID(n1), s2=getID(n2);
		//qDebug()<<n1.CanonicalName()<<"->"<<n2.CanonicalName();
		d_out<<s2<<"->"<<s1<<";\n";
	}
	virtual void visit(NarratorNodeIfc & n) //this is enough
	{
		getID(n);
	}
	virtual void finish()
	{
	#ifdef FORCE_RANKS
		QString s;
		if (RanksList.size()>0)
		{
			d_out<<QString("r%1 [label=\"%1\"];\n").arg(0);
			d_out<<"{ rank = sink;";
			foreach(s,RanksList[0])
				d_out<<s<<";";
			d_out<<QString("r%1;").arg(0);
			d_out<<"}\n";
		}

		for (int rank=1;rank<RanksList.size()-1;rank++)
		{
			if (RanksList[rank].size()>0)
			{
				d_out<<QString("r%1 [label=\"%1\"];\n").arg(rank);
				d_out<<QString("r%1 -> r%2 [style=invis];\n").arg(rank).arg(rank-1);
				d_out<<"{ rank = same;";
				foreach(s,RanksList[rank])
					d_out<<s<<";";
				d_out<<QString("r%1;").arg(rank);
				d_out<<"}\n";
			}
		}
		if (RanksList.size()>2)
		{
			int rank=RanksList.size()-1;
			d_out<<QString("r%1 [label=\"%1\"];\n").arg(rank);
			d_out<<QString("r%1 -> r%2 [style=invis];\n").arg(rank).arg(rank-1);
			d_out<<"{ rank = source;";
			foreach(s,RanksList[rank])
				d_out<<s<<";";
			d_out<<QString("r%1;").arg(rank);
			d_out<<"}\n";
		}
	#endif
		d_out<<"}\n";
		delete dot_out;
		file->close();
		delete file;
	}
};


class RankCorrectorNodeVisitor:	public NodeVisitor
{
private:
	int highest_rank;
public:
	void initialize()
	{
		highest_rank=0;
	}
	int getHighestRank()
	{
		return highest_rank;
	}
	virtual void visit(NarratorNodeIfc & n1,NarratorNodeIfc & n2, int /*child_num*/)
	{
		int rank1=n1.getGraphRank(), rank2=n2.getGraphRank();
		if (rank1>=rank2)
			rank2=rank1+1;
		n2.setSavedRank(rank2);
		n1.setSavedRank(rank1);
		if (rank2>highest_rank)
			highest_rank=rank2;

	}
	virtual void visit(NarratorNodeIfc &) {	}
	virtual void finish(){	}
};


class LoopBreakingVisitor: public NodeVisitor
{
private:
	bool cycle_fixed;
	bool detected_cycle;
	NarratorGraph * graph;
	bool has_self_cycle(NarratorNodeIfc &  n)
	{
		int num=n.getNumChildren();
		for (int i=0;i<num;i++)
		{
			NarratorNodeIfcRfc r=n.getChild(i);
			NarratorNodeIfc & c=r.Rfc();
			if (&c==&n)
				return true;
		}
		return false;
	}
	bool has_dual_cycle(NarratorNodeIfc &  n1,NarratorNodeIfc &  n2) //assuming when this is called n1 is parent of n2
	{
		int num=n2.getNumChildren();
		for (int i=0;i<num;i++)
		{
			NarratorNodeIfcRfc r=n2.getChild(i);
			NarratorNodeIfc & c=r.Rfc();
			if (&c==&n1)
				return true;
		}
		return false;
	}
public:
	LoopBreakingVisitor(NarratorGraph * graph)
	{
		cycle_fixed=false;
		detected_cycle=false;
		this->graph=graph;
	}
	void initialize()	{	}
	virtual void visit(NarratorNodeIfc & ,NarratorNodeIfc & , int ) { 	}
	virtual void visit(NarratorNodeIfc &) {	}
	virtual void finish();
	virtual void cycle_detected()//any cycle of length larger than 2 detected
	{
		detected_cycle=true;
		controller->stop_searching_for_cycles=false;
	}
	virtual void cycle_detected(NarratorNodeIfc & n)//any cycle of length larger than 1 or 2 detected
	{
		static const double step=parameters.equality_delta/2;
		static const int num_steps=4;
		detected_cycle=true;
		controller->stop_searching_for_cycles=true;
		out<< "Error: Self cycle detected at node"<<n.CanonicalName()<<"!\n";
		if (!n.isGraphNode())
			return; //unable to resolve
		GraphNarratorNode * g=(GraphNarratorNode *)&n;
		QList<ChainNarratorNodeIterator *> narrators;
		for (int i=0;i<g->equalnarrators.size();i++)
		{
			narrators.append(&(g->equalnarrators[i]));
			(g->equalnarrators[i])->setCorrespondingNarratorNode(NULL);
		}

		double original_threshold=parameters.equality_threshold;
		for (int i=0;i<num_steps;i++)
		{
			parameters.equality_threshold-=step;
			for (int j=1;j<narrators.size();j++)
				if (equal(narrators[j-1]->getNarrator(),narrators[j]->getNarrator()))
					mergeNodes(*narrators[j-1],*narrators[j]);
			bool all_fixed=true;
			for (int j=0;j<narrators.size();j++)
			{
				NarratorNodeIfc & node=narrators[j]->getCorrespondingNarratorNode();
				if (node.isGraphNode() && has_self_cycle(n))
				{
					all_fixed=false;
					break;
				}
			}
			cycle_fixed=all_fixed;
		}
		//check how to free memory correctly
		//g->equalnarrators.clear();
		delete g;
		parameters.equality_threshold=original_threshold;
	}
	virtual void cycle_detected(NarratorNodeIfc & n1,NarratorNodeIfc & n2)//any cycle of length larger than 1 or 2 detected
	{
		detected_cycle=true;
		controller->stop_searching_for_cycles=true;
		out<< "Error: Cycle detected between nodes:"<<n1.CanonicalName()<<" and "<<n2.CanonicalName()<<"!\n";
	}
};

class NarratorGraph
{
private:
	NarratorNodesList top_g_nodes;
	QList<int> top_c_indices;
	int deapest_rank;//rank of the deapmost node;

	void computeRanks()
	{
		RankCorrectorNodeVisitor *r=new RankCorrectorNodeVisitor;
		GraphVisitorController c(r,true,false);
		BFS_traverse(c);
		deapest_rank=r->getHighestRank();
		delete r;
	}
	void breakManageableCycles()
	{
		LoopBreakingVisitor *l=new LoopBreakingVisitor(this);
		GraphVisitorController c(l,true,true);
		DFS_traverse(c,true);
		delete l;
	}
	void deduceTopNodes(ChainsContainer & chains);
	void DFS_traverse(NarratorNodeIfc & n,GraphVisitorController & visitor)
	{
		visitor.visit(n);
		int size=n.getNumChildren();
		//qDebug()<<"parent:"<<n.CanonicalName()<<" "<<size;
		for (int i=0;i<size;i++)
		{
			NarratorNodeIfcRfc r=n.getChild(i);
			NarratorNodeIfc & c=r.Rfc();
			//qDebug()<<"child:"<<(!c.isNull()?c.CanonicalName():"null");
			if (!c.isNull() && !visitor.isPreviouslyVisited(&n, &c,i))
			{
				if (visitor.detect_cycles && visitor.stop_searching_for_cycles )
					return;
				bool prev_visited=visitor.isPreviouslyVisited( &c);
				visitor.visit(n,c,i);
				if (!prev_visited)
					DFS_traverse(c,visitor);
				else
				{
					//visitor.stop_searching_for_cycles=true;//stops the traversal whenever a cycle is detected
					//detect cycles types
					int start=(visitor.merged_edges_as_one?1:0);
					int max=(visitor.merged_edges_as_one?2:n.getNumChildren());
					bool found_small_cycle=false;
					for (int j=start;j<max;j++)
					{
						if (visitor.isPreviouslyVisited(&c, &c,j))
						{
							visitor.visitor->cycle_detected(c);
							found_small_cycle=true;
							break;
						}
						if (visitor.isPreviouslyVisited(&c, &n,j))
						{
							visitor.visitor->cycle_detected(c,n);
							found_small_cycle=true;
							break;
						}
					}
					if (!found_small_cycle)
						visitor.visitor->cycle_detected();
					return;

				}
			}
		}
	}
public:
	NarratorGraph(ChainsContainer & chains)
	{
		deduceTopNodes(chains);
		//breakManageableCycles();
		computeRanks();
	}
	int getDeapestRank()
	{
		return deapest_rank;
	}

	void DFS_traverse(GraphVisitorController & visitor,bool detect_cycles=false);
	void BFS_traverse(GraphVisitorController & visitor);
};

void buildGraph(ChainsContainer &chs);
int test_NarratorEquality(QString input_str);//just for testing purposes

#endif // CHAIN_GRAPH_H

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
	//NarratorNodeIfc(){setSavedRank(-1);}
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
	virtual QString toString()=0;
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
	virtual QString toString() {return "NULLNarratorNodeIfc";}
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
	QString CanonicalName();
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
	QString getRanks(){ return QString("[%1](%2)").arg(getAutomaticRank()).arg(getSavedRank());}
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

class GraphNarratorNode: public NarratorNodeIfc
{
protected:
	QList<ChainNarratorNodeIterator>  equalnarrators;
	int savedRank;
	void setSavedRank(int rank){savedRank=rank;}
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
		QString ranks= "[";
		for (int i=0;i<equalnarrators.size();i++)
			ranks+=QString("%1,").arg(equalnarrators[i].getAutomaticRank());
		ranks+="]";
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

class NodeVisitor
{
public:
	virtual bool previouslyVisited( NarratorNodeIfc * node)=0;
	virtual bool previouslyVisited( NarratorNodeIfc * n1, NarratorNodeIfc * n2, int chain_num)=0;
	virtual void initialize()=0;
	virtual void visit(NarratorNodeIfc & n1,NarratorNodeIfc & n2, int chain_num)=0;
	virtual void finish()=0;
};

class NarratorNodeVisitor: public NodeVisitor
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

	int last_id;
	QFile * file;
	QTextStream * dot_out;
	#define d_out (*dot_out)
	QList<QStringList> RanksList;

	void setGraphRank(int rank, QString s)
	{
		while(rank>=RanksList.size())
			RanksList.append(QStringList());
		RanksList[rank].append(s);
	}
	QString getID(NarratorNodeIfc & n)
	{
		int curr_id;
		if (n.isGraphNode())
		{
			G_IDMap::iterator it=GraphNodesID.find((GraphNarratorNode*)&n);
			if (it==GraphNodesID.end())
			{
				curr_id=++last_id;
				GraphNodesID.insert((GraphNarratorNode*)&n,curr_id);
				d_out<<QString("g")<<curr_id<<" [label=\""<<n.CanonicalName().replace('\n',"");
			#ifdef SHOW_RANKS
				d_out<<n.getRanks();
			#endif
				d_out<<"\", shape=box];\n";
				setGraphRank(n.getGraphRank(),QString("g%1").arg(curr_id));
			}
			else
				curr_id=it.value();
			return QString("g%1").arg(curr_id);
		}
		else
		{
			C_IDMap::iterator it=ChainNodesID.find(&*(ChainNarratorNodeIterator&)n);
			if (it==ChainNodesID.end())
			{
				curr_id=++last_id;
				ChainNodesID.insert(&*(ChainNarratorNodeIterator&)n,curr_id);
				d_out<<QString("c")<<curr_id<<" [label=\""<<n.CanonicalName().replace('\n',"");
			#ifdef SHOW_RANKS
				d_out<<n.getRanks();
			#endif
				d_out<<"\"]"<<";\n";
				setGraphRank(n.getGraphRank(),QString("c%1").arg(curr_id));
			}
			else
				curr_id=it.value();
			return QString("c%1").arg(curr_id);
		}

	}
public:
	virtual bool previouslyVisited( NarratorNodeIfc * g_node)//only for graph nodes
	{
		if (!g_node->isGraphNode())
			return false; //No info
		G_IDMap::iterator it=GraphNodesID.find((GraphNarratorNode*)g_node);
		return !(it==GraphNodesID.end());
	}
	virtual bool previouslyVisited( NarratorNodeIfc * n1, NarratorNodeIfc * n2,int child_num)
	{
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
	virtual void initialize()
	{
		RanksList.clear();
		cgEdgeMap.clear();
		gcEdgeMap.clear();
		ggEdgeMap.clear();
		ccEdgeMap.clear();
		file=new QFile("graph.dot");
		file->remove();
		if (!file->open(QIODevice::ReadWrite))
		{
			out<<"Error openning file\n";
			return;
		}

		dot_out=new QTextStream(file);
		d_out.setCodec("utf-8");
		last_id=0;
		d_out<<"digraph hadith_graph {\n";
	}
	virtual void visit(NarratorNodeIfc & n1,NarratorNodeIfc & n2, int chain_num)
	{
		QString s1=getID(n1), s2=getID(n2);
		//qDebug()<<n1.CanonicalName()<<"-->"<<n2.CanonicalName();
		d_out<<s2<<"->"<<s1<<";\n";
		if (n1.isGraphNode())
		{
			if (!n2.isGraphNode())
				gcEdgeMap.insert(CGEdge(&*((ChainNarratorNodeIterator &)n2),&(GraphNarratorNode &)n1,chain_num),true);
			else
				ggEdgeMap.insert(GGEdge(&(GraphNarratorNode &)n1,&(GraphNarratorNode &)n2,chain_num),true);
		}
		else
		{
			if (n2.isGraphNode())
				cgEdgeMap.insert(CGEdge(&*((ChainNarratorNodeIterator &)n1),&(GraphNarratorNode &)n2,chain_num),true);
			else
				ccEdgeMap.insert(CCEdge(&*((ChainNarratorNodeIterator &)n1),&*((ChainNarratorNodeIterator &)n2),chain_num),true);
		}
	}
	virtual void finish()
	{
	#ifdef FORCE_RANKS
		QString s;
		d_out<<"{ rank = sink;";
		if (RanksList.size()>0)
		{
			foreach(s,RanksList[0])
				d_out<<s<<";";
		}
		d_out<<"}\n";

		for (int rank=1;rank<RanksList.size()-1;rank++)
		{
			if (RanksList[rank].size()>1)
			{
				d_out<<"{ rank = same;";
				foreach(s,RanksList[rank])
					d_out<<s<<";";
				d_out<<"}\n";
			}
		}
		if (RanksList.size()>2)
		{
			d_out<<"{ rank = source;";
			foreach(s,RanksList[RanksList.size()-1])
			{
				d_out<<s<<";";
			}
			d_out<<"}\n";
		}
	#endif
		d_out<<"}\n";
		delete dot_out;
		file->close();
		delete file;
	}
};

#define SKIPNODES

class RankCorrectorNodeVisitor:
#ifdef SKIPNODES
		public NarratorNodeVisitor
#else
		public NodeVisitor
#endif
{
public:

	virtual bool previouslyVisited( NarratorNodeIfc *)//get thru all path even if visited from another path
	{
		return false;
	}
#ifndef SKIPNODES
	virtual bool previouslyVisited( NarratorNodeIfc * , NarratorNodeIfc * ,int)//get thru all path even if visited from another path
	{
		return false;
	}
	virtual void initialize(){	}
#endif
	virtual void visit(NarratorNodeIfc & n1,NarratorNodeIfc & n2, int chain_num)
	{
		int rank1=n1.getGraphRank(), rank2=n2.getGraphRank();
		if (rank1>=rank2)
			n2.setSavedRank(rank1+1);
		else
			n2.setSavedRank(rank2);
		n1.setSavedRank(rank1);
	#ifdef SKIPNODES
		if (n1.isGraphNode())
		{
			if (!n2.isGraphNode())
				gcEdgeMap.insert(CGEdge(&*((ChainNarratorNodeIterator &)n2),&(GraphNarratorNode &)n1,chain_num),true);
			else
				ggEdgeMap.insert(GGEdge(&(GraphNarratorNode &)n1,&(GraphNarratorNode &)n2,chain_num),true);
		}
		else
		{
			if (n2.isGraphNode())
				cgEdgeMap.insert(CGEdge(&*((ChainNarratorNodeIterator &)n1),&(GraphNarratorNode &)n2,chain_num),true);
			else
				ccEdgeMap.insert(CCEdge(&*((ChainNarratorNodeIterator &)n1),&*((ChainNarratorNodeIterator &)n2),chain_num),true);
		}
	#endif
	}
#ifndef SKIPNODES
	virtual void finish(){	}
#endif
};


class NarratorGraph
{
private:
	NarratorNodesList top_g_nodes;
	QList<int> top_c_indices;

	void computeRanks()
	{
		RankCorrectorNodeVisitor r;
		traverse(r);
	}
	void deduceTopNodes(ChainsContainer & chains);
	void traverse(NarratorNodeIfc & n,NodeVisitor & visitor)
	{
		int size=n.getNumChildren();
		//qDebug()<<"parent:"<<n.CanonicalName()<<" "<<size;
		for (int i=0;i<size;i++)
		{
			NarratorNodeIfcRfc r=n.getChild(i);
			NarratorNodeIfc & c=r.Rfc();
			//qDebug()<<"child:"<<(!c.isNull()?c.CanonicalName():"null");
			if (!c.isNull() && !visitor.previouslyVisited(&n, &c,1))//i
			{
				bool prev_visited=visitor.previouslyVisited( &c);
				visitor.visit(n,c,1);//i
				if (!prev_visited)
					traverse(c,visitor);
			}
		}
	}
public:
	NarratorGraph(ChainsContainer & chains)
	{
		deduceTopNodes(chains);
		computeRanks();
	}
	void traverse(NodeVisitor & visitor);
};

void buildGraph(ChainsContainer &chs);
int test_NarratorEquality(QString input_str);//just for testing purposes

#endif // CHAIN_GRAPH_H

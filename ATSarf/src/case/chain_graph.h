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


class NarratorNodeIfc;
class ChainNarratorNode;
class ChainNarratorNodeIterator;
class GraphNarratorNode;
class NULLGraphNarratorNode;
class NULLNarratorNodeIfc;
class NULLChainNarratorNodeIterator;

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

	virtual ChainNarratorNodeIterator & getChainNodeItrInChain(int chain_num)=0;
	virtual QString toString()=0;
};

class NULLNarratorNodeIfc: public NarratorNodeIfc
{
	NarratorNodeIfc & firstChild();
	NarratorNodeIfc & nextChild(NarratorNodeIfc & current);
	NarratorNodeIfc & firstParent();
	NarratorNodeIfc & nextParent(NarratorNodeIfc & current);

	int getNumChildren();
	NarratorNodeIfcRfc getChild(int index);

	ChainNarratorNodeIterator & firstNarrator();
	ChainNarratorNodeIterator & nextNarrator(ChainNarratorNodeIterator & current);

	NodeAddress prevInChain(ChainNarratorNodeIterator &);
	NodeAddress nextInChain(ChainNarratorNodeIterator &);

	QString CanonicalName(){return QString::null;}

	virtual bool isNull(){return true;}
	virtual ChainNarratorNodeIterator & getChainNodeItrInChain(int chain_num);
	virtual bool isGraphNode() {return false;}
	virtual QString toString() {return "NULLNarratorNodeIfc";}
};

class ChainNarratorNode
{
protected:
	GraphNarratorNode * narrNode;
public:
	ChainNarratorNode()
	{
		narrNode=NULL;
	}
	GraphNarratorNode & getCorrespondingNarratorNode();
	void  setCorrespondingNarratorNode(GraphNarratorNode * narrNode)
	{
		this->narrNode=narrNode;
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
	NarratorNodeIfcRfc getChild(int index)
	{
		assert(index==0);
		return NarratorNodeIfcRfc(nextInChain(),true);
	}

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
	virtual bool isNull()
	{
		return true;
	}
	virtual QString toString()
	{
		return "NULLChainNarratorNodeIterator";
	}
};

class GraphNarratorNode: public NarratorNodeIfc
{
private:
	QList<ChainNarratorNodeIterator>  equalnarrators;
public:
	GraphNarratorNode(){}
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
	QString CanonicalName()
	{
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

class NarratorNodeVisitor
{
private:
	typedef QMap<GraphNarratorNode*,int> G_IDMap;
	typedef QMap<ChainNarratorNode*,int> C_IDMap;
	typedef QPair<ChainNarratorNode *,GraphNarratorNode*> CGEdge;
	typedef QPair<GraphNarratorNode *,GraphNarratorNode*> GGEdge;
	typedef QMap<CGEdge, bool> CGEdgeMap; //(chain, graph)
	typedef QMap<GGEdge, bool> GGEdgeMap; //(graph, graph)
	G_IDMap GraphNodesID;
	C_IDMap ChainNodesID;
	CGEdgeMap cgEdgeMap;
	GGEdgeMap ggEdgeMap;

	int last_id;
	QFile * file;
	QTextStream * dot_out;
	#define d_out (*dot_out)

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
				d_out<<QString("g")<<curr_id<<" [label=\""<<n.CanonicalName().replace('\n',"")<<"\"];\n"; //"\", shape=box];\n";
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
				d_out<<QString("c")<<curr_id<<" [label=\""<<n.CanonicalName().replace('\n',"")<<"\"]"<<";\n";
			}
			else
				curr_id=it.value();
			return QString("c%1").arg(curr_id);
		}

	}
public:
	bool previouslyVisited( NarratorNodeIfc * g_node)//only for graph nodes
	{
		if (!g_node->isGraphNode())
			return false; //No info
		G_IDMap::iterator it=GraphNodesID.find((GraphNarratorNode*)g_node);
		return !(it==GraphNodesID.end());
	}
	bool previouslyVisited( NarratorNodeIfc * n1, NarratorNodeIfc * n2)
	{
		if (!n2->isGraphNode() )
			return false; //No info
		if (!n1->isGraphNode())
		{
			CGEdgeMap::iterator it=cgEdgeMap.find(CGEdge(&**((ChainNarratorNodeIterator*)n1),(GraphNarratorNode*)n2));
			return !(it==cgEdgeMap.end());
		}
		else
		{
			GGEdgeMap::iterator it=ggEdgeMap.find(GGEdge((GraphNarratorNode*)n1,(GraphNarratorNode*)n2));
			return !(it==ggEdgeMap.end());
		}
	}
	virtual void initialize()
	{
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
	virtual void visit(NarratorNodeIfc & n1,NarratorNodeIfc & n2)
	{
		QString s1=getID(n1), s2=getID(n2);
		//qDebug()<<n1.CanonicalName()<<"-->"<<n2.CanonicalName();
		d_out<<s2<<"->"<<s1<<";\n";
		if (n2.isGraphNode())
		{
			if (!n1.isGraphNode())
				cgEdgeMap.insert(CGEdge(&*((ChainNarratorNodeIterator &)n1),&(GraphNarratorNode &)n2),true);
			else
				ggEdgeMap.insert(GGEdge(&(GraphNarratorNode &)n1,&(GraphNarratorNode &)n2),true);
		}
	}
	virtual void finish()
	{
		d_out<<"}\n";
		delete dot_out;
		file->close();
		delete file;
	}
};

class NarratorGraph
{
private:
	NarratorNodesList top_g_nodes;
	QList<int> top_c_indices;

	void deduceTopNodes(ChainsContainer & chains);
	void traverse(NarratorNodeIfc & n,NarratorNodeVisitor & visitor)
	{
		int size=n.getNumChildren();
		//qDebug()<<"parent:"<<n.CanonicalName();
		for (int i=0;i<size;i++)
		{
			NarratorNodeIfcRfc r=n.getChild(i);
			NarratorNodeIfc & c=r.Rfc();
			//qDebug()<<"child:"<<(!c.isNull()?c.CanonicalName():"null");
			if (!c.isNull() && !visitor.previouslyVisited(&n, &c))
			{
				visitor.visit(n,c);
				traverse(c,visitor);
			}
		}
	}
public:
	NarratorGraph(ChainsContainer & chains)
	{
		deduceTopNodes(chains);
	}
	void traverse(NarratorNodeVisitor & visitor);
};

void buildGraph(ChainsContainer &chs);
int test_NarratorEquality(QString input_str);//just for testing purposes

#endif // CHAIN_GRAPH_H

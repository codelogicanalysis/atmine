#ifndef GRAPH_H
#define GRAPH_H

#include "graph_nodes.h"
#include <QStack>

typedef QList<NarratorNodeIfc *> NarratorNodesList;

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

	virtual void detectedCycle(NarratorNodeIfc & n)=0;
	virtual void finishVisit(NarratorNodeIfc & n)=0;
};

class NarratorGraph;
class LoopBreakingVisitor;

class ColorIndices
{
private:
	NarratorGraph * graph;
	unsigned int usedBits;
	unsigned int nextUnused;

	unsigned int maxBits(){ return sizeof(int)<<3;}
	void setGraph(NarratorGraph * graph){this->graph=graph;}

	friend class NarratorGraph;
public:
	ColorIndices()
	{
		usedBits=0;
		nextUnused=0;
	}
	void use(unsigned int bit)
	{
		assert (bit<maxBits());
		usedBits |= 1 << bit;
		if (nextUnused==bit)
		{
			unsigned int max=maxBits();
			for (unsigned int i=bit+1;i<max;i++)
			{
				if (!isUsed(i))
				{
					nextUnused=i;
					return;
				}
			}
			nextUnused=maxBits();//unacceptable value for usage
		}
	}
	void unUse(unsigned int bit);//unuse and clear color bit for all nodes in graph
	bool isUsed(unsigned int bit)
	{
		assert (bit<maxBits());
		return (usedBits & (1 << bit)) != 0;
	}
	unsigned int getNextUnused()
	{
		return nextUnused;
	}
};

class GraphVisitorController
{
public:
	typedef QStack< NarratorNodeIfc*> ParentStack;
protected:
	typedef Triplet<NarratorNodeIfc *,NarratorNodeIfc*,int> Edge;
	typedef QMap<Edge, bool> EdgeMap;
	EdgeMap edgeMap;
	ParentStack parentStack;
	unsigned int visitIndex, finishIndex ;

	NarratorGraph * graph;
	bool keep_track_of_edges, keep_track_of_nodes,merged_edges_as_one, detect_cycles;
	NodeVisitor * visitor;
	bool stop_searching_for_cycles;

	friend class NarratorGraph;

	void init()
	{
		if (keep_track_of_edges)
			edgeMap.clear();
		parentStack.clear();
	}
	void construct(NodeVisitor * visitor,NarratorGraph * graph,bool keep_track_of_edges,bool keep_track_of_nodes, bool merged_edges_as_one)
	{
		this->visitor=visitor;
		assert(visitor!=NULL);
		this->visitor->controller=this;
		this->keep_track_of_edges=keep_track_of_edges || merged_edges_as_one;//if merged_edges_as_one is needed we must keep_track_of_edges
		this->keep_track_of_nodes=keep_track_of_nodes;
		this->merged_edges_as_one=merged_edges_as_one;
		this->graph=graph;
		detect_cycles=false;
		stop_searching_for_cycles=false;
		init();
	}

public:

	GraphVisitorController(NodeVisitor * visitor,NarratorGraph * graph,unsigned int visitIndex,unsigned int finishIndex,bool keep_track_of_edges=true, bool merged_edges_as_one=true); //assumes keep_track_of_nodes by default
	GraphVisitorController(NodeVisitor * visitor,NarratorGraph * graph,bool keep_track_of_edges=true,bool keep_track_of_nodes=true, bool merged_edges_as_one=true);
	bool isPreviouslyVisited( NarratorNodeIfc & node)
	{
		if (!keep_track_of_nodes)
			return false;
		bool visited= node.isVisited(visitIndex);
		if (visited)
		{
			if (!isFinished(node))
				visitor->detectedCycle(node);
		}
		return visited;
	}
	bool isPreviouslyVisited( NarratorNodeIfc & n1, NarratorNodeIfc & n2,int child_num)
	{
		if (!keep_track_of_edges)
			return false;
		assert(&n2==&(n1.getChild(child_num))); //to check if the edge exists i.e. we can go from n1 to n2
		if (merged_edges_as_one)
		{
			child_num=0; //so that they will all be treated equally
			EdgeMap::iterator it=edgeMap.find(Edge(&n1,&n2,child_num));
			return !(it==edgeMap.end());
		}
		else
			return n1[child_num].isVisited(visitIndex);
	}
	void initialize();
	void visit(NarratorNodeIfc & n1,NarratorNodeIfc & n2, int child_num)
	{
		visitor->visit(n1,n2,child_num);
		if (!keep_track_of_edges)
			return;
		if (merged_edges_as_one)
		{
			child_num=0; //so that they will all be treated equally
			edgeMap.insert(Edge(&n1,&n2,child_num),true);
		}
		else
			n1[child_num].setVisited(visitIndex);
	}
	void visit(NarratorNodeIfc & n)
	{
		if (keep_track_of_nodes)
			n.resetVisited(finishIndex);
		parentStack.push(&n);
		visitor->visit(n);

		if (!keep_track_of_nodes)
			return ;
		n.setVisited(visitIndex);
	}
	bool isFinished( NarratorNodeIfc & node)
	{
		if (!keep_track_of_nodes)
			return false;
		return node.isVisited(finishIndex);
	}
	void finishVisit(NarratorNodeIfc & n)
	{
		if (keep_track_of_nodes)
			n.setVisited(finishIndex);
		n.setVisited(finishIndex);
		visitor->finishVisit(n);
		parentStack.pop();
	}
	void finish();
	unsigned int getVisitColorIndex(){return visitIndex;}
	unsigned int getFinishColorIndex(){return finishIndex;}
	const ParentStack & getParentStack() {return parentStack;}
	NarratorGraph * getGraph(){return graph;}
};

class DisplayNodeVisitor: public NodeVisitor
{
protected:
	QFile * file;
	QTextStream * dot_out;
	#define d_out (*dot_out)
	QList<QStringList> ranksList;

	typedef QMap<NarratorNodeIfc*,int> IDMap;
	IDMap nodeMap;
	int last_id;
	int getUniqueNodeID(NarratorNodeIfc & n)//if not there returns -1
	{
		IDMap::iterator it=nodeMap.find(&n);
		if (it==nodeMap.end())
			return -1;
		else
			return it.value();
	}
	int generateUniqueNodeID(NarratorNodeIfc & n) //if not there generates one and returns it
	{
		int curr_id;
		IDMap::iterator it=nodeMap.find(&n);
		if (it==nodeMap.end())
		{
			curr_id=++last_id;
			nodeMap.insert(&n,curr_id);
		}
		else
			curr_id=it.value();
		return curr_id;
	}
	void setGraphRank(int rank, QString s)
	{
		while(rank>=ranksList.size())
			ranksList.append(QStringList());
		ranksList[rank].append(s);
	}
	void displayChainNumsEndingJustAfter(NarratorNodeIfc & n, QString name)
	{
		if (parameters.display_chain_num )
		{
			//qDebug()<<name;
			for (int i=0;i<n.size();i++)
			{
				ChainNarratorNode & c=n[i];
				if (c.isLast())
				{
					int num=c.getChainNum();
					QString ch_node=QString("ch%1").arg(num+1);
					d_out<<ch_node<<" [label=\""<<num+1<<"\", shape=triangle];\n";
					d_out<<name<<"->"<<ch_node<<";\n";
					setGraphRank(n.getRank()+2,ch_node);
				}
			}
		}
	}
	QString getAndInitializeDotNode(NarratorNodeIfc & n)
	{
		int curr_id=getUniqueNodeID(n);
		QString name;
		if (curr_id==-1)
		{
			curr_id=generateUniqueNodeID(n);
			if (n.isGraphNode())
			{
				d_out<<QString("g")<<curr_id<<" [label=\""<<n.CanonicalName().replace('\n',"");
			#ifdef SHOW_RANKS
				d_out<<n.rank2String();
			#endif
				d_out<<"\", shape=box];\n";
				name=QString("g%1").arg(curr_id);
			}
			else
			{
				d_out<<QString("c")<<curr_id<<" [label=\""<<n.CanonicalName().replace('\n',"");
			#ifdef SHOW_RANKS
				d_out<<n.rank2String();
			#endif
				d_out<<"\"]"<<";\n";
				name=QString("c%1").arg(curr_id);
			}
			setGraphRank(n.getRank()+1,name);
			return name;
		}
		else
			return QString("%1%2").arg((n.isGraphNode()?"g":"c")).arg(curr_id);
	}
public:
	DisplayNodeVisitor(){	}
	virtual void initialize()
	{
		nodeMap.clear();
		last_id=0;
		ranksList.clear();
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
		QString s1=getAndInitializeDotNode(n1), s2=getAndInitializeDotNode(n2);
		d_out<<s1<<"->"<<s2<<";\n";
	}
	virtual void visit(NarratorNodeIfc & n) //this is enough
	{
		QString s=getAndInitializeDotNode(n);
		displayChainNumsEndingJustAfter(n,s);
	#ifdef DISPLAY_GRAPHNODES_CONTENT
		if (n.isGraphNode())
			out<<n.toString()<<"\n";
	#endif
	}
	virtual void finishVisit(NarratorNodeIfc & ){}
	virtual void detectedCycle(NarratorNodeIfc & n)
	{
		NarratorNodeIfc * current=&n;
		out<<"cycle at ";
		out<<"[";
		QString s=n.CanonicalName();
		out<<s<<",";
		const GraphVisitorController::ParentStack & stack=controller->getParentStack();
		int size=stack.size();
		for (int i=size-1; i>=0; i--)
		{
			current=stack[i];
			if (current==&n && i!=size-1)
				break;
			s=current->CanonicalName();
			out<<s<<",";
		}
		out<<"]\n";
	#if 0
		do
		{
			if (controller->parentStack.isEmpty())
				break;
			controller->parentStack.pop();
			QString s=current->CanonicalName();
			out<<s<<",";
			//qDebug()<<s<<",";
			if (controller->parentStack.isEmpty())
				break;
			current=controller->parentStack.top();
		}while(current!=&n && !current->isNull());
		out<<"]\n";
	#endif
	}
	virtual void finish()
	{
	#ifdef FORCE_RANKS
		QString s;
		int startingRank=(parameters.display_chain_num?0:1);
		int currRank=startingRank,lastRank=startingRank;
		if (ranksList.size()>0)
		{
			while (ranksList[currRank].size()==0)
				currRank++;
			d_out<<QString("r%1 [label=\"%1\"];\n").arg(lastRank);
			d_out<<"{ rank = source;";
			foreach(s,ranksList[currRank])
				d_out<<s<<";";
			d_out<<QString("r%1;").arg(lastRank);
			d_out<<"}\n";
			lastRank++;
		}

		for (int rank=currRank+1;rank<ranksList.size()-1;rank++)
		{
			if (ranksList[rank].size()>0)
			{
				d_out<<QString("r%1 [label=\"%1\"];\n").arg(lastRank);
				d_out<<QString("r%2 -> r%1 [style=invis];\n").arg(lastRank).arg(lastRank-1);
				d_out<<"{ rank = same;";
				foreach(s,ranksList[rank])
					d_out<<s<<";";
				d_out<<QString("r%1;").arg(lastRank);
				d_out<<"}\n";
				lastRank++;
			}
		}

		int rank=ranksList.size()-1;
		if (rank>startingRank)
		{
			d_out<<QString("r%1 [label=\"%1\"];\n").arg(lastRank);
			d_out<<QString("r%2 -> r%1 [style=invis];\n").arg(lastRank).arg(lastRank-1);
			d_out<<"{ rank = sink;";
			foreach(s,ranksList[rank])
				d_out<<s<<";";
			d_out<<QString("r%1;").arg(lastRank);
			d_out<<"}\n";
		}
	#endif
		d_out<<"}\n";
		delete dot_out;
		file->close();
		delete file;
	}

};

class FillNodesVisitor: public NodeVisitor
{
private:
	NarratorNodesList * list_all, * list_bottom;
public:
	FillNodesVisitor(NarratorNodesList * list_all,NarratorNodesList * list_bottom){this->list_all=list_all; this->list_bottom=list_bottom;}
	void initialize(){ list_all->clear(); list_bottom->clear();}
	NarratorNodesList * getFilledList(){return list_all;}
	virtual void visit(NarratorNodeIfc & ,NarratorNodeIfc &, int ){	}
	virtual void visit(NarratorNodeIfc & n) {
		list_all->append(&n);
		/*if (n.size()==0) //TODO: do a function that checks if it is a leaf node (needs a for loop in the case of a graph node)
			list_bottom->append(&n);*/
	}
	virtual void finishVisit(NarratorNodeIfc & ){}
	virtual void detectedCycle(NarratorNodeIfc & ){}
	virtual void finish(){}
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
		int rank1=n1.getRank(), rank2=n2.getRank();
		if (rank1>=rank2)
			rank2=rank1+1;
		n2.setRank(rank2);
		n1.setRank(rank1);
		if (rank2>highest_rank)
			highest_rank=rank2;

	}
	virtual void visit(NarratorNodeIfc &) {	}
	virtual void finishVisit(NarratorNodeIfc & ){ }
	virtual void detectedCycle(NarratorNodeIfc & ){ }
	virtual void finish(){	}
};
class LoopBreakingVisitor: public NodeVisitor
{
private:
	double threshold;
	void swapThresholds() //swap threshold and parameters.equality_threshold
	{
		double temp=parameters.equality_threshold;
		parameters.equality_threshold=threshold;
		threshold=temp;
	}
	void reMergeNodes(NarratorNodeIfc & n);
public:
	LoopBreakingVisitor(double equality_threshold) {threshold=equality_threshold; }
	virtual void initialize() { /*swapThresholds();*/ }
	virtual void visit(NarratorNodeIfc & ,NarratorNodeIfc & , int ) { 	}
	virtual void visit(NarratorNodeIfc &) {	}
	virtual void finishVisit(NarratorNodeIfc & ){}
	virtual void detectedCycle(NarratorNodeIfc & n)
	{
		NarratorNodeIfc * current=&n;
		reMergeNodes(*current);
		const GraphVisitorController::ParentStack & stack=controller->getParentStack();
		int size=stack.size();
		for (int i=size-1; i>=0; i--)
		{
			current=stack[i];
			if (current==&n && i!=size-1)
				break;
			reMergeNodes(*current);
		}
	}
	virtual void finish() { /*swapThresholds();*/ }
};
class NarratorGraph
{
private:
	ColorIndices colorGuard;
	NarratorNodesList	top_nodes,
						bottom_nodes, all_nodes;
	friend class ColorIndices;

	ATMProgressIFC *prg;

	int highest_rank;//rank of the deapmost node;
	int getDeapestRank(){return highest_rank;}

	GraphNarratorNode & mergeNodes(ChainNarratorNode & n1,ChainNarratorNode & n2)
	{
		NarratorNodeIfc & narr1=n1.getCorrespondingNarratorNode(),
						& narr2=n2.getCorrespondingNarratorNode();
		if (!narr1.isGraphNode() && !narr2.isGraphNode())
		{
			GraphNarratorNode * g=new GraphNarratorNode(n1,n2);
		#if 0
			out<<g->toString()<<"\n";
		#endif
			return *g;
		}
		else if (narr1.isGraphNode())
		{
			((GraphNarratorNode &)narr1).addNarrator(n2);
		#if 0
			out<<narr1.toString()<<"\n";
		#endif
			return (GraphNarratorNode &)narr1;
		}
		else if (narr2.isGraphNode())
		{
			((GraphNarratorNode &)narr2).addNarrator(n1);
		#if 0
			out<<narr2.toString()<<"\n";
		#endif
			return (GraphNarratorNode &)narr2;
		}
		else if (&narr1!=&narr2)
		{
			GraphNarratorNode & g_node=(GraphNarratorNode &)narr2;
			GraphNarratorNode * dlt_g_node= &(GraphNarratorNode &)narr1;
			for (int i=0;i<n1.size();i++)
			{
				ChainNarratorNode & c_node=n1[i];
				g_node.addNarrator(c_node);
			}
		#if 0
			out<<g_node.toString()<<"\n";
		#endif
			delete dlt_g_node;
			return g_node;
		}
		else
			return (GraphNarratorNode &)narr1;
	}
	friend class LoopBreakingVisitor;
	friend class GraphVisitorController;

	void transform2ChainNodes(ChainsContainer &chains)
	{//pre-condition: chain contains the valid chains extracted from the hadith
		prg->setCurrentAction("Creating Nodes");
		int num_chains=chains.count();
		for (int chain_num=0;chain_num<num_chains;chain_num++)
		{
			int size=chains.at(chain_num)->m_chain.count();
			ChainNarratorNode* last=NULL;
			for (int j=0;j<size;j++)
			{
				if (chains.at(chain_num)->m_chain[j]->isNarrator())
				{
					Narrator *n=(Narrator *)(chains.at(chain_num)->m_chain[j]);
					ChainNarratorNode* current= new ChainNarratorNode(n,0,chain_num);
					if (last !=NULL)
					{
						last->previous=current;
						current->next=last;
					}
					last=current;
				}
			}
			top_nodes.append(last);
			//code below to correct index
			int index=0;
			for (ChainNarratorNode * current=last;!current->isNull();current=&current->nextInChain())
			{
				current->setIndex(index);
				index++;
			}
			prg->report(100.0*chain_num/num_chains+0.5);
		}
	//postcondition: Narrator's are transformed to ChainNarratorNode's and linked into
	//				 chains and top_nodes stores the link to the first node of each chain
	}
	void buildGraph()
	{ //note: compares chain by chain then moves to another
		//TODO: check how to remove duplication when it occurs in the contents of a graph node, if not taken care when inserting
		int radius=parameters.equality_radius;
		double threshold=parameters.equality_threshold;
		int num_chains=top_nodes.size();
		prg->setCurrentAction("Merging Nodes");
		prg->report(0);
		for (int i=0;i<num_chains;i++)
		{
			ChainNarratorNode & c1= *(ChainNarratorNode *)top_nodes[i]; //since at this stage all are ChainNode's
			for (int k=i+1;k<num_chains;k++)
			{
				ChainNarratorNode & c2= *(ChainNarratorNode *)top_nodes[k];
				int needle=0;
				ChainNarratorNode * n1=&c1; //start from beginning of c1
				int u=0,offset=-1;
				for (;!n1->isNull();n1=&(n1->nextInChain()))
				{
					NarratorNodeIfc & g_node=n1->getCorrespondingNarratorNode();//get the graph node corresponding to n1, or n1 if no such exists
					ChainNarratorNode & lastMergedNode //the node we should start comparing from after it
							=g_node.getChainNodeInChain(k);//return the chain k node in the graph node if it exists

					if (!lastMergedNode.isNull())
						offset=max(offset, //previous offset which may be also increased since 2 were found equal
								   lastMergedNode.getIndex());//index of lastMergedNode, which we must skip
					u=max(offset+1,needle-radius); //the iterator over nodes of chain[k] for comparison
					u=u>0?u:0;
					bool match=false;

					int increment=u-offset; //increment that we should add to start at u
					ChainNarratorNode * n2=(lastMergedNode.isNull()
											? &(c2+u) //start iteration at u (from beginning +u)
											:&(lastMergedNode+increment)); //start iteration at u ( lastNode+(u-index(lastNode)) )
					for(;u<needle+radius && !n2->isNull();u++,n2=&(n2->nextInChain())) //we are within bound of radius and of the chain size (if null => finished)
					{
						Narrator & n1_ref=n1->getNarrator();
						Narrator & n2_ref=n2->getNarrator();
						double eq_val=equal(n1_ref,n2_ref);
						if (eq_val>=threshold)
						{
						#ifdef DEBUG_BUILDGRAPH
							int index2=n2->getIndex();
							assert(index2<20);
							qDebug()<<n1_ref.getString()<<"["<<i<<","<<n1->getIndex()<<"]Versus["<<n2_ref.getString()<<"["<<k<<","<<index2<<"]\n";
						#endif
							mergeNodes(*n1,*n2);
							offset=u;	//this is matched, we must skip it in search for match for next node in c1
							needle=u+1; //since the node is matched, we move to match the next
							match=true;
							break;
						}
					}
					if (!match)
						needle++;
				}
			}
			prg->report(100.0*i/num_chains);
		}
		prg->report(100);
	}
	void computeRanks()
	{
	#if 0
		for (int trials=0;trials<2;trials++)
		{
			for (int i=0;i<top_nodes.size();i++)
			{
				for (int j=0;j<top_nodes[i]->size();j++)
				{
					ChainNarratorNode * last=&(*top_nodes[i])[j];
					for (ChainNarratorNode * current=last;!current->isNull();current=&current->nextInChain())
					{
						int rank1=last->getCorrespondingNarratorNode().getAutomaticRank(),
							rank2=current->getCorrespondingNarratorNode().getAutomaticRank();
						if (rank1>=rank2)
							rank2=rank1+1;
						current->setRank(rank2);
						//current->getCorrespondingNarratorNode().setRank(rank2);
						//last->getCorrespondingNarratorNode().setRank(rank1);
						last->setRank(rank1);
						if (rank2>highest_rank)
							highest_rank=rank2;
						last=current;
					}
				}
			}
		}
	#else
		RankCorrectorNodeVisitor r;
		GraphVisitorController c(&r,this);
		prg->setCurrentAction("Computing Ranks");
		prg->report(0);
		BFS_traverse(c);
		prg->report(50);
		BFS_traverse(c);
		prg->report(100);
		highest_rank=r.getHighestRank();
	#endif
	}
	void breakManageableCycles()
	{
		const double step=parameters.equality_delta;
		const int num_steps=3;
		prg->setCurrentAction("Breaking Cycles");
		prg->report(0);
		for (int i=1;i<=num_steps;i++)
		{
		#ifdef DISPLAY_NODES_BEING_BROKEN
			qDebug()<<"--";
		#endif
			double threshold=parameters.equality_threshold+i*step;
			LoopBreakingVisitor l(threshold);
			GraphVisitorController c(&l,this);
			DFS_traverse(c);
			prg->report(i/num_steps*100+0.5);
		}

	}
	void correctTopNodesList()
	{
		prg->setCurrentAction("Correct TopList");
		prg->report(0);
		NarratorNodesList & new_top_list=*(new NarratorNodesList);
		int size=top_nodes.size();
		for (int i=0;i<size;i++)
		{
			ChainNarratorNode & c=(ChainNarratorNode &)*top_nodes[i];
			NarratorNodeIfc * g=&c.getCorrespondingNarratorNode();
			assert(!g->isNull());//it can only be null if c was not a ChainNarratorNode, anyways, does not hurt to check
			if (g->isGraphNode())
			{
				if (!new_top_list.contains(g)) //might have been already added
					new_top_list.append(g);
			}
			else
				new_top_list.append(g);
			prg->report(100.0*i/size+0.5);
		}
		top_nodes=new_top_list; //TODO: try reduce copy cost
		/*&delete &top_nodes; //we do this to reduce a copy operation
		NarratorNodesList *& temp=&top_nodes;
		temp=&new_top_list;*/
	}
	void fillNodesLists()
	{
		prg->setCurrentAction("Correct NodeList");
		prg->report(0);
		FillNodesVisitor visitor(&all_nodes, &bottom_nodes);
		GraphVisitorController c(&visitor,this);
		DFS_traverse(c);
		prg->report(100);
	}
	void DFS_traverse(NarratorNodeIfc & n,GraphVisitorController & visitor)
	{
		visitor.visit(n);
		//int size=n.size();
	#ifdef DEBUG_DFS_TRAVERSAL
		out<<"parent:"<<n.toString()<<" "<<size<<"\n";
	#endif
		for (int i=0;i<n.size();i++)//n.size() instead of saved variable bc in case LoopBreakingVisitor does change thru the traversal
		{
			NarratorNodeIfc & c=n.getChild(i);
		#ifdef DEBUG_DFS_TRAVERSAL
			out<<n.CanonicalName()<<".child("<<i<<"):"<<(!c.isNull()?c.CanonicalName():"null")<<"\n";
		#endif
			if (!c.isNull() && !visitor.isPreviouslyVisited(n, c,i))
			{
				bool prev_visited=visitor.isPreviouslyVisited(c);
				visitor.visit(n,c,i);
				if (!prev_visited)
					DFS_traverse(c,visitor);
			}
		}
		visitor.finishVisit(n);
	}
public:
	NarratorGraph(ChainsContainer & chains, ATMProgressIFC *prg)
	{
		this->prg=prg;
		colorGuard.setGraph(this);
		transform2ChainNodes(chains);
		buildGraph();
		correctTopNodesList();
		fillNodesLists();
		if (parameters.break_cycles)
			breakManageableCycles();
		computeRanks();
	}
	void DFS_traverse(GraphVisitorController & visitor)
	{
		visitor.initialize();
		int size=top_nodes.size();
		for (int i=0; i<size;i++)
		{
			NarratorNodeIfc * node=top_nodes[i];
			if (!visitor.isPreviouslyVisited(*node))
				DFS_traverse(*node,visitor);
		}
		visitor.finish();
	}
	void BFS_traverse(GraphVisitorController & visitor)
	{
		visitor.initialize();
		QQueue<NarratorNodeIfc *> queue;
		int size=top_nodes.size();
		for (int i=0; i<size;i++)
		{
			NarratorNodeIfc * node=top_nodes[i];
			queue.enqueue(node);
		}
		while (!queue.isEmpty())
		{
			NarratorNodeIfc & n=*(queue.dequeue());
			visitor.visit(n);
			int size=n.size();
		#ifdef DEBUG_BFS_TRAVERSAL
			out<<"parent:"<<n.toString()<<" "<<size<<"\n";
		#endif
			for (int i=0;i<size;i++)
			{
				NarratorNodeIfc & c=n.getChild(i);
			#ifdef DEBUG_BFS_TRAVERSAL
				out<<n.CanonicalName()<<".child("<<i<<"):"<<(!c.isNull()?c.CanonicalName():"null")<<"\n";
			#endif
				if (!c.isNull() && !visitor.isPreviouslyVisited(n, c,i))
				{
					bool prev_visited=visitor.isPreviouslyVisited( c);
					visitor.visit(n,c,i);
					if (!prev_visited)
						queue.enqueue(&c);
				}
			}
			visitor.finishVisit(n);
		}
		visitor.finish();
	}
};

inline int test_GraphFunctionalities(ChainsContainer &chains, ATMProgressIFC *prg)
{
	NarratorGraph graph(chains,prg);
	prg->setCurrentAction("Display Graph");
	prg->report(0);
	DisplayNodeVisitor visitor;
	GraphVisitorController c(&visitor,&graph);
	graph.DFS_traverse(c);
	prg->setCurrentAction("Completed");
	prg->report(100);
	return 0;
}

#endif // GRAPH_H

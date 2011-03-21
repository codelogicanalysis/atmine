#ifndef GRAPH_H
#define GRAPH_H

#include "graph_nodes.h"

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

	virtual void detectedCycle(NarratorNodeIfc & n);
	virtual void finishVisit(NarratorNodeIfc & ) {}
};

class NarratorGraph;
class LoopBreakingVisitor;

class ColorIndices
{
private:
	static ColorIndices * instance;

	unsigned int usedBits;
	unsigned int nextUnused;
	ColorIndices()
	{
		usedBits=0;
		nextUnused=0;
	}
	unsigned int maxBits(){ return sizeof(int)<<3;}

public:
	static ColorIndices & getInstance()
	{
		if (instance==NULL)
			instance=new ColorIndices();
		return *instance;
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
	void unUse(unsigned int bit)
	{
		assert (bit<maxBits());
		usedBits &= (~(1 << bit));
		if (nextUnused>bit)
			nextUnused=bit;
	}
	void unUse(unsigned int bit, NarratorGraph * graph);//unuse and clear color bit for all nodes in graph
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
protected:
	typedef Triplet<NarratorNodeIfc *,NarratorNodeIfc*,int> Edge;
	typedef QMap<Edge, bool> EdgeMap;
	typedef QMap<NarratorNodeIfc *, NarratorNodeIfc*> ParentMap;
	EdgeMap edgeMap;
	ParentMap parentMap;
	unsigned int visitIndex, finishIndex ;

	NarratorGraph * graph;
	bool keep_track_of_edges, keep_track_of_nodes,merged_edges_as_one, detect_cycles;
	NodeVisitor * visitor;
	bool stop_searching_for_cycles;

	friend class NarratorGraph;
	friend class LoopBreakingVisitor;

	void init()
	{
		if (keep_track_of_edges)
			edgeMap.clear();
		parentMap.clear();
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

	GraphVisitorController(NodeVisitor * visitor,NarratorGraph * graph,unsigned int visitIndex,unsigned int finishIndex,bool keep_track_of_edges=true, bool merged_edges_as_one=true) //assumes keep_track_of_nodes by default
	{
		construct(visitor,graph,keep_track_of_edges,true,merged_edges_as_one);
		this->visitIndex=visitIndex; //assumed already cleared
		ColorIndices::getInstance().use(visitIndex);
		this->finishIndex=finishIndex; //assumed already cleared
		ColorIndices::getInstance().use(finishIndex);
	}
	GraphVisitorController(NodeVisitor * visitor,NarratorGraph * graph,bool keep_track_of_edges=true,bool keep_track_of_nodes=true, bool merged_edges_as_one=true)
	{
		construct(visitor,graph,keep_track_of_edges,keep_track_of_nodes,merged_edges_as_one);
		if (keep_track_of_nodes)
		{
			this->visitIndex=ColorIndices::getInstance().getNextUnused();
			ColorIndices::getInstance().use(visitIndex);
			this->finishIndex=ColorIndices::getInstance().getNextUnused();
			ColorIndices::getInstance().use(finishIndex);
		}
	}
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
	void initialize()
	{
		init();
		ColorIndices::getInstance().use(visitIndex);
		visitor->initialize();
	}
	NarratorNodeIfc & getParent(NarratorNodeIfc & n)
	{
		ParentMap::iterator it=parentMap.find(&n);
		if (it!=parentMap.end())
			return *(*it);
		else
			return nullNarratorNodeIfc;
	}
	void visit(NarratorNodeIfc & n1,NarratorNodeIfc & n2, int child_num)
	{
		//if (!isPreviouslyVisited(n1,n2,child_num))
		//{
			parentMap[&n2]=&n1;
			visitor->visit(n1,n2,child_num);
		//}

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
		//if (!isPreviouslyVisited(n))
		//{
			if (keep_track_of_nodes)
				n.resetVisited(finishIndex);
			visitor->visit(n);
		//}

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
	}
	void finish()
	{
		ColorIndices::getInstance().unUse(visitIndex,graph);
		ColorIndices::getInstance().unUse(finishIndex,graph);
		visitor->finish();
	}
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
		if (parameters.display_chain_num)
		{
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
	#ifdef DISPLAY_GRAPHNODES_CONTENT
		if (n.isGraphNode())
			out<<n.toString()<<"\n";
	#endif
	}
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
		int num=n.size();
		for (int i=0;i<num;i++)
		{
			NarratorNodeIfc & c=n.getChild(i);
			if (&c==&n)
				return true;
		}
		return false;
	}
	bool has_dual_cycle(NarratorNodeIfc &  n1,NarratorNodeIfc &  n2) //assuming when this is called n1 is parent of n2
	{
		int num=n2.size();
		for (int i=0;i<num;i++)
		{
			NarratorNodeIfc & c=n2.getChild(i);
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
	virtual void cycle_detected(NarratorNodeIfc & n);//any cycle of length larger than 1 or 2 detected
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

	void transform2ChainNodes(ChainsContainer &chains)
	{//pre-condition: chain contains the valid chains extracted from the hadith
		for (int chain_num=0;chain_num<chains.count();chain_num++)
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
		}
	//postcondition: Narrator's are transformed to ChainNarratorNode's and linked into
	//				 chains and top_nodes stores the link to the first node of each chain
	}
	void buildGraph()
	{ //note: compares chain by chain then moves to another
		int radius=parameters.equality_radius;
		double threshold=parameters.equality_threshold;
		int num_chains=top_nodes.size();
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
					#ifdef DEBUG_BUILDGRAPH
						out<<n1_ref.getString()<<"]Versus["<<n2_ref.getString()<<"\n";
					#endif
						double eq_val=equal(n1_ref,n2_ref);
						if (eq_val>=threshold)
						{
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
		BFS_traverse(c);
		BFS_traverse(c);
		highest_rank=r.getHighestRank();
	#endif
	}
	void breakManageableCycles() //not re-looked at
	{
	#if 0
		LoopBreakingVisitor l(this);
		GraphVisitorController c(&l,this,true,true);
		DFS_traverse(c,true);
	#endif
	}
	void correctTopNodesList()
	{
		NarratorNodesList & new_top_list=*(new NarratorNodesList);
		for (int i=0;i<top_nodes.size();i++)
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
		}
		top_nodes=new_top_list; //TODO: try reduce copy cost
		/*&delete &top_nodes; //we do this to reduce a copy operation
		NarratorNodesList *& temp=&top_nodes;
		temp=&new_top_list;*/
	}
	void fillNodesLists()
	{
		FillNodesVisitor visitor(&all_nodes, &bottom_nodes);
		GraphVisitorController c(&visitor,this);
		DFS_traverse(c);
	}
	void DFS_traverse(NarratorNodeIfc & n,GraphVisitorController & visitor)
	{
		visitor.visit(n);
		int size=n.size();
	#ifdef DEBUG_DFS_TRAVERSAL
		out<<"parent:"<<n.toString()<<" "<<size<<"\n";
	#endif
		for (int i=0;i<size;i++)
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
		transform2ChainNodes(chains);
		buildGraph();
		correctTopNodesList();
		fillNodesLists();
		//breakManageableCycles();
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
	DisplayNodeVisitor visitor;
	GraphVisitorController c(&visitor,&graph);
	graph.DFS_traverse(c);
	return 0;
}

#endif // GRAPH_H
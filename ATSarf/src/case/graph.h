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

	virtual void cycle_detected(){}//any cycle of length larger than 1 or 2 detected
	virtual void cycle_detected(NarratorNodeIfc &){}//any cycle of length larger than 1 or 2 detected
	virtual void cycle_detected(NarratorNodeIfc & ,NarratorNodeIfc &){}//any cycle of length larger than 1 or 2 detected
};

class NarratorGraph;
class LoopBreakingVisitor;

class GraphVisitorController
{
protected:
	typedef QMap<NarratorNodeIfc*,int> IDMap;
	typedef Triplet<NarratorNodeIfc *,NarratorNodeIfc*,int> Edge;
	typedef QMap<Edge, bool> EdgeMap;
	IDMap nodeMap;
	EdgeMap edgeMap;

	bool keep_track_of_edges, keep_track_of_nodes,merged_edges_as_one, detect_cycles;
	int last_id;

	bool stop_searching_for_cycles;

	NodeVisitor * visitor;
	void init()
	{
		edgeMap.clear();
		nodeMap.clear();
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
	bool isPreviouslyVisited( NarratorNodeIfc & node)
	{
		if (!keep_track_of_nodes)
			return false;
		IDMap::iterator it=nodeMap.find(&node);
		return !(it==nodeMap.end());
	}
	bool isPreviouslyVisited( NarratorNodeIfc & n1, NarratorNodeIfc & n2,int child_num)
	{
		if (!keep_track_of_edges)
			return false;
		if (merged_edges_as_one)
			child_num=1; //so that they will all be treated equally
		EdgeMap::iterator it=edgeMap.find(Edge(&n1,&n2,child_num));
		return !(it==edgeMap.end());
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
		if (!isPreviouslyVisited(n1,n2,child_num))
			visitor->visit(n1,n2,child_num);

		if (!keep_track_of_edges)
			return;
		edgeMap.insert(Edge(&n1,&n2,child_num),true);
	}
	void visit(NarratorNodeIfc & n)
	{
		if (!isPreviouslyVisited(n))
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

class DisplayNodeVisitor: public NodeVisitor
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
			if (n.size()==0)
				setGraphRank(highest_rank,name);
			else
				setGraphRank(n.getRank(),name);
			return name;
		}
		else
			return QString("%1%2").arg((n.isGraphNode()?"g":"c")).arg(curr_id);
	}
public:
	DisplayNodeVisitor(int deapest_rank)
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
						bottom_nodes, all_nodes; //not used yet, TODO: a seperate function to fill them
	int deapest_rank;//rank of the deapmost node;

	GraphNarratorNode & mergeNodes(ChainNarratorNode & n1,ChainNarratorNode & n2)
	{
		NarratorNodeIfc & narr1=n1.getCorrespondingNarratorNode(),
						& narr2=n2.getCorrespondingNarratorNode();
		if (!narr1.isGraphNode() && !narr2.isGraphNode())
		{
			GraphNarratorNode * g=new GraphNarratorNode(n1,n2);
			out<<g->toString()<<"\n";
			return *g;
		}
		else if (narr1.isGraphNode())
		{
			((GraphNarratorNode &)narr1).addNarrator(n2);
			out<<narr1.toString()<<"\n";
			return (GraphNarratorNode &)narr1;
		}
		else if (narr2.isGraphNode())
		{
			((GraphNarratorNode &)narr2).addNarrator(n1);
			out<<narr2.toString()<<"\n";
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
			out<<g_node.toString()<<"\n";
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
			int size=chains.at(chain_num)->m_chain.count(),index=0;
			ChainNarratorNode* last;
			for (int j=0;j<size;j++)
			{
				if (chains.at(chain_num)->m_chain[j]->isNarrator())
				{
					//qDebug()<<chains.at(i)->m_chain[j]->getString()<<"\n";
					Narrator *n=(Narrator *)(chains.at(chain_num)->m_chain[j]);
					ChainNarratorNode* current= new ChainNarratorNode(n,index,chain_num);
					if (index!=0)
					{
						last->next=current;
						current->previous=last;
					}
					else if (index==0)
					{
						top_nodes.append(current);
					}
					last=current;
					//n->getRank().printRank();
					index++;
				}
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

					offset=(lastMergedNode.isNull()
									?-1
									:lastMergedNode.getIndex());//index of lastMergedNode, which we must skip
					u=max(offset+1,needle-radius); //the iterator over nodes of chain[k] for comparison
					u=u>0?u:0;
					bool match=false;

					int increment=u-offset; //increment that we should add to start at u
					ChainNarratorNode * n2=(offset==-1
											? &(c2+u) //start iteration at u (from beginning +u)
											:&(lastMergedNode+increment)); //start iteration at u ( lastNode+(u-index(lastNode)) )
					for(;u<needle+radius && !n2->isNull();u++,n2=&(n2->nextInChain())) //we are within bound of radius and of the chain size (if null => finished)
					{
						Narrator & n1_ref=n1->getNarrator();
						Narrator & n2_ref=n2->getNarrator();
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
		}
	}
	void computeRanks()
	{
		//TODO: if cycles exist find all strongly connected components and treat them as 1 node
		//		and that results in an acyclic graph then use it when visiting nodes
		//  OR  create a visitor that notices cycles and acts accordingly not to get into an infinite loop
		//		but still a correct ranking scheme.
		RankCorrectorNodeVisitor *r=new RankCorrectorNodeVisitor;
		GraphVisitorController c(r,true,false);
		BFS_traverse(c);
		deapest_rank=r->getHighestRank();
		delete r;
	}
	void breakManageableCycles() //not re-looked at
	{
		LoopBreakingVisitor *l=new LoopBreakingVisitor(this);
		GraphVisitorController c(l,true,true);
		DFS_traverse(c,true);
		delete l;
	}
	void correctTopNodesList();
	void DFS_traverse(NarratorNodeIfc & n,GraphVisitorController & visitor)
	{
		visitor.visit(n);
		int size=n.size();
		//qDebug()<<"parent:"<<n.CanonicalName()<<" "<<size;
		for (int i=0;i<size;i++)
		{
			NarratorNodeIfc & c=n.getChild(i);
			//qDebug()<<"child:"<<(!c.isNull()?c.CanonicalName():"null");
			if (!c.isNull() && !visitor.isPreviouslyVisited(n, c,i))
			{
				if (visitor.detect_cycles && visitor.stop_searching_for_cycles )
					return;
				bool prev_visited=visitor.isPreviouslyVisited( c);
				visitor.visit(n,c,i);
				if (!prev_visited)
					DFS_traverse(c,visitor);
				else
				{
					//visitor.stop_searching_for_cycles=true;//stops the traversal whenever a cycle is detected
					//detect cycles types
					int start=(visitor.merged_edges_as_one?1:0);
					int max=(visitor.merged_edges_as_one?2:n.size());
					bool found_small_cycle=false;
					for (int j=start;j<max;j++)
					{
						if (visitor.isPreviouslyVisited(c, c,j))
						{
							visitor.visitor->cycle_detected(c);
							found_small_cycle=true;
							break;
						}
						if (visitor.isPreviouslyVisited(c, n,j))
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
		transform2ChainNodes(chains);
		buildGraph();
		correctTopNodesList();
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

int test_GraphFunctionalities(ChainsContainer &chs);

#endif // GRAPH_H

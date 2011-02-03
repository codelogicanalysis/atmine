#include <QQueue>
#include "graph.h"

void LoopBreakingVisitor::finish() //not re-looked at
{
	if (cycle_fixed)
	{
		cycle_fixed=false;
		detected_cycle=false;
		controller->initialize();
		graph->DFS_traverse(*(this->controller),true);//retry breaking more loops
	}
	if (detected_cycle && ! cycle_fixed)
		out<< "Error: Cycles detected, which were not corrected!\n";
}

void LoopBreakingVisitor::cycle_detected(NarratorNodeIfc & n)//any cycle of length larger than 1 or 2 detected
{
	static const double step=parameters.equality_delta/2;
	static const int num_steps=4;
	detected_cycle=true;
	controller->stop_searching_for_cycles=true;
	out<< "Error: Self cycle detected at node"<<n.CanonicalName()<<"!\n";
	if (!n.isGraphNode())
		return; //unable to resolve
	GraphNarratorNode * g=(GraphNarratorNode *)&n;
	QList<ChainNarratorNode *> narrators;
	for (int i=0;i<g->equalChainNodes.size();i++)
	{
		narrators.append((g->equalChainNodes[i]));
		(g->equalChainNodes[i])->setCorrespondingNarratorNode(NULL);
	}

	double original_threshold=parameters.equality_threshold;
	for (int i=0;i<num_steps;i++)
	{
		parameters.equality_threshold-=step;
		for (int j=1;j<narrators.size();j++)
			if (equal(narrators[j-1]->getNarrator(),narrators[j]->getNarrator()))
				graph->mergeNodes(*narrators[j-1],*narrators[j]);
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

void NarratorGraph::correctTopNodesList()
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

void NarratorGraph::DFS_traverse(GraphVisitorController & visitor, bool detect_cycles)
{
	visitor.initialize();
	if (detect_cycles)
	{
		visitor.detect_cycles=true;//forces VisitorController to detect cycles
		visitor.keep_track_of_nodes=true;
	}
	int size=top_nodes.size();
	for (int i=0; i<size;i++)
	{
		NarratorNodeIfc * node=top_nodes[i];
		if (!(detect_cycles && visitor.stop_searching_for_cycles ))
		{
			DFS_traverse(*node,visitor);
			if (detect_cycles)
				visitor.initialize();
		}
	}
	visitor.finish();
}

void NarratorGraph::BFS_traverse(GraphVisitorController & visitor)
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
		//qDebug()<<"parent:"<<n.CanonicalName()<<" "<<size;
		for (int i=0;i<size;i++)
		{
			NarratorNodeIfc & c=n.getChild(i);
			//qDebug()<<"child:"<<(!c.isNull()?c.CanonicalName():"null");
			if (!c.isNull() && !visitor.isPreviouslyVisited(n, c,i))
			{
				bool prev_visited=visitor.isPreviouslyVisited( c);
				visitor.visit(n,c,i);
				if (!prev_visited)
					queue.enqueue(&c);
			}
		}
	}
	visitor.finish();
}

int test_GraphFunctionalities(ChainsContainer & chains)
{
	NarratorGraph graph(chains);
	DisplayNodeVisitor *visitor=new DisplayNodeVisitor(graph.getDeapestRank());
	GraphVisitorController c(visitor);
	graph.DFS_traverse(c);
	delete visitor;
	return 0;
}

#include <QQueue>
#include "graph.h"

void ColorIndices::unUse(unsigned int bit)//unuse and clear color bit for all nodes in graph
{
	assert (bit<maxBits());
	usedBits &= (~(1 << bit));

	int max=graph->all_nodes.size();
	for (int i=0;i<max;i++)
	{
		graph->all_nodes[i]->resetVisited(bit);
		int size=graph->all_nodes[i]->size();
		for (int j=0;j<size;j++)
			(*graph->all_nodes[i])[j].resetVisited(bit);
	}

	if (nextUnused>bit)
		nextUnused=bit;
}

GraphVisitorController::GraphVisitorController(NodeVisitor * visitor,NarratorGraph * graph,unsigned int visitIndex,unsigned int finishIndex,bool keep_track_of_edges, bool merged_edges_as_one) //assumes keep_track_of_nodes by default
{
	construct(visitor,graph,keep_track_of_edges,true,merged_edges_as_one);
	this->visitIndex=visitIndex; //assumed already cleared
	graph->colorGuard.use(visitIndex);
	this->finishIndex=finishIndex; //assumed already cleared
	graph->colorGuard.use(finishIndex);
}
GraphVisitorController::GraphVisitorController(NodeVisitor * visitor,NarratorGraph * graph,bool keep_track_of_edges,bool keep_track_of_nodes, bool merged_edges_as_one)
{
	construct(visitor,graph,keep_track_of_edges,keep_track_of_nodes,merged_edges_as_one);
	if (keep_track_of_nodes)
	{
		this->visitIndex=graph->colorGuard.getNextUnused();
		graph->colorGuard.use(visitIndex);
		this->finishIndex=graph->colorGuard.getNextUnused();
		graph->colorGuard.use(finishIndex);
	}
}
void GraphVisitorController::initialize()
{
	init();
	graph->colorGuard.use(visitIndex);
	visitor->initialize();
}
void GraphVisitorController::finish()
{
	graph->colorGuard.unUse(visitIndex);
	graph->colorGuard.unUse(finishIndex);
	visitor->finish();
}

void NodeVisitor::detectedCycle(NarratorNodeIfc & n)
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

void DisplayNodeVisitor::detectedCycle(NarratorNodeIfc & n)
{
	//TODO: add code to draw cycle in a seperate file whenever detected
}

#if 0
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

//not re-looked at
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
#endif

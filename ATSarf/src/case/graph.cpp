#include <QQueue>
#include "graph.h"

ColorIndices * ColorIndices::instance=NULL;
void ColorIndices::unUse(unsigned int bit, NarratorGraph * graph)//unuse and clear color bit for all nodes in graph
{
	unUse(bit);
	int max=graph->all_nodes.size();
	for (int i=0;i<max;i++)
	{
		graph->all_nodes[i]->resetVisited(bit);
		int size=graph->all_nodes[i]->size();
		for (int j=0;j<size;j++)
			(*graph->all_nodes[i])[j].resetVisited(bit);
	}
}

void NodeVisitor::detectedCycle(NarratorNodeIfc & n)
{
	NarratorNodeIfc * current=&n;
	out<<"cycle at ";
	out<<"[";
	do
	{
		QString s=current->CanonicalName();
		out<<s<<",";
		//qDebug()<<s<<",";
		current=&(controller->getParent(*current));
	}while(current!=&n && !current->isNull());
	out<<"]\n";
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

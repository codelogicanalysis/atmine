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

void LoopBreakingVisitor::reMergeNodes(NarratorNodeIfc & n)
{
	if (!n.isGraphNode())
		return; //unable to resolve
	GraphNarratorNode & g=*(GraphNarratorNode *)&n;
#ifdef DISPLAY_NODES_BEING_BROKEN
	qDebug()<<g.CanonicalName();
#endif
	QList<ChainNarratorNode *> narrators;
	for (int i=0;i<g.size();i++)
	{
		narrators.append(&(g[i]));
		g[i].setCorrespondingNarratorNode(NULL);
	}
	g.equalChainNodes.clear();
	NarratorGraph* graph=controller->getGraph();
	QList<NarratorNodeIfc *> new_nodes;
	int size=narrators.size();
	for (int j=0;j<size;j++)
	{
		for (int k=j+1;k<size;k++)
		{
			double eq_val=equal(narrators[j]->getNarrator(),narrators[k]->getNarrator());
			if (eq_val>threshold)
			{
				NarratorNodeIfc & n_new=graph->mergeNodes(*narrators[j],*narrators[k]);
				if (!new_nodes.contains(&n_new))
					new_nodes.append(&n_new);
			}
		}
	}
	//to keep all_nodes consistent
	graph->all_nodes.removeOne(&g);
	graph->all_nodes.append(new_nodes);
	if (graph->top_nodes.contains(&g))
	{
		graph->top_nodes.removeOne(&g);
		graph->top_nodes.append(new_nodes);
	}
	for (int i=0;i<new_nodes.size();i++)
		new_nodes[i]->color=g.color;
	//delete &g;
}

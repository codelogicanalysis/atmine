#include <QQueue>
#include "graph.h"

void ColorIndices::unUse(unsigned int bit)//unuse and clear color bit for all nodes in graph
{
	assert (bit<maxBits());
	usedBits &= (~(1 << bit));

	int max=graph->all_nodes.size();
	for (int i=0;i<max;i++) {
		NarratorNodeIfc *n = graph->all_nodes[i];
		if (n!=NULL) {
			n->resetVisited(bit);
		}
	}

	if (nextUnused>bit)
		nextUnused=bit;
}
void ColorIndices::setAllNodesVisited(unsigned int bit) {
	assert (bit<maxBits());
	int max=graph->all_nodes.size();
	for (int i=0;i<max;i++) {
		NarratorNodeIfc *n = graph->all_nodes[i];
		if (n!=NULL) {
			n->setVisited(bit);
		}
	}
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
		visitIndex=graph->colorGuard.getNextUnused();
		graph->colorGuard.use(visitIndex);
		finishIndex=graph->colorGuard.getNextUnused();
		graph->colorGuard.use(finishIndex);
	}
}
void GraphVisitorController::init()
{
	if (firstCall) {
		firstCall=false;
	} else {
		graph->colorGuard.unUse(visitIndex); //to clear color bits
		graph->colorGuard.unUse(finishIndex);
		graph->colorGuard.use(visitIndex);
		graph->colorGuard.use(finishIndex);
	}
	if (keep_track_of_edges)
		edgeMap.clear();
	parentStack.clear();
}
void GraphVisitorController::initialize()
{
	init();
	visitor->initialize();
}
void GraphVisitorController::finish()
{
	graph->colorGuard.unUse(visitIndex);
	graph->colorGuard.unUse(finishIndex);
	visitor->finish();
}
GraphNarratorNode * LoopBreakingVisitor::mergeNodes(GroupNode & g1,GroupNode & g2) {
	NarratorNodeIfc * narr1=&g1.getCorrespondingNarratorNode(),
					* narr2=&g2.getCorrespondingNarratorNode();
	bool null1=(narr1==NULL),
		 null2=(narr2==NULL);
	if (null1 && null2) {
		GraphNarratorNode * g=new GraphNarratorNode(*controller->getGraph(),g1,g2);
		return g;
	} else if (!null1 && null2) {
		GraphNarratorNode * g=(GraphNarratorNode *)narr1;
		g->groupList.append(&g2);
		g2.setGraphNode(g);
		return (GraphNarratorNode *)narr1;
	} else if (null1 && !null2) {
		GraphNarratorNode * g=(GraphNarratorNode *)narr2;
		g->groupList.append(&g1);
		g1.setGraphNode(g);
		return (GraphNarratorNode *)narr2;
	} else if (narr1!=narr2) {
		GraphNarratorNode & g_node=*(GraphNarratorNode *)narr2;
		GraphNarratorNode * dlt_g_node= (GraphNarratorNode *)narr1;
		for (int i=0;i<dlt_g_node->size();i++) {
			g_node.groupList.append(dlt_g_node->groupList[i]);
			dlt_g_node->groupList[i]->setGraphNode(&g_node);
		}
		dlt_g_node->groupList.clear();
		if (!toDelete.contains(dlt_g_node))
			toDelete.append(dlt_g_node);
		return &g_node;
	} else
		return (GraphNarratorNode *)narr1;
}

void LoopBreakingVisitor::reMergeNodes(NarratorNodeIfc * n)
{
	if (n==NULL)
		return;
	if (!n->isGraphNode())
		return; //unable to resolve
	assert(!n->isChainNode());
	GraphNarratorNode * g=(GraphNarratorNode *)n;
#ifdef DISPLAY_NODES_BEING_BROKEN
	qDebug()<<g->CanonicalName();
#endif
	QList<GroupNode *> narrators;
	int size=g->size();
	for (int i=0;i<size;i++) {
		GroupNode & c=(*g)[i];
		narrators.append(&c);
		c.setGraphNode(NULL);
	}
	g->groupList.clear();
	NarratorGraph* graph=controller->getGraph();
	QList<NarratorNodeIfc *> new_nodes;
	QList<GroupNode *> unmatchedGroups;
	size=narrators.size();
	for (int j=0;j<size;j++) {
		unmatchedGroups.append(narrators[j]);
	}
	for (int j=0;j<size;j++) {
		assert(narrators[j]->size()>0);
		Narrator & n1=(*narrators[j])[0].getNarrator();
		for (int k=j+1;k<size;k++) {
			assert(narrators[k]->size()>0);
			Narrator & n2=(*narrators[k])[0].getNarrator();
			double eq_val=equal(n1,n2);
			if (eq_val>threshold) {
				NarratorNodeIfc * n1=&narrators[j]->getCorrespondingNarratorNode();
				NarratorNodeIfc * n2=&narrators[k]->getCorrespondingNarratorNode();
				NarratorNodeIfc * n_new=mergeNodes((*narrators[j]),(*narrators[k]));
				if (n1!=n_new && n1!=NULL && new_nodes.contains(n1))
					new_nodes.removeOne(n1);
				if (n2!=n_new && n2!=NULL && new_nodes.contains(n2))
					new_nodes.removeOne(n2);
				unmatchedGroups.removeOne(narrators[j]);
				unmatchedGroups.removeOne(narrators[k]);
				if (!new_nodes.contains(n_new))
					new_nodes.append(n_new);
			}
		}
	}
	for (int i=0;i<unmatchedGroups.size();i++) {
		if (unmatchedGroups[i]->size()>1) {
			GraphNarratorNode * g=new GraphNarratorNode(*controller->getGraph(),*unmatchedGroups[i]);
			new_nodes.append(g);
		} else {
			ChainNarratorNode * c=&(*unmatchedGroups[i])[0];
			c->setCorrespondingNarratorNodeGroup(NULL);
			new_nodes.append(c);
		}
	}

	//to keep all_nodes consistent
	if (graph->top_nodes.contains(g)) {
		graph->top_nodes.removeOne(g);
		//check if still top both
		graph->top_nodes.append(new_nodes);
	}
	for (int i=0;i<new_nodes.size();i++)
		new_nodes[i]->color=g->color;
	if (!toDelete.contains(g))
		toDelete.append(g);
}

void LoopBreakingVisitor::finish() {
	for (int i=0;i<toDelete.size();i++) {
		controller->getGraph()->removeNode(toDelete[i]);
		delete toDelete[i];
	}
	toDelete.clear();
}

int deserializeGraph(QString fileName,ATMProgressIFC * prg) {
	fileName=fileName.split("\n")[0];
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly))
		return -1;
	QDataStream fileStream(&file);
	NarratorGraph *graph=new NarratorGraph(fileStream,prg);
	file.close();
#if 0
	QFile file2("graph2.dat");
	if (!file2.open(QIODevice::ReadWrite))
		return -1;
	QDataStream fileStream2(&file2);
	graph->serialize(fileStream2);
	file2.close();
	delete graph;
#else
	biographies(graph);
#endif
	return 0;
}

int mergeGraphs(QString fileName1,QString fileName2,ATMProgressIFC * prg) {
	fileName1=fileName1.split("\n")[0];
	QFile file1(fileName1);
	if (!file1.open(QIODevice::ReadOnly))
		return -1;
	QDataStream fileStream1(&file1);
	NarratorGraph *graph1=new NarratorGraph(fileStream1,prg);
	file1.close();
	fileName2=fileName2.split("\n")[0];
	QFile file2(fileName2);
	if (!file2.open(QIODevice::ReadOnly))
		return -1;
	QDataStream fileStream2(&file2);
	NarratorGraph *graph2=new NarratorGraph(fileStream2,prg);
	file2.close();

	graph1->mergeWith(graph2);
	delete graph2;
#if 1
	QFile file("graphMerged.dat");
	if (!file.open(QIODevice::ReadWrite))
		return -1;
	QDataStream fileStream(&file);
	graph1->serialize(fileStream);
	file.close();
	delete graph1;
#else
	biographies(graph1);
#endif
	return 0;
}

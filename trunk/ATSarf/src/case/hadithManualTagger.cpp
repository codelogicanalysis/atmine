#include <QtGui>
#include "hadithManualTagger.h"
#include "hadithChainGraph.h"
#include "hadithDagGraph.h"


HadithTaggerDialog::HadithTaggerDialog(QString filename)
		:AbstractTwoLevelAnnotator(filename,"Hadith") {

	resultTreeMainMenu=new QMenu("Tree Menu", resultTree);
	resultTreeMainMenu->addAction("Merge",this,SLOT(mergeIndividual()));
	resultTreeMainMenu->addAction("Merge All",this,SLOT(mergeAll()));
	resultTreeMainMenu->addSeparator();
	resultTreeMainMenu->addAction("Un-Merge",this,SLOT(unMerge()));

	resultTreeOperationMenu=new QMenu("Tree Menu", resultTree);
	resultTreeOperationMenu->addAction("Apply Merge",this,SLOT(applyMerge()));
	resultTreeOperationMenu->addSeparator();
	resultTreeOperationMenu->addAction("Cancel Operation",this,SLOT(cancelMerge()));
	resultTree->setContextMenuPolicy(Qt::CustomContextMenu);


	connect(resultTree,SIGNAL(clicked(QModelIndex )),this,SLOT(resultTree_clicked(QModelIndex)));
	connect(resultTree,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resultTree_contextMenu(QPoint)));

	resultTreeMenuMIdxMain=resultTreeMenuMIdxApplied=NULL;
	chosenAction=false;
}

HadithTaggerDialog::~HadithTaggerDialog() {
	delete resultTreeMainMenu;
}

void HadithTaggerDialog::regenerateGlobalGraph() {
	if (globalGraph!=NULL)
		globalGraph->deleteGraph();
	if (tags.size()>0) {
		globalGraph=new HadithDagGraph();
		for (int i=0;i<tags.size();i++) {
			AbstractGraph * g=tags[i].getGraph();
			AbstractGraph * d=g->duplicate();
			globalGraph=globalGraph->merge(d);
		}
	} else
		globalGraph=NULL;
}

AbstractGraph * HadithTaggerDialog::newGraph(bool global) {
	if (global)
		return new HadithDagGraph();
	else
		return new HadithChainGraph();
}

void HadithTaggerDialog::resultTree_contextMenu(const QPoint & p) {
	if (globalGraphAct->isChecked()) {
		QModelIndex index  =resultTree->indexAt(p);
		resultTree_clicked(index);
		if (!chosenAction) {
			if (resultTreeMenuMIdxMain!=NULL)
				delete resultTreeMenuMIdxMain;
			resultTreeMenuMIdxMain=NULL;
		}
		if (resultTreeMenuMIdxMain==NULL) {
			resultTreeMenuMIdxMain=new QModelIndex(index);
			bool individual= (index.parent().isValid());
			resultTreeMainMenu->actions()[0]->setEnabled(individual);
			resultTreeMainMenu->actions()[3]->setEnabled(individual);
			resultTreeMainMenu->exec(QCursor::pos());
		} else {
			if (resultTreeMenuMIdxApplied!=NULL)
				delete resultTreeMenuMIdxApplied;
			resultTreeMenuMIdxApplied=new QModelIndex(index);
			NarratorNodeIfc * node1=(NarratorNodeIfc *)resultTreeMenuMIdxApplied->internalPointer();
			NarratorNodeIfc * node2=(NarratorNodeIfc *)resultTreeMenuMIdxMain->internalPointer();
			bool same=(&node1->getCorrespondingNarratorNode()==&node2->getCorrespondingNarratorNode());
			resultTreeOperationMenu->actions()[0]->setEnabled(!same);
			resultTreeOperationMenu->exec(QCursor::pos());
		}
	}
}

void HadithTaggerDialog::resultTree_clicked ( const QModelIndex & index ) {
	if (globalGraphAct->isChecked()) {
		QIODevice * d=out.device();
		QString s;
		out.setString(&s);
		DisplayLocalNodesVisitor::DetectedNodesMap map;
		NarratorNodeIfc * node=(NarratorNodeIfc *)index.internalPointer();
		HadithDagGraph * g=dynamic_cast<HadithDagGraph*>(globalGraph);
		map[&node->getCorrespondingNarratorNode()]=1;
		DisplayNodeVisitorColoredNarrator v(map);
		GraphVisitorController c(&v,g->graph,true,true);
		g->graph->DFS_traverse(c);
		if (globalGraph!=NULL) {
			try{
				system("dot -Tsvg graph.dot -o graph.svg");
				graph->setPixmap(QPixmap("./graph.svg"));
			} catch(...) {}
		} else
			graph->setPixmap(QPixmap());
		graph->repaint();
		out.setDevice(d);
	}
}

void HadithTaggerDialog::mergeIndividual() {
	isMergeAll=false;
	chosenAction=true;
}

void HadithTaggerDialog::mergeAll() {
	isMergeAll=true;
	chosenAction=true;
}
void HadithTaggerDialog::clearCacheFroUpdatedNode(NarratorNodeIfc * node) {
	HadithDagGraph *g=dynamic_cast<HadithDagGraph*>(globalGraph);
	g->graph->clearCachedChildrenParents();
	return;
	NarratorNodeIfc * currCorresponding =&node->getCorrespondingNarratorNode();
	if (currCorresponding->isGraphNode()) {
		GraphNarratorNode * g=dynamic_cast<GraphNarratorNode *>(currCorresponding);
		NodeIterator itr=g->parentsBegin();
		for (;!itr.isFinished();++itr) {
			NarratorNodeIfc * parent=itr.getNode();
			if (parent->isGraphNode()) {
				GraphNarratorNode * g=dynamic_cast<GraphNarratorNode *>(parent);
				g->children.clear();
			}
		}
		itr=g->childrenBegin();
		for (;!itr.isFinished();++itr) {
			NarratorNodeIfc * children=itr.getNode();
			if (children->isGraphNode()) {
				GraphNarratorNode * g=dynamic_cast<GraphNarratorNode *>(children);
				g->parents.clear();
			}
		}
		g->clearCache();
	}
}

void HadithTaggerDialog::unMerge() {
	NarratorNodeIfc* node=(NarratorNodeIfc*)resultTreeMenuMIdxMain->internalPointer();
	clearCacheFroUpdatedNode(node);
	assert(node->isChainNode());
	ChainNarratorNode* c_node=dynamic_cast<ChainNarratorNode *>(node);
	HadithDagGraph * g=dynamic_cast<HadithDagGraph *>(globalGraph);
	NarratorGraph * graph=g->graph;
	graph->unMerge(*c_node);
	updateGraphDisplay();
	chosenAction=false; //because applied in one step
}

void HadithTaggerDialog::applyMerge() {
	HadithDagGraph * g=dynamic_cast<HadithDagGraph *>(globalGraph);
	NarratorGraph * graph=g->graph;
	NarratorNodeIfc* node1=(NarratorNodeIfc*)resultTreeMenuMIdxMain->internalPointer();
	NarratorNodeIfc* node2=(NarratorNodeIfc*)resultTreeMenuMIdxApplied->internalPointer();
	if (!isMergeAll) {
		if (node1->isChainNode() && !node1->isActualNode()) {
			ChainNarratorNode * c_node=dynamic_cast<ChainNarratorNode *>(node1);
			graph->unMerge(*c_node);
		}
	}
	clearCacheFroUpdatedNode(node1);
	clearCacheFroUpdatedNode(node2);
	graph->mergeNodes(*node1,*node2);
	graph->correctTopNodesList();

	updateGraphDisplay();
	chosenAction=false; //finished from current one
}

void HadithTaggerDialog::cancelMerge() {
	assert(resultTreeMenuMIdxMain!=NULL);
	delete resultTreeMenuMIdxMain;
	assert(resultTreeMenuMIdxApplied!=NULL);
	delete resultTreeMenuMIdxApplied;
	resultTreeMenuMIdxMain=NULL;
	resultTreeMenuMIdxApplied=NULL;
	chosenAction=false;
}

int hadithTagger(QString input_str){
	HadithTaggerDialog * d=new HadithTaggerDialog(input_str);
	d->show();
	return 0;
}


#include <stdlib.h>
#include <time.h>
#include <QtGui>
#include "hadithManualTagger.h"
#include "hadithChainGraph.h"
#include "hadithDagGraph.h"


HadithTaggerDialog::HadithTaggerDialog(QString filename)
		:AbstractTwoLevelAnnotator(filename,"Hadith") {
	resultTreeMenuMIdxMain=resultTreeMenuMIdxApplied=NULL;
	chosenAction=false;
}

void HadithTaggerDialog::createActions(QString mainStructure)
{
	AbstractTwoLevelAnnotator::createActions(mainStructure);

	colorSelectedNodeInGraphAct = new QAction(QString("&Color Selected Node in Graph"), this);
	colorSelectedNodeInGraphAct->setStatusTip(QString("When selected, selected node in Tree View is colored in the image of the graph"));
	colorSelectedNodeInGraphAct->setCheckable(true);
	colorSelectedNodeInGraphAct->setChecked(true);


	resetOneEqualityAct = new QAction(QString("&Remove Entry"), this);
	resetOneEqualityAct->setStatusTip(QString("Removes selected item from the table"));
	connect(resetOneEqualityAct, SIGNAL(triggered()), this, SLOT(resetOneEquality_clicked()));

	resetAllEqualityAct = new QAction(QString("&Clear All"), this);
	resetAllEqualityAct->setStatusTip(QString("Clear contents of whole table file"));
	connect(resetAllEqualityAct, SIGNAL(triggered()), this, SLOT(resetAllEqualities_clicked()));

	addEqualityPairsAct = new QAction(QString("Add Extra Entries"), this);
	addEqualityPairsAct->setStatusTip(QString("Adds the specified number of entries to table randomly from already tagged sanads of hadith book"));
	connect(addEqualityPairsAct, SIGNAL(triggered()), this, SLOT(addEqualityEntries_clicked()));

	narratorsEqualAct = new QAction(QString("Equal"), this);
	narratorsEqualAct->setStatusTip(QString("Indicates the whether the pair of the selected narrators are equal. (Edittable)"));
	narratorsEqualAct->setCheckable(true);
	narratorsEqualAct->setEnabled(false);
	connect(narratorsEqualAct, SIGNAL(triggered()), this, SLOT(equalNarrator_clicked()));
}



void HadithTaggerDialog::createToolbar() {
	AbstractTwoLevelAnnotator::createToolbar();

	graphToolbar->addAction(colorSelectedNodeInGraphAct);

	narratorEqualityToolBar = addToolBar(tr("Narrator Equality"));
	numberExtraNarratorPairs=new QTextEdit("5",narratorEqualityToolBar);
	numberExtraNarratorPairs->setMaximumHeight(30);
	numberExtraNarratorPairs->setMaximumWidth(50);
	narratorEqualityToolBar->addWidget(numberExtraNarratorPairs);
	narratorEqualityToolBar->addAction(addEqualityPairsAct);
	narratorEqualityToolBar->addAction(resetOneEqualityAct);
	narratorEqualityToolBar->addAction(resetAllEqualityAct);
	narratorEqualityToolBar->addAction(narratorsEqualAct);
}

void HadithTaggerDialog::createMenus() {
	AbstractTwoLevelAnnotator::createMenus();
}

void HadithTaggerDialog::createDocWindows() {
	AbstractTwoLevelAnnotator::createDocWindows();

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

	QDockWidget *dock = new QDockWidget(tr("Equality Matrix"), this);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	equalityMatrixTable=new QTableView(dock);
	equalityMatrixTable->setRowHeight(1,20);
	equalityMatrixTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	equalityMatrixTable->setSelectionMode(QAbstractItemView::SingleSelection);
	dock->setWidget(equalityMatrixTable);
	addDockWidget(Qt::RightDockWidgetArea , dock);
	viewMenu->addAction(dock->toggleViewAction());

	//equalityTableModel=new NarratorEqualityModel(map);
	//equalityMatrixTable->setModel(equalityTableModel);
	equalityTableModel=NULL;
}

QModelIndex HadithTaggerDialog::getEqualitySelectedIndex() {
	QModelIndexList  selection=equalityMatrixTable->selectionModel()->selectedRows(NarratorEqualityModel::COL_EQUALITY);
	for (int i=0;i<selection.size();i++) {
		QModelIndex index=selection[i];
		return index;
	}
	return QModelIndex();
}

void HadithTaggerDialog::refreshEqualityTableModel() {
	if (equalityTableModel!=NULL)
		delete equalityTableModel;
	equalityTableModel=new NarratorEqualityModel(map);
	equalityMatrixTable->setModel(equalityTableModel);
	connect(equalityMatrixTable->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),
			this,SLOT(tableEquality_changedSelection(QModelIndex,QModelIndex)));
}

void HadithTaggerDialog::equalNarrator_clicked() {
	const QModelIndex index=getEqualitySelectedIndex();
	if (!index.isValid() || !narratorsEqualAct->isEnabled())
		return;
	QVariant newValue(narratorsEqualAct->isChecked());
	equalityTableModel->setData(index,newValue);
	equalityMatrixTable->selectRow(index.row());
	//refreshEqualityTableModel();
}

void HadithTaggerDialog::resetOneEquality_clicked() {
	QModelIndex index=getEqualitySelectedIndex();
	if (!index.isValid() )
		return;
	int selectedRow=index.row();
	NarratorMap::iterator itr=map.begin()+selectedRow;
	map.erase(itr);
	refreshEqualityTableModel();
}

void HadithTaggerDialog::resetAllEqualities_clicked() {
	map.clear();
	refreshEqualityTableModel();
}

void HadithTaggerDialog::addEqualityEntries_clicked() {
	typedef NarratorEqualityModel::NarratorPair NarratorPair;
	HadithDagGraph *g=dynamic_cast<HadithDagGraph *>(globalGraph);
	NarratorGraph * graph=g->getGraph();
	srand ( time(NULL) );
	bool ok;
	int num=numberExtraNarratorPairs->toPlainText().toInt(&ok);
	if (!ok) {
		num=1;
	}
	for (int q=0;q<num/2;q++) {
		int i= rand() % tags.size();
		int j= rand() % tags.size();
		const TwoLevelSelection::MainSelectionList	& namesI=tags[i].getNamesList(),
													& namesJ=tags[j].getNamesList();

		if (namesI.size()==0 || namesJ.size()==0) {
			q--;
			continue;
		}
		int k= rand() % namesI.size();
		int h= rand() % namesJ.size();
		if (h==k) {
			q--;
			continue;
		}
		Name n1(string,namesI[k].first,namesI[k].second);
		QString narr1=n1.getString();
		Name n2(string,namesJ[h].first,namesJ[h].second);
		QString narr2=n2.getString();
		if (map.contains(NarratorPair(narr1,narr2)) ||
			map.contains(NarratorPair(narr2,narr1)) ) {
			q--;
			continue;
		}
		map[NarratorPair(narr1,narr2)]=false;
	}
	for (int q=num/2;q<num;q++) {
		int f= rand() % graph->size();
		NarratorNodeIfc * n=graph->getNode(f);
		if (n==NULL || !n->isGraphNode()) {
			q--;
			continue;
		}
		GraphNarratorNode * g=dynamic_cast<GraphNarratorNode*>(n);
		int i=rand()%g->size();
		int j=rand()%g->size();
		int k= rand() % (*g)[i].size();
		int h= rand() % (*g)[j].size();
		if (h==k) {
			q--;
			continue;
		}
		QString narr1=(*g)[i][k].CanonicalName();
		QString narr2=(*g)[j][h].CanonicalName();
		if (map.contains(NarratorPair(narr1,narr2)) ||
			map.contains(NarratorPair(narr2,narr1)) ) {
			q--;
			continue;
		}
		map[NarratorPair(narr1,narr2)]=true;
	}
	refreshEqualityTableModel();
}

void HadithTaggerDialog::tableEquality_changedSelection(const QModelIndex & current,const QModelIndex & previous) {
	const QModelIndex & index=current;
	if (!index.isValid()) {
		narratorsEqualAct->setEnabled(false);
		return;
	}
	narratorsEqualAct->setEnabled(true);
	NarratorMap::iterator itr=map.begin()+index.row();
	bool val=*itr;
	narratorsEqualAct->setChecked(val);
}

bool HadithTaggerDialog::open_action() {
	bool ret= (AbstractTwoLevelAnnotator::open_action());
	QFile file(QString("%1.equal").arg(filename).toStdString().data());
	if (file.open(QIODevice::ReadOnly))	{
		QDataStream in(&file);   // we will serialize the data into the file
		in>>map;
		file.close();
		refreshEqualityTableModel();
	} else {
		error << "Narrator Equality Annotation File does not exist\n";
	}
	return ret;
}

bool HadithTaggerDialog::save_action() {
	AbstractTwoLevelAnnotator::save_action();
	QFile file(QString("%1.equal").arg(filename).toStdString().data());
	if (file.open(QIODevice::WriteOnly)) {
		QDataStream out(&file);   // we will serialize the data into the file
		out	<< map;
		file.close();
		QFile::remove(file.fileName()+".copy");
		file.copy(file.fileName()+".copy");
		return true;
	} else {
		error << "Unexpected Error: Unable to open file '"<<file.fileName()<<"'\n";
		return false;
	}

}

HadithTaggerDialog::~HadithTaggerDialog() {
	delete resultTreeMainMenu;
}

void HadithTaggerDialog::regenerateGlobalGraph() {
	if (globalGraph!=NULL)
		globalGraph->deleteGraph();
	if (tags.size()>0) {
		ChainsContainer container;
		for (int i=0;i<tags.size();i++) {
			AbstractGraph * g=tags[i].getGraph();
			/*Name n(string,tags[i].getMainStart(),tags[i].getMainEnd());
			qDebug()<<n.getString();*/
			AbstractGraph * d=g->duplicate();
			HadithChainGraph * duplicate=dynamic_cast<HadithChainGraph *>(d);
			container.append(new Chain(duplicate->chain));
			duplicate->chain.m_chain.clear();
		}
		globalGraph=new HadithDagGraph(container);
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
	if (colorSelectedNodeInGraphAct->isChecked() && globalGraphAct->isChecked()) {
		QIODevice * d=out.device();
		QString s;
		out.setString(&s);
		DisplayLocalNodesVisitor::DetectedNodesMap map;
		NarratorNodeIfc * node=(NarratorNodeIfc *)index.internalPointer();
		HadithDagGraph * g=dynamic_cast<HadithDagGraph*>(globalGraph);
		map[&node->getCorrespondingNarratorNode()]=1;
		if (node->isChainNode()) {
			ChainNarratorNode * chainNode=dynamic_cast<ChainNarratorNode *>(node);
			ChainNarratorNode * tempNode=chainNode;
			if (!tempNode->isLast()) {
				tempNode=&tempNode->nextInChain();
				do {
					map[&tempNode->getCorrespondingNarratorNode()]=0.5;
					if (tempNode->isLast())
						break;
					tempNode=&tempNode->nextInChain();

				}while(true);
			}
			tempNode=chainNode;
			if (!tempNode->isFirst()) {
				tempNode=&tempNode->prevInChain();
				do {
					map[&tempNode->getCorrespondingNarratorNode()]=0.5;
					if (tempNode->isFirst())
						break;
					tempNode=&tempNode->prevInChain();

				}while(true);
			}
		}
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
	NarratorNodeIfc * newNode=graph->mergeNodes(*node1,*node2);
	bool inTop=false;
	if (graph->top_nodes.contains(node1)) {
		inTop=true;
		graph->top_nodes.removeOne(node1);

	}
	if (graph->top_nodes.contains(node2)) {
		inTop=true;
		graph->top_nodes.removeOne(node2);
	}
	if (inTop)
		graph->top_nodes.append(newNode);
	//graph->correctTopNodesList();

	updateGraphDisplay();
	chosenAction=false; //finished from current one
}

void HadithTaggerDialog::modifiedLocalGraph() {
	if (globalGraph!=NULL) {
		globalGraph->deleteGraph();
		globalGraph=NULL;
	}
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


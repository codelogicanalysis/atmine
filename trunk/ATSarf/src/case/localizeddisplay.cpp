#include "localizeddisplay.h"

void localizedDisplay(NarratorGraph * graph) {
	LocalizedDisplay * l=new LocalizedDisplay(graph);
	l->show();
}

void LocalizedDisplay::find_action() {
	errors_text->clear();
	nodeMap.clear();
	nodeList.clear();
	int nodeCount=0;
	QString text=input->toPlainText();
	segmentNarrators(&text,&LocalizedDisplay::searchNarratorsInHash,this); //will fill map
	ColorNarratorsAction::DetectedNodesMap::iterator itr=nodeMap.begin();
	for (;itr!=nodeMap.end();itr++) {
		NarratorNodeIfc * node=itr.key();
		double similarity=itr.value();
		narratorListDisplay->setRowCount(nodeCount+1);
		narratorListDisplay->setItem(nodeCount,0,new QTableWidgetItem(node->CanonicalName()));
		narratorListDisplay->setItem(nodeCount,1,new QTableWidgetItem(QString("%1").arg(similarity)));
		nodeList.append(node);
		nodeCount++;
	}
	errors->setText(*errors_text);
}
void LocalizedDisplay::display_action() {
	errors_text->clear();
	QList<QTableWidgetSelectionRange>  selection=narratorListDisplay->selectedRanges();
	NarratorNodeIfc * node=NULL;
	for (int i=0;i<selection.size();i++) {
		int topRow=selection[i].topRow();
		for (int j=0;j<selection[i].rowCount();j++) {
			int row=topRow+j;
			node=nodeList[row];
		}
	}
	if (node==NULL)
		return;
	bool v=false;
	int num_steps=steps->toPlainText().toInt(&v);
	if (!v) {
		error<<"Input for steps is not an invalid number!\n";
		errors->setText(*errors_text);
		return;
	}
	report(0);
	DsiplayLocalNodesVisitor visitor(nodeMap);
	GraphVisitorController c(&visitor,graph,true,true);
	node->BFS_traverse(c,num_steps,1);
	node->BFS_traverse(c,num_steps,-1);
	setCurrentAction("Completed");
	report(100);
	try{
		system("dot -Tsvg graph.dot -o graph.svg");
		pic->setPixmap(QPixmap("./graph.svg"));
		subScrollArea->setWidget(pic);
	}catch(...) {}
	errors->setText(*errors_text);
}

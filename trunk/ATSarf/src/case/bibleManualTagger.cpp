#include <QtAlgorithms>
#include <QtGui>

#include "bibleManualTagger.h"
#include "geneMainwindow.h"
#include "bibleGeneology.h"
#include "genealogyItem.h"

BibleTaggerDialog::BibleTaggerDialog(QString filename)
	:AbstractTwoLevelAnnotator(filename,"Genealogy") { }

void BibleTaggerDialog::regenerateGlobalGraph() {
	if (tags.size()>0)
		globalGraph=tags[0].getGraph()->duplicate();
	GeneTree * global=dynamic_cast<GeneTree *>(globalGraph);
	for (int i=1;i<tags.size();i++)
		global->merge(tags[i].getGraph());
	if (global!=NULL)
		global->mergeLeftovers();
}
void BibleTaggerDialog::displayGraph(AbstractGraph * tree){
	try{
		if (tree!=NULL) {
			system("dot -Tsvg graph.dot -o graph.svg");
			graph->setPixmap(QPixmap("./graph.svg"));
		} else
			graph->setPixmap(QPixmap());
		graph->repaint();
		GeneTree * t=dynamic_cast<GeneTree *>(tree);
		treeModel=new GeneItemModel(t);
		resultTree->setModel(treeModel);
	} catch(...) {}
}

BibleTaggerDialog::~BibleTaggerDialog() { }

int bibleTagger(QString input_str){
	BibleTaggerDialog * d=new BibleTaggerDialog(input_str);
	d->show();
	return 0;
}

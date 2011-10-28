#include <QtGui>
#include <assert.h>
#include "abstractTwoLevelAnnotator.h"
#include "logger.h"

#define min(a,b) (a<b?a:b)
#define max(a,b) (a>b?a:b)

void AbstractTwoLevelAnnotator::SelectionList::readFromStream(QDataStream & in,AbstractGraph * graph){
	int count;
	in>>count;
	for (int i=0;i<count;i++) {
		AbstractGraph *g=graph->duplicate();
		TwoLevelSelection s(g);
		in >>s;
		append(s);
	}
}

AbstractTwoLevelAnnotator::AbstractTwoLevelAnnotator(QString filename, QString mainStructure, QString ext):QMainWindow() {
	this->filename=filename;
	this->ext=ext;
	string=NULL;
	globalGraph=NULL;

	createActions(mainStructure);
	createToolbar();
	createMenus();
	createDocWindows();

	connect(text,SIGNAL(selectionChanged()),this,SLOT(text_selectionChanged()));

	setWindowTitle(filename);
	this->resize(700,700);
	selectedTagIndex=-1;
}

void AbstractTwoLevelAnnotator::createActions(QString mainStructure)
{
	tagMainAct = new QAction(QString("&Tag "+mainStructure), this);
	tagMainAct->setStatusTip(QString("Tag the main structure in the heirarchy"));
	connect(tagMainAct, SIGNAL(triggered()), this, SLOT(tagMain_clicked()));

	unTagMainAct = new QAction(QString("&Un-Tag "+mainStructure), this);
	unTagMainAct->setStatusTip(QString("Untag a previously taggged main structure in the heirarchy"));
	connect(unTagMainAct, SIGNAL(triggered()), this, SLOT(unTagMain_clicked()));

	unTagNameAct = new QAction(QString("Un-Ta&g Name"), this);
	unTagNameAct->setStatusTip(QString("Untag a previously tagged name inside the main structure"));
	connect(unTagNameAct, SIGNAL(triggered()), this, SLOT(unTagName_clicked()));

	tagNameAct = new QAction(QString("Tag &Name"), this);
	tagNameAct->setStatusTip(QString("Tag a name inside the main structure"));
	connect(tagNameAct, SIGNAL(triggered()), this, SLOT(tagName_clicked()));

	forceWordNamesAct = new QAction(QString("&Full Word Names"), this);
	forceWordNamesAct->setStatusTip(QString("When selected forces selected names to be full words delimited by spaces and punctuation "));
	forceWordNamesAct->setCheckable(true);
	forceWordNamesAct->setChecked(true);

	forcePunctuationMainBoundaryAct= new QAction(QString("&Punctuation-Delimeted "+mainStructure), this);
	forcePunctuationMainBoundaryAct->setStatusTip(QString("When selected forces selected "+ mainStructure+ " to start and end at punctuation marks only"));
	forcePunctuationMainBoundaryAct->setCheckable(true);
	forcePunctuationMainBoundaryAct->setChecked(true);

	saveAct = new QAction(QString("&Save"), this);
	saveAct->setShortcuts(QKeySequence::Save);
	saveAct->setStatusTip(QString("Save the annotation file"));
	connect(saveAct, SIGNAL(triggered()), this, SLOT(save_clicked()));

	modifyGraphAct = new QAction(QString("&Modify Graph"), this);
	modifyGraphAct->setStatusTip(QString("Reset Graph of current selection using written text"));
	connect(modifyGraphAct, SIGNAL(triggered()), this, SLOT(modifyGraph_clicked()));

	globalGraphAct = new QAction(QString("&Global Graph"), this);
	globalGraphAct->setCheckable(true);
	globalGraphAct->setStatusTip(QString("When selected, displays global graph"));
	connect(globalGraphAct, SIGNAL(toggled(bool)), this, SLOT(isGlobalGraph_toggled(bool)));


	resetGlobalGraphAct = new QAction(QString("&Reset Global Graph"), this);
	resetGlobalGraphAct->setStatusTip(QString("Recompute global graph using automatic default algorithm"));
	resetGlobalGraphAct->setEnabled(false);
	connect(resetGlobalGraphAct, SIGNAL(triggered()), this, SLOT(resetGlobalGraph_clicked()));

}

void AbstractTwoLevelAnnotator::createToolbar() {
	annotationToolbar = addToolBar(tr("Annotation"));
	annotationToolbar->addAction(tagMainAct);
	annotationToolbar->addAction(unTagMainAct);
	annotationToolbar->addSeparator();
	annotationToolbar->addAction(tagNameAct);
	annotationToolbar->addAction(unTagNameAct);
	annotationToolbar->addAction(forcePunctuationMainBoundaryAct);
	annotationToolbar->addAction(forceWordNamesAct);
	annotationToolbar->addAction(saveAct);

	addToolBarBreak();

	graphToolbar = addToolBar(tr("Graph"));
	graphToolbar->addAction(modifyGraphAct);
	graphToolbar->addAction(globalGraphAct);
	graphToolbar->addAction(resetGlobalGraphAct);
}

void AbstractTwoLevelAnnotator::createMenus() {
	viewMenu = menuBar()->addMenu(tr("&View"));
	menuBar()->addSeparator();
}

void AbstractTwoLevelAnnotator::createDocWindows() {
	QDockWidget *dock = new QDockWidget(tr("File Text"), this);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	text=new QTextBrowser(dock);
	dock->setWidget(text);
	addDockWidget(Qt::LeftDockWidgetArea, dock);
	viewMenu->addAction(dock->toggleViewAction());
	setCentralWidget(dock);

	dock = new QDockWidget(tr("Graph Text"), this);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	treeText=new QTextBrowser(dock);
	treeText->setReadOnly(false);
	treeText->setLayoutDirection(Qt::RightToLeft);
	dock->setWidget(treeText);
	addDockWidget(Qt::RightDockWidgetArea, dock);
	viewMenu->addAction(dock->toggleViewAction());

	dock = new QDockWidget(tr("Graph View"), this);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	graphArea=new QScrollArea(dock);
	dock->setWidget(graphArea);
	graph=new QLabel(graphArea);
	graphArea->setWidget(graph);
	graphArea->setWidgetResizable(true);
	graphArea->setAlignment(Qt::AlignCenter);
	dock->setWidget(graphArea);
	addDockWidget(Qt::RightDockWidgetArea, dock);
	viewMenu->addAction(dock->toggleViewAction());

	dock = new QDockWidget(tr("Tree View"), this);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	resultTree = new QTreeView (dock);
	resultTree->setSelectionBehavior (QAbstractItemView::SelectRows);
	resultTree->setSelectionMode (QAbstractItemView::SingleSelection);
	resultTree->setAlternatingRowColors (true);
	dock->setWidget(resultTree);
	addDockWidget(Qt::BottomDockWidgetArea, dock);
	viewMenu->addAction(dock->toggleViewAction());
	treeModel=NULL;
	resultTree->setModel(treeModel);
}

void AbstractTwoLevelAnnotator::show(){
	globalGraph=newGraph();
	open_action();
	if (globalGraph==NULL)
		regenerateGlobalGraph();
	QMainWindow::show();
}

void AbstractTwoLevelAnnotator::refreshTreeModel(QAbstractItemModel * newModel) {
	resultTree->setModel(newModel);
	if (treeModel!=NULL)
		delete treeModel;
	treeModel=newModel;
}

int AbstractTwoLevelAnnotator::getNameIndexInAll(const QString & name) {
	for (int i=0;i<tags.size();i++) {
		int j=tags[i].getNameIndex(name);
		if (j>=0)
			return tags[i].getNamesList()[j].first;
	}
	return -1;
}

int AbstractTwoLevelAnnotator::findSubSelection(int tagIndex,int startSubIndex, SelectionMode selectionMode) { //based on user text selection
	if (this==NULL)
		return -1;
	QTextCursor c=text->textCursor();
	int start=c.selectionStart();
	int end=c.selectionEnd()-1;
	TwoLevelSelection::MainSelectionList names=tags[tagIndex].getNamesList();
	for (int i=startSubIndex;i<names.size();i++) {
		if (isConsistentWithSelectionCondidtion(start,end,names[i].first,names[i].second,selectionMode)) {
			return i;
		}
	}
	return -1;
}


AbstractTwoLevelAnnotator::~AbstractTwoLevelAnnotator() {
	for (int i=0;i<tags.size();i++) {
		if (tags[i].graph!=NULL)
			delete tags[i].graph;
	}
	if (globalGraph!=NULL)
		delete globalGraph;
	if (string !=NULL)
		delete string;
	if (treeModel!=NULL)
		delete treeModel;
	delete text;
	delete treeText;
	delete graph;
}

bool AbstractTwoLevelAnnotator::mergeMainStructures(TwoLevelSelection * oldSelection, int newTagIndex) {
	TwoLevelSelection::MainSelectionList mergedNames;
	TwoLevelSelection::mergeNames(string,oldSelection->names,tags[newTagIndex].names,mergedNames);
	AbstractGraph * duplicate=oldSelection->graph->duplicate();
	duplicate->merge(tags[newTagIndex].graph);
	if (duplicate->isRepresentativeOf(mergedNames)) {
		oldSelection->names=mergedNames;
		oldSelection->graph->deleteGraph();
		tags[newTagIndex].graph->deleteGraph();
		oldSelection->graph=duplicate;
		oldSelection->setMainInterval(min(oldSelection->getMainStart(),tags[newTagIndex].getMainStart()),max(oldSelection->getMainEnd(),tags[newTagIndex].getMainEnd()));
		return true;
	} else {
		duplicate->deleteGraph(); //stopped before completion i>=0
		return false;
	}
}

void AbstractTwoLevelAnnotator::tagMain_action() {
	if (this==NULL)
		return;
	if (forcePunctuationMainBoundaryAct->isChecked())
		moveSelectionToSentenceBoundaries();
	else
		moveSelectionToWordBoundaries();
	QTextCursor c=text->textCursor();
	int start=c.selectionStart();
	int end=c.selectionEnd();
	if (start==end)
		return;
	int i=findSelection(0,SELECTION_OUTSIDEOVERLAP);
	TwoLevelSelection * sel=NULL;
	while (i>=0) {
		if (sel==NULL) {
			sel=&tags[i];
		} else {
			if (mergeMainStructures(sel,i)) {
				tags.removeAt(i);
				i--;
			} else {
				break;
			}
		}
		i=findSelection(i+1,SELECTION_OUTSIDEOVERLAP);
	}
	if (sel==NULL) {
		tags.append(TwoLevelSelection(newGraph(),string,start,end-1));
	} else if (i>=0) { //stopped before completion
		start=sel->getMainStart();
		end=sel->getMainEnd();
	} else {
		sel->setMainInterval(min(sel->getMainStart(),start),max(sel->getMainEnd(),end-1));
	}
	c.setPosition(start,QTextCursor::MoveAnchor);
	c.setPosition(end,QTextCursor::KeepAnchor);
	text->setTextCursor(c);
	text->setTextBackgroundColor(Qt::darkYellow);
	c.clearSelection();
	text->setTextCursor(c);
	if (globalGraphAct->isChecked())
		return;
	updateGraphDisplay();
}

void AbstractTwoLevelAnnotator::unTagMain_action() {
	if (this==NULL)
		return;
	TwoLevelSelection::MainSelectionList listForRemoval;
	int i=findSelection(0,SELECTION_OUTSIDE);
	if (i>=0) {
		while (i>=0) {
			listForRemoval.append(TwoLevelSelection::MainSelection(tags[i].getMainStart(),tags[i].getMainEnd()));
			tags.removeAt(i);
			i--;
			i=findSelection(i+1,SELECTION_OUTSIDE);
		}
	} else {
		QTextCursor c=text->textCursor();
		int start=c.selectionStart();
		int end=c.selectionEnd();
		int i=findSelection(0,SELECTION_OUTSIDEOVERLAP);
		if (i>=0) {
			listForRemoval.append(TwoLevelSelection::MainSelection(start,end-1));
		}
		while (i>=0) {
			if (tags[i].getMainStart()>=start) {
				tags[i].setMainInterval(end,tags[i].getMainEnd());
			} else {
				tags[i].setMainInterval(tags[i].getMainStart(),start-1);
			}
			if (tags[i].getMainStart()==tags[i].getMainEnd()) {
				tags.removeAt(i);
				i--;
			} else
				tags[i].removeExtraNames();
			i=findSelection(i+1,SELECTION_OUTSIDEOVERLAP);
		}
	}
	QTextCursor c=text->textCursor();
	for (int i=0;i<listForRemoval.size();i++) {
		c.setPosition(listForRemoval[i].first,QTextCursor::MoveAnchor);
		c.setPosition(listForRemoval[i].second+1,QTextCursor::KeepAnchor);
		text->setTextCursor(c);
		text->setTextBackgroundColor(Qt::white);
		text->setTextColor(Qt::black);
		text->setTextCursor(c);
	}
	c.clearSelection();
	text->setTextCursor(c);
	if (globalGraphAct->isChecked())
		return;
	updateGraphDisplay();
}

void AbstractTwoLevelAnnotator::save_action() {
	qSort(tags.begin(),tags.end());
	QFile file(QString("%1"+ext).arg(filename).toStdString().data());
	if (file.open(QIODevice::WriteOnly)) {
		QDataStream out(&file);   // we will serialize the data into the file
		out	<< tags;
		assert (globalGraph!=NULL);
		globalGraph->writeToStream(out);
		file.close();
		QFile::remove(file.fileName()+".copy");
		file.copy(file.fileName()+".copy");
	} else
		error << "Unexpected Error: Unable to open file\n";
}

void AbstractTwoLevelAnnotator::open_action() {
	QFile fileOriginal(QString(filename).toStdString().data());
	if (fileOriginal.open(QIODevice::ReadOnly))	{
		text->clear();
		QTextStream s(&fileOriginal);
		s.setCodec("utf-8");
		text->setLayoutDirection(Qt::RightToLeft);
		QTextCursor c=text->textCursor();
		c.clearSelection();
		c.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
		text->setTextCursor(c);
		text->setTextBackgroundColor(Qt::white);
		text->setTextColor(Qt::black);
		QString sss=s.readAll();
		//qDebug()<<sss.size();
		string=new QString(sss);
		//qDebug()<<string->size();
		text->setText(*string);
		//qDebug()<<text->toPlainText().size();

		QFile file(QString("%1"+ext).arg(filename).toStdString().data());
		if (file.open(QIODevice::ReadOnly))	{
			QDataStream in(&file);   // we will serialize the data into the file
			globalGraph=newGraph();
			tags.readFromStream(in,globalGraph);
			globalGraph->deleteGraph();
			globalGraph=newGraph(true);
			if (!in.atEnd())
				globalGraph->readFromStream(in);
			globalGraph->fillTextPointers(string);
			file.close();
			for (int i=0;i<tags.size();i++) {
				tags[i].text=string;
				tags[i].getGraph()->fillTextPointers(string);
				QTextCursor c=text->textCursor();
				int start=tags[i].getMainStart();
				int end=tags[i].getMainEnd();
				c.setPosition(start,QTextCursor::MoveAnchor);
				c.setPosition(end+1,QTextCursor::KeepAnchor);
				text->setTextCursor(c);
				text->setTextBackgroundColor(Qt::darkYellow);
				const TwoLevelSelection::MainSelectionList & names=tags[i].getNamesList();
				for (int i=0;i<names.size();i++) {
					int start=names[i].first;
					int end=names[i].second;
					c.setPosition(start,QTextCursor::MoveAnchor);
					c.setPosition(end+1,QTextCursor::KeepAnchor);
					text->setTextCursor(c);
					text->setTextColor(Qt::white);
				}
			}
			c.clearSelection();
			c.setPosition(0);
			text->setTextCursor(c);

		} else {
			error << "Annotation File does not exist\n";
		}
	} else {
		error << "File does not exist\n";
	}
}

void AbstractTwoLevelAnnotator::unTagName_action() {
	if (this==NULL)
		return;
	QTextCursor c=text->textCursor();
	int i=findSelection(0,SELECTION_INSIDE);
	TwoLevelSelection::MainSelectionList listForRemoval;
	while (i>=0) { //while not useful
		const TwoLevelSelection::MainSelectionList & names=tags[i].getNamesList();
		int j=findSubSelection(i,0);
		while (j>=0) {
			listForRemoval.append(TwoLevelSelection::MainSelection(names[j].first,names[j].second));
			tags[i].removeNameAt(j);
			j--;
			j=findSubSelection(i,j+1);
		}
		i=findSelection(i+1,SELECTION_INSIDE);
	}
	for (int i=0;i<listForRemoval.size();i++) {
		c.setPosition(listForRemoval[i].first,QTextCursor::MoveAnchor);
		c.setPosition(listForRemoval[i].second+1,QTextCursor::KeepAnchor);
		text->setTextCursor(c);
		text->setTextColor(Qt::black);
		text->setTextCursor(c);
	}
	c.clearSelection();
	text->setTextCursor(c);
	if (globalGraphAct->isChecked())
		return;
	updateGraphDisplay();
}

void AbstractTwoLevelAnnotator::tagName_action() {
	if (this==NULL)
		return;
	if (forceWordNamesAct->isChecked())
		moveSelectionToWordBoundaries();
	QTextCursor c=text->textCursor();
	int start=c.selectionStart();
	int end=c.selectionEnd();
	if (start==end)
		return;
	int i=findSelection(0,SELECTION_INSIDE);
	if (i>=0) {
		int j=findSubSelection(0);
		if (j<0) {
			tags[i].addName(start,end-1);
			text->setTextColor(Qt::white);
			c.clearSelection();
			text->setTextCursor(c);
		}
	}
	if (globalGraphAct->isChecked())
		return;
	updateGraphDisplay();
}

void AbstractTwoLevelAnnotator::text_selectionChangedAction() {
	int i=findSelection(0,SELECTION_INSIDE);
	if (selectedTagIndex==i)
		return;
	if (globalGraphAct->isChecked())
		return;
	updateGraphDisplay();
}

void AbstractTwoLevelAnnotator::updateGraphDisplay() {
	if (!globalGraphAct->isChecked()) {
		int i=findSelection(0,SELECTION_INSIDE);
		if (i>=0) {
			treeText->setText(tags[i].getText());
			treeText->setReadOnly(false);
			modifyGraphAct->setEnabled(true);
			AbstractGraph * graph=tags[i].getGraph();
			if (graph==NULL)
				displayGraph(NULL);
			else
				graph->displayGraph(this);
			selectedTagIndex=i;
		} else {
			treeText->setText("");
			treeText->setReadOnly(true);
			modifyGraphAct->setEnabled(false);
			displayGraph(NULL);
			selectedTagIndex=-1;
		}
	} else {
		treeText->setText(globalGraph->getText());
		treeText->setReadOnly(false);
		modifyGraphAct->setEnabled(true);
		if (globalGraph==NULL)
			displayGraph(NULL);
		else
			globalGraph->displayGraph(this);
		selectedTagIndex=-1;
	}
}

void AbstractTwoLevelAnnotator::displayGraph(AbstractGraph *aGraph) {
	QAbstractItemModel * model;
	if (aGraph!=NULL) {
		model=aGraph->getTreeModel();
	} else {
		model=NULL;
	}
	refreshTreeModel(model);
	if (aGraph!=NULL) {
		try{
			system("dot -Tsvg graph.dot -o graph.svg");
			graph->setPixmap(QPixmap("./graph.svg"));
		} catch(...) {}
	} else
		graph->setPixmap(QPixmap());
	graph->repaint();
}

void AbstractTwoLevelAnnotator::modifyGraph_action() {
	if (!globalGraphAct->isChecked()) {
		int i=findSelection(0,SELECTION_INSIDE);
		assert(i>=0);
		if (tags[i].getGraph()->buildFromText(treeText->toPlainText(),&tags[i]))
			tags[i].getGraph()->displayGraph(this);
	} else {
		if (globalGraph->buildFromText(treeText->toPlainText(),NULL,string,this))
			globalGraph->displayGraph(this);
	}
}

void AbstractTwoLevelAnnotator::isGlobalGraph_action(){
	updateGraphDisplay();
	resetGlobalGraphAct->setEnabled(globalGraphAct->isChecked());
}

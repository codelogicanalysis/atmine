#include <QtGui>
#include <assert.h>
#include "abstractTwoLevelAnnotator.h"
#include "logger.h"

#define min(a,b) (a<b?a:b)
#define max(a,b) (a>b?a:b)



AbstractTwoLevelAnnotator::AbstractTwoLevelAnnotator(QString filename, QString mainStructure):QMainWindow() {
	this->filename=filename;
	text=new QTextBrowser(this);
	tagMain=new QPushButton("&Tag "+mainStructure,this);
	unTagMain=new QPushButton("&Un-Tag "+mainStructure,this);
	tagName=new QPushButton("Tag &Name",this);
	unTagName=new QPushButton("Un-Ta&g Name",this);
	save=new QPushButton("&Save",this);
	modifyGraph=new QPushButton("&Modify Graph",this);
	scrollArea=new QScrollArea(this);
	forceWordNames=new QCheckBox("Full Word Names");
	resetGlobalGraph=new QPushButton("Reset Global Graph");
	resetGlobalGraph->setEnabled(false);
	isGlobalGraph=new QPushButton("Global Graph");
	isGlobalGraph->setCheckable(true);
	graphArea=new QScrollArea(this);
	treeText=new QTextBrowser(this);
	treeText->setReadOnly(false);
	grid=new QGridLayout(scrollArea);
	grid->addWidget(tagMain,0,0);
	grid->addWidget(unTagMain,0,1);
	grid->addWidget(forceWordNames,0,2);
	grid->addWidget(tagName,0,3);
	grid->addWidget(unTagName,0,4);
	grid->addWidget(save,0,5);
	grid->addWidget(text,1,0,4,2);
	grid->addWidget(treeText,1,2,1,1);
	grid->addWidget(modifyGraph,2,2);
	grid->addWidget(isGlobalGraph,3,2);
	grid->addWidget(resetGlobalGraph,4,2);
	grid->addWidget(graphArea,1,3,4,3);
	grid->setColumnStretch(0,2);
	grid->setColumnStretch(1,2);
	grid->setColumnStretch(2,4);
	grid->setColumnStretch(3,2);
	grid->setColumnStretch(4,2);
	grid->setColumnStretch(5,2);
	graph=new QLabel(graphArea);
	graphArea->setWidgetResizable(true);
	graphArea->setAlignment(Qt::AlignCenter);
	graphArea->setWidget(graph);
	forceWordNames->setChecked(true);
	setCentralWidget(scrollArea);
	treeText->setLayoutDirection(Qt::RightToLeft);
#ifdef ERRORS_BIBLE
	errors=new QTextBrowser(this);
	errors->resize(errors->width(),50);
	errors_text=new QString();
	grid->addWidget(errors,5,0,1,6);
	displayed_error.setString(errors_text);
	errors->setText(*errors_text);
#endif
	connect(tagMain,SIGNAL(clicked()),this,SLOT(tagMain_clicked()));
	connect(unTagMain,SIGNAL(clicked()),this,SLOT(unTagMain_clicked()));
	connect(tagName,SIGNAL(clicked()),this,SLOT(tagName_clicked()));
	connect(unTagName,SIGNAL(clicked()),this,SLOT(unTagName_clicked()));
	connect(save,SIGNAL(clicked()),this,SLOT(save_clicked()));
	connect(text,SIGNAL(selectionChanged()),this, SLOT(text_selectionChanged()));
	connect(modifyGraph,SIGNAL(clicked()),this, SLOT(modifyGraph_clicked()));
	connect(isGlobalGraph,SIGNAL(toggled(bool)),this, SLOT(isGlobalGraph_toggled(bool)));
	connect(resetGlobalGraph,SIGNAL(clicked()),this, SLOT(resetGlobalGraph_clicked()));
	string=NULL;
	globalGraph=NULL;
	open_action();
	/*if (globalGraph==NULL)
		regenerateGlobalGraph();*/
	setWindowTitle(filename);
	this->resize(700,700);
	selectedTagIndex=-1;
	modifyGraph->setEnabled(false);
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
	int end=c.selectionEnd();
	Selection::MainSelectionList names=tags[tagIndex].getNamesList();
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

	delete isGlobalGraph;
	delete resetGlobalGraph;
	delete tagMain;
	delete unTagMain;
	delete tagName;
	delete unTagName;
	delete modifyGraph;
	delete save;
	delete text;
	delete treeText;
	delete forceWordNames;
	delete scrollArea;
	delete graphArea;
	delete graph;
#ifdef ERRORS_BIBLE
	delete errors;
	delete errors_text;
#endif
	delete grid;
}

void AbstractTwoLevelAnnotator::tagMain_action() {
	if (this==NULL)
		return;
	moveSelectionToSentenceBoundaries();
	QTextCursor c=text->textCursor();
	int start=c.selectionStart();
	int end=c.selectionEnd();
	if (start==end)
		return;
	int i=findSelection(0,SELECTION_OUTSIDEOVERLAP);
	QList<int> listForRemoval;
	Selection * sel=NULL;
	while (i>=0) {
		if (sel==NULL) {
			sel=&tags[i];
		} else {
			Selection::MainSelectionList mergedNames;
			Selection::mergeNames(string,sel->names,tags[i].names,mergedNames);
			AbstractGraph * duplicate=sel->graph->duplicate();
			duplicate->merge(tags[i].graph);
			if (duplicate->isRepresentativeOf(mergedNames)) {
				listForRemoval.append(i);
				sel->names=mergedNames;
				sel->graph->deleteGraph();
				tags[i].graph->deleteGraph();
				sel->graph=duplicate;
				sel->setMainInterval(min(sel->getMainStart(),tags[i].getMainStart()),max(sel->getMainEnd(),tags[i].getMainEnd()));
				tags.removeAt(i);
				i--;
			} else {
				duplicate->deleteGraph(); //stopped before completion i>=0
				break;
			}
		}
		i=findSelection(i+1,SELECTION_OUTSIDEOVERLAP);
	}
	if (sel==NULL) {
		tags.append(Selection(string,start,end-1));
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
	if (isGlobalGraph->isChecked())
		return;
	updateGraphDisplay();
}

void AbstractTwoLevelAnnotator::unTagMain_action() {
	if (this==NULL)
		return;
	Selection::MainSelectionList listForRemoval;
	int i=findSelection(0,SELECTION_OUTSIDE);
	if (i>=0) {
		while (i>=0) {
			listForRemoval.append(Selection::MainSelection(tags[i].getMainStart(),tags[i].getMainEnd()));
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
			listForRemoval.append(Selection::MainSelection(start,end-1));
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
	if (isGlobalGraph->isChecked())
		return;
	updateGraphDisplay();
}

void AbstractTwoLevelAnnotator::save_action() {
	qSort(tags.begin(),tags.end());
	QFile file(QString("%1.tags").arg(filename).toStdString().data());
	if (file.open(QIODevice::WriteOnly)) {
		QDataStream out(&file);   // we will serialize the data into the file
		out	<< tags;
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

		QFile file(QString("%1.tags").arg(filename).toStdString().data());
		if (file.open(QIODevice::ReadOnly))	{
			QDataStream in(&file);   // we will serialize the data into the file
			in>>tags;
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
				const Selection::MainSelectionList & names=tags[i].getNamesList();
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
	Selection::MainSelectionList listForRemoval;
	while (i>=0) { //while not useful
		const Selection::MainSelectionList & names=tags[i].getNamesList();
		int j=findSubSelection(i,0);
		while (j>=0) {
			listForRemoval.append(Selection::MainSelection(names[j].first,names[j].second));
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
	if (isGlobalGraph->isChecked())
		return;
	updateGraphDisplay();
}

void AbstractTwoLevelAnnotator::tagName_action() {
	if (this==NULL)
		return;
	if (forceWordNames->isChecked())
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
	if (isGlobalGraph->isChecked())
		return;
	updateGraphDisplay();
}

void AbstractTwoLevelAnnotator::text_selectionChangedAction() {
	int i=findSelection(0,SELECTION_INSIDE);
	if (selectedTagIndex==i)
		return;
	if (isGlobalGraph->isChecked())
		return;
	updateGraphDisplay();
}

void AbstractTwoLevelAnnotator::updateGraphDisplay() {
	if (!isGlobalGraph->isChecked()) {
		int i=findSelection(0,SELECTION_INSIDE);
		if (i>=0) {
			treeText->setText(tags[i].getText());
			treeText->setReadOnly(false);
			modifyGraph->setEnabled(true);
			tags[i].getGraph()->displayGraph(this);
			selectedTagIndex=i;
		} else {
			treeText->setText("");
			treeText->setReadOnly(true);
			modifyGraph->setEnabled(false);
			graph->clear();
			selectedTagIndex=-1;
		}
	} else {
		treeText->setText(globalGraph->getText());
		treeText->setReadOnly(false);
		modifyGraph->setEnabled(true);
		globalGraph->displayGraph(this);
		selectedTagIndex=-1;
	}
}

void AbstractTwoLevelAnnotator::modifyGraph_action() {
	if (!isGlobalGraph->isChecked()) {
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
	resetGlobalGraph->setEnabled(isGlobalGraph->isChecked());
}

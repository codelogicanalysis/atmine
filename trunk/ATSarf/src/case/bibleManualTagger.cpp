#include "bibleManualTagger.h"
#include "geneMainwindow.h"
#include <QtAlgorithms>



void BibleTaggerDialog::Selection::FillNullTree() {
	if (tree==NULL && names.size()>0) {
		GeneNode * root=new GeneNode(Name(text,names.at(0).first,names.at(0).second),NULL);
		tree=new GeneTree(root);
		for (int i=1;i<names.size();i++) {
			root->addChild(new GeneNode(Name(text,names.at(i).first,names.at(i).second),NULL));
		}
	}
}

int BibleTaggerDialog::Selection::getNameIndex(QString s) {
	for (int i=0;i<names.size();i++) {
		if (equalNames(Name(text,names[i].first,names[i].second).getString(),s))
			return i;
	}
	return -1;
}

void BibleTaggerDialog::Selection::removeExtraNames() {
	for (int i=0;i<names.size();i++) {
		if (!isConsistentWithSelectionCondidtion(main.first,main.second,names[i].first,names[i].second,SELECTION_OUTSIDE)) {
			removeNameAt(i);
			i--;
		}
	}
}

BibleTaggerDialog::Selection::Selection():main(-1,-1) {
	tree=NULL;
	text=NULL;
}
BibleTaggerDialog::Selection::Selection(QString * text,int start,int end):main(start,end) {
	tree=NULL;
	this->text=text;
}
void BibleTaggerDialog::Selection::addName(int start,int end) {
	names.append(MainSelection(start,end));
	FillNullTree();
	if (names.size()>1) {
		tree->getRoot()->addChild(new GeneNode(Name(text,start,end),NULL));
	}
}
void BibleTaggerDialog::Selection::addName( const Name & name) {
	addName(name.getStart(),name.getEnd());
}

void BibleTaggerDialog::Selection::removeNameAt(int i) {
	MainSelection s=names[i];
	names.removeAt(i);
	QString name=Name(text,s.first,s.second/*-1*/).getString();
	GeneNode * n=tree->findTreeNode(name,true);
	assert(n!=NULL);
	if (n->getParent()==NULL) { //is root
		assert(tree->getRoot()==n);
		if (n->spouses.size()>0) {
			if (equalNames(n->toString(),name)) {
				n->name=n->spouses[0]->getName();
				n->spouses.removeAt(0);
			} else {
				for (int i=0;i<n->spouses.size();i++) {
					if (equalNames(n->spouses[i]->getString(),n->toString())){
						n->spouses.removeAt(i);
						break;
					}
				}
			}
		} else if (n->children.size()>0) {
			tree->setRoot(n->children[0]);
			n->children[0]->parent=NULL;
			for (int i=1;i<n->children.size();i++)
				n->children[0]->addChild(n->children[i]);
			n->children[0]->parent=NULL;
			delete n;
		} else {
			tree->deleteTree();
			tree=NULL;
			FillNullTree();
		}
	} else {
		n->parent->children.removeOne(n);
		for (int i=0;i<n->spouses.size();i++) {
			n->parent->addChild(new GeneNode(n->spouses[i]->getName(),NULL));
		}
		for (int i=0;i<n->children.size();i++) {
			n->parent->addChild(n->children[i]);
		}
		delete n;
	}
}
bool BibleTaggerDialog::Selection::updateGraph(QString text, Selection * sel,GeneTree ** tree, QString *fileText,BibleTaggerDialog * tagger) {
	QStringList lines=text.split("\n",QString::SkipEmptyParts);
	QString s;
	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setWindowTitle("Error Processing Tree");
	GeneTree * newtree=new GeneTree();
	GeneNode * lastNode=NULL;
	int lastIndentation=-1;
	foreach(s,lines) {
		int indentation=0;
		GeneNode * child=NULL;
		while (indentation<s.size() && s.at(indentation)=='>') {
			indentation++;
		}
		if (indentation==lastIndentation+1) {
			//just keep old values for lastIndentation and lastNode
		} else if (indentation>lastIndentation+1) {
			msgBox.setText(QString("Syntax error with indentation at line: \"%1\"").arg(s));
			msgBox.exec();
			newtree->deleteTree();
			return false;
		} else {
			int diff=lastIndentation-indentation+1;
			for (int i=0;i<diff;i++) {
				if (lastNode==NULL) {
					msgBox.setText(QString("Syntax error with indentation at line: \"%1\", parent to which to attach node not found").arg(s));
					msgBox.exec();
					newtree->deleteTree();
					return false;
				}
				lastNode=lastNode->parent;
			}
			lastIndentation=indentation-1;
		}

		bool noSpouses=false;
		int l=s.indexOf('[')-1;
		if (l<0) {
			l=s.length()-1;
			noSpouses=true;
		}
		QString nameString=Name(&s,indentation,l).getString();
		if (sel==NULL) {
			int pos=tagger->getNameIndexInAll(nameString);
			if (pos>=0) {
				Name name=Name(fileText,pos, pos+nameString.length()-1);
				child=new GeneNode(name,lastNode);
				if (lastNode ==NULL) {
					newtree->setRoot(child);
					lastNode=child;
				}
			} else {
				msgBox.setText(QString("Name \"%1\" does not exist in the file at line \"%2\".").arg(nameString).arg(s));
				msgBox.exec();
				newtree->deleteTree();
				return false;
			}
		} else {
			int i=sel->getNameIndex(nameString);
			if (i<0) {
				msgBox.setText(QString("Name \"%1\" does not exist among the genealogical names tagged.").arg(nameString));
				msgBox.exec();
				newtree->deleteTree();
				return false;
			} else {
				child=new GeneNode(Name(sel->text,sel->names[i].first,sel->names[i].second/*-1*/),lastNode);
				if (lastNode ==NULL) {
					newtree->setRoot(child);
					lastNode=child;
				}
			}
		}
		bool finish=false;
		if (!noSpouses) {
			l++;
			do {
				int st=l+1;
				l=s.indexOf('\t',l+1);
				if (l<0) {
					l=s.indexOf(']');
					finish=true;
					if (l<0) {
						msgBox.setText(QString("Syntax error at line \"%1\" was expecting ']'").arg(s));
						msgBox.exec();
						newtree->deleteTree();
						return false;
					}
				}
				QString nameString=Name(&s,st,l-1).getString();
				if (sel==NULL) {
					int pos=tagger->getNameIndexInAll(nameString);
					if (pos>=0) {
						Name name=Name(fileText,pos, pos+nameString.length()-1);
						child->addSpouse(name);
					} else {
						msgBox.setText(QString("Spouse Name \"%1\" does not exist in the file at line \"%2\".").arg(nameString).arg(s));
						msgBox.exec();
						newtree->deleteTree();
						return false;
					}
				} else {
					int i=sel->getNameIndex(nameString);
					if (i<0) {
						msgBox.setText(QString("Spouse Name \"%1\" does not exist among the genealogical names tagged.").arg(nameString));
						msgBox.exec();
						newtree->deleteTree();
						return false;
					} else {
						Name name=Name(sel->text,sel->names[i].first, sel->names[i].second/*-1*/);
						child->addSpouse(name);
					}
				}
			} while (!finish);
		}
		lastIndentation=indentation;
		lastNode=child;
	}
	if (sel==NULL) {
		(*tree)->deleteTree();
		*tree=newtree;
		return true;
	}else if (newtree->getTreeNodesCount(true)==sel->names.size()) {
		sel->tree->deleteTree();
		sel->tree=newtree;
		return true;
	} else {
		msgBox.setText(QString("Nodes in the tree already constructed does not match those available in the genealogical tags"));
		msgBox.exec();
		newtree->deleteTree();
		return false;
	}
}

bool BibleTaggerDialog::Selection::updateGraph(QString text) {
	return updateGraph(text,this);
}
QString BibleTaggerDialog::Selection::getTreeText(GeneTree * tree) {
	QString *out_text=out.string();
	QString s;
	out.setString(&s);
	tree->outputTree();
	out.setString(out_text);
	return s;
}
void BibleTaggerDialog::Selection::setMainInterval(int start,int end) {
	main.first=start;
	main.second=end;
}
void BibleTaggerDialog::Selection::setTree(GeneTree * tree) { //must not be used by tagger just by possibly the parser
	this->tree=tree;
}
void BibleTaggerDialog::Selection::setText(QString *text) { //must not be used by tagger just by possibly the parser
	this->text=text;
}
void BibleTaggerDialog::Selection::clear() {
	names.clear();
	tree=NULL;
}
int BibleTaggerDialog::Selection::mergeNames(QString * text,const MainSelectionList & list1, const MainSelectionList & list2,MainSelectionList & mergedNames){
	mergedNames.clear();
	mergedNames.append(list2);
	for (int i=0;i<list1.size();i++) {
		bool found=false;
		for (int j=0;j<mergedNames.size();j++) {
			QString s1=Name(text,mergedNames[j].first,mergedNames[j].second).getString(),
					s2=Name(text,list1[i].first,list1[i].second).getString();
			if (equalNames(s1,s2) ) {
				found=true;
				break;
			}
		}
		if (!found) {
			mergedNames.append(list1[i]);
		}
	}
	return mergedNames.size();
}


BibleTaggerDialog::BibleTaggerDialog(QString filename):QMainWindow() {
	this->filename=filename;
	text=new QTextBrowser(this);
	tagGenealogy=new QPushButton("Tag &Genealogy",this);
	unTagGenealogy=new QPushButton("&Un-Tag Genealogy",this);
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
	grid->addWidget(tagGenealogy,0,0);
	grid->addWidget(unTagGenealogy,0,1);
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
	connect(tagGenealogy,SIGNAL(clicked()),this,SLOT(tagGene_clicked()));
	connect(unTagGenealogy,SIGNAL(clicked()),this,SLOT(unTagGene_clicked()));
	connect(tagName,SIGNAL(clicked()),this,SLOT(tagName_clicked()));
	connect(unTagName,SIGNAL(clicked()),this,SLOT(unTagName_clicked()));
	connect(save,SIGNAL(clicked()),this,SLOT(save_clicked()));
	connect(text,SIGNAL(selectionChanged()),this, SLOT(text_selectionChanged()));
	connect(modifyGraph,SIGNAL(clicked()),this, SLOT(modifyGraph_clicked()));
	connect(isGlobalGraph,SIGNAL(toggled(bool)),this, SLOT(isGlobalGraph_toggled(bool)));
	connect(resetGlobalGraph,SIGNAL(clicked()),this, SLOT(resetGlobalGraph_clicked()));
	string=NULL;
	globalGraph=NULL;
	displayWindow=new GeneMainWindow();
	displayWindow->showMinimized();
	open_action();
	if (globalGraph==NULL)
		regenerateGlobalGraph();
	setWindowTitle(filename);
	this->resize(700,700);
	selectedTagIndex=-1;
	modifyGraph->setEnabled(false);
}

int BibleTaggerDialog::getNameIndexInAll(const QString & name) {
	for (int i=0;i<tags.size();i++) {
		int j=tags[i].getNameIndex(name);
		if (j>=0)
			return tags[i].getNamesList()[j].first;
	}
	return -1;
}

void BibleTaggerDialog::regenerateGlobalGraph() {
	if (tags.size()>0)
		globalGraph=tags[0].getTree()->duplicateTree();
	for (int i=1;i<tags.size();i++)
		globalGraph->mergeTrees(tags[i].getTree());
	if (globalGraph!=NULL)
		globalGraph->mergeLeftovers();
}

int BibleTaggerDialog::findSubSelection(int tagIndex,int startSubIndex, SelectionMode selectionMode) { //based on user text selection
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

void BibleTaggerDialog::displayGraph(void * tree){
	try{
		system("dot -Tsvg graph.dot -o graph.svg");
		graph->setPixmap(QPixmap("./graph.svg"));
		graphArea->setWidget(graph);
		GeneTree * t=(GeneTree *)tree;
		t->fixSpouseGraphParent();
		displayWindow->display(t,"./graph.svg");
	} catch(...) {}
}

BibleTaggerDialog::~BibleTaggerDialog() {
	for (int i=0;i<tags.size();i++) {
		if (tags[i].tree!=NULL)
			delete tags[i].tree;
	}
	if (globalGraph!=NULL)
		delete globalGraph;
	if (string !=NULL)
		delete string;

	delete isGlobalGraph;
	delete resetGlobalGraph;
	delete tagGenealogy;
	delete unTagGenealogy;
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
	delete displayWindow;
}

void BibleTaggerDialog::tagGenealogy_action() {
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
			GeneTree * duplicate=sel->tree->duplicateTree();
			duplicate->mergeTrees(tags[i].tree);
			Selection::MainSelectionList mergedNames;
			Selection::mergeNames(string,sel->names,tags[i].names,mergedNames);
			int count=duplicate->getTreeNodesCount(true);
			if (count==mergedNames.size()) {
				listForRemoval.append(i);
				sel->names=mergedNames;
				sel->tree->deleteTree();
				tags[i].tree->deleteTree();
				sel->tree=duplicate;
				sel->setMainInterval(min(sel->getMainStart(),tags[i].getMainStart()),max(sel->getMainEnd(),tags[i].getMainEnd()));
				tags.removeAt(i);
				i--;
			} else {
				duplicate->deleteTree(); //stopped before completion i>=0
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

void BibleTaggerDialog::unTagGenealogy_action() {
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

void BibleTaggerDialog::save_action() {
	qSort(tags.begin(),tags.end());
	QFile file(QString("%1.tags").arg(filename).toStdString().data());
	if (file.open(QIODevice::WriteOnly)) {
		QDataStream out(&file);   // we will serialize the data into the file
		out	<< tags
			<<*globalGraph;
		file.close();
		QFile::remove(file.fileName()+".copy");
		file.copy(file.fileName()+".copy");
	} else
		error << "Unexpected Error: Unable to open file\n";
}

void BibleTaggerDialog::open_action() {
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
			QDataStream out(&file);   // we will serialize the data into the file
			out	>> tags;
			if (!out.atEnd()) {
				globalGraph=new GeneTree();
				out	>>*globalGraph;
				FillTextVisitor v(string);
				v(globalGraph);
			}
			file.close();
			for (int i=0;i<tags.size();i++) {
				tags[i].text=string;
				FillTextVisitor v(string);
				v(tags[i].getTree());
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

void BibleTaggerDialog::unTagName_action() {
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

void BibleTaggerDialog::tagName_action() {
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

void BibleTaggerDialog::text_selectionChangedAction() {
	int i=findSelection(0,SELECTION_INSIDE);
	if (selectedTagIndex==i)
		return;
	if (isGlobalGraph->isChecked())
		return;
	updateGraphDisplay();
}

void BibleTaggerDialog::updateGraphDisplay() {
	if (!isGlobalGraph->isChecked()) {
		int i=findSelection(0,SELECTION_INSIDE);
		if (i>=0) {
			treeText->setText(tags[i].getText());
			treeText->setReadOnly(false);
			modifyGraph->setEnabled(true);
			tags[i].getTree()->displayTree(this);
			selectedTagIndex=i;
		} else {
			treeText->setText("");
			treeText->setReadOnly(true);
			modifyGraph->setEnabled(false);
			graph->clear();
			selectedTagIndex=-1;
		}
	} else {
		treeText->setText(Selection::getTreeText(globalGraph));
		treeText->setReadOnly(false);
		modifyGraph->setEnabled(true);
		globalGraph->displayTree(this);
		selectedTagIndex=-1;
	}
}

void BibleTaggerDialog::modifyGraph_action() {
	if (!isGlobalGraph->isChecked()) {
		int i=findSelection(0,SELECTION_INSIDE);
		assert(i>=0);
		if (tags[i].updateGraph(treeText->toPlainText()))
			tags[i].getTree()->displayTree(this);
	} else {
		if (Selection::updateGraph(treeText->toPlainText(),NULL,&globalGraph,string,this))
			globalGraph->displayTree(this);
	}
}

void BibleTaggerDialog::isGlobalGraph_action(){
	updateGraphDisplay();
	resetGlobalGraph->setEnabled(isGlobalGraph->isChecked());
}

int bibleTagger(QString input_str){
	BibleTaggerDialog * d=new BibleTaggerDialog(input_str);
	d->show();
	return 0;
}

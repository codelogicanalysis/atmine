#ifndef BIBLEMANUALTAGGER_H
#define BIBLEMANUALTAGGER_H

#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QPushButton>
#include <QGridLayout>
#include <QTextBrowser>
#include <QScrollArea>
#include "logger.h"
#include <iostream>
#include <QList>
#include <QPair>
#include <QDataStream>
#include <QCheckBox>
#include <QLabel>
#include <QPixmap>

#include "ATMProgressIFC.h"
#include "letters.h"
#include "bibleGeneology.h"

using namespace std;

class BibleTaggerDialog:public QMainWindow, public ATMProgressIFC{
	Q_OBJECT
public:
	enum SelectionMode {SELECTION_OUTSIDE,SELECTION_INSIDE,SELECTION_OUTSIDEOVERLAP};
	inline static bool isConsistentWithSelectionCondidtion(int start, int end, int tagStart,int tagEnd,SelectionMode selectionMode=SELECTION_OUTSIDE) {
		bool con1= selectionMode==SELECTION_OUTSIDE && start<=tagStart && end>=tagEnd,
			 con2= selectionMode==SELECTION_INSIDE && start>=tagStart && end<=tagEnd,
			 con3= selectionMode==SELECTION_OUTSIDEOVERLAP && ((start<=tagStart && end>=tagStart) ||
														(start<=tagEnd && end>=tagEnd));
		return con1 || con2 || con3;
	}
	class Selection {
		friend QDataStream &operator<<(QDataStream &out, const BibleTaggerDialog::Selection &t);
		friend QDataStream &operator>>(QDataStream &in, BibleTaggerDialog::Selection &t);
		friend class BibleTaggerDialog;
	public:
		typedef QPair<int,int> MainSelection;
		typedef QList<MainSelection> MainSelectionList;
	private:
		MainSelection main;
		MainSelectionList names;
		GeneTree * tree;
		QString * text;

	private:
		void updateGraph() {
			if (tree==NULL && names.size()>0) {
				GeneNode * root=new GeneNode(Name(text,names.at(0).first,names.at(0).second),NULL);
				tree=new GeneTree(root);
				for (int i=1;i<names.size();i++) {
					root->addChild(new GeneNode(Name(text,names.at(i).first,names.at(i).second),NULL));
				}
			}
		}
		int getNameIndex(QString s) {
			for (int i=0;i<names.size();i++) {
				if (equal_withoutLastDiacritics(Name(text,names[i].first,names[i].second).getString(),s))
					return i;
			}
			return -1;
		}
		void removeExtraNames() {
			for (int i=0;i<names.size();i++) {
				if (!isConsistentWithSelectionCondidtion(main.first,main.second,names[i].first,names[i].second,SELECTION_OUTSIDE)) {
					removeNameAt(i);
					i--;
				}
			}
		}

	public:
		Selection():main(-1,-1) {
			tree=NULL;
			text=NULL;
		}
		Selection(QString * text,int start,int end):main(start,end) {
			tree=NULL;
			this->text=text;
		}
		void addName(int start,int end) {
			names.append(MainSelection(start,end));
			updateGraph();
			if (names.size()>1) {
				tree->getRoot()->addChild(new GeneNode(Name(text,start,end),NULL));
			}
		}
		void addName( const Name & name) {
			addName(name.getStart(),name.getEnd());
		}
		int getMainStart() { return main.first;}
		int getMainEnd() {return main.second;}
		bool operator <(const Selection & second) const {
			return main<second.main;
		}
		GeneTree * getTree() { return tree;}
		const MainSelectionList & getNamesList() { return names;}
		void removeNameAt(int i) {
			MainSelection s=names[i];
			names.removeAt(i);
			GeneNode * n=tree->findTreeNode(Name(text,s.first,s.second/*-1*/).getString(),true);
			assert(n!=NULL);
			if (n->parent==NULL) { //is root
				assert(tree->getRoot()==n);
				if (n->spouses.size()>0) {
					if (equal_withoutLastDiacritics(n->spouses[0].getString(),n->toString())){
						//TODO: maybe should delete spouse,bc spouse gives same node
					} else {
						n->name=n->spouses[0];
						n->spouses.removeAt(0);
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
					updateGraph();
				}
			} else {
				n->parent->children.removeOne(n);
				for (int i=0;i<n->spouses.size();i++) {
					n->parent->addChild(new GeneNode(n->spouses[i],NULL));
				}
				for (int i=0;i<n->children.size();i++) {
					n->parent->addChild(n->children[i]);
				}
				delete n;
			}
		}
		bool updateGraph(QString text) {
			QStringList lines=text.split("\n",QString::SkipEmptyParts);
			QString s;
			GeneTree * tree=new GeneTree();
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
					tree->deleteTree();
					return false;
				} else {
					int diff=lastIndentation-indentation+1;
					for (int i=0;i<diff;i++) {
						if (lastNode==NULL) {
							tree->deleteTree();
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
				QString name=Name(&s,indentation,l).getString();
				int i=getNameIndex(name);
				if (i<0) {
					tree->deleteTree();
					return false;
				} else {
					child=new GeneNode(Name(this->text,names[i].first,names[i].second/*-1*/),lastNode);
					if (lastNode ==NULL) {
						tree->setRoot(child);
						lastNode=child;
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
								tree->deleteTree();
								return false;
							}
						}
						QString name=Name(&s,st,l-1).getString();
						int i=getNameIndex(name);
						if (i<0) {
							tree->deleteTree();
							return false;
						} else {
							Name name=Name(this->text,names[i].first, names[i].second/*-1*/);
							child->addSpouse(name);
						}
					} while (!finish);
				}
				lastIndentation=indentation;
				lastNode=child;
			}
			if (/*this->tree->getTreeNodesCount(true)==*/tree->getTreeNodesCount(true)==names.size()) {
				this->tree->deleteTree();
				this->tree=tree;
				return true;
			} else {
				tree->deleteTree();
				return false;
			}
		}
		QString getText() {
			QString *out_text=out.string();
			QString s;
			out.setString(&s);
			tree->outputTree();
			out.setString(out_text);
			return s;
		}
		void setMainInterval(int start,int end) {
			main.first=start;
			main.second=end;
		}
		void setTree(GeneTree * tree) { //must not be used by tagger just by possibly the parser
			this->tree=tree;
		}
		void setText(QString *text) { //must not be used by tagger just by possibly the parser
			this->text=text;
		}
		void clear() {
			names.clear();
			tree=NULL;
		}
		static int mergeNames(QString * text,const MainSelectionList & list1, const MainSelectionList & list2,MainSelectionList & mergedNames){
			mergedNames.clear();
			mergedNames.append(list1);
			for (int i=0;i<list2.size();i++) {
				bool found=false;
				for (int j=0;j<mergedNames.size();j++) {
					QString s1=Name(text,mergedNames[j].first,mergedNames[j].second).getString(),
							s2=Name(text,list2[i].first,list2[i].second).getString();
					if (equal_withoutLastDiacritics(s1,s2)) {
						found=true;
						break;
					}
				}
				if (!found)
					mergedNames.append(list2[i]);
			}
			return mergedNames.size();
		}
	};
	typedef QList<Selection> SelectionList;
public:
	BibleTaggerDialog(QString filename):QMainWindow() {
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
		grid->addWidget(text,1,0,3,4);
		grid->addWidget(treeText,1,4,1,2);
		grid->addWidget(modifyGraph,2,4,1,2);
		grid->addWidget(graphArea,3,4,1,2);
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
		grid->addWidget(errors,3,0,1,6);
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
		string=NULL;
		open_action();
		setWindowTitle(filename);
		this->resize(700,700);
		selectedTagIndex=-1;
		modifyGraph->setEnabled(false);
	}

public slots:
	void tagGene_clicked() {
		tagGenealogy_action();
	}
	void unTagGene_clicked() {
		unTagGenealogy_action();
	}
	void tagName_clicked() {
		tagName_action();
	}
	void unTagName_clicked() {
		unTagName_action();
	}
	void save_clicked() {
		save_action();
	}
	void text_selectionChanged() {
		text_selectionChangedAction();
	}
	void modifyGraph_clicked() {
		modifyGraph_action();
	}

private:
	void tagGenealogy_action();
	void unTagGenealogy_action();
	void tagName_action();
	void unTagName_action();
	void save_action();
	void open_action();
	void text_selectionChangedAction();
	void modifyGraph_action();
	int findSelection(int startIndex=0, SelectionMode selectionMode=SELECTION_OUTSIDE) { //based on user text selection
		if (this==NULL)
			return -1;
		QTextCursor c=text->textCursor();
		int start=c.selectionStart();
		int end=c.selectionEnd();
		for (int i=startIndex;i<tags.size();i++) {
			if (isConsistentWithSelectionCondidtion(start,end,tags[i].getMainStart(),tags[i].getMainEnd(),selectionMode)) {
				return i;
			}
		}
		return -1;
	}
	int findSubSelection(int tagIndex,int startSubIndex=0, SelectionMode selectionMode=SELECTION_OUTSIDE) { //based on user text selection
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
	void moveSelectionToWordBoundaries() {
		QTextCursor c=text->textCursor();
		int start=c.selectionStart();
		int end=c.selectionEnd();
		QChar chr=(c.selectedText().length()>0?c.selectedText().at(0):'\0');
		if (isDelimiterOrNumber(chr)) {
			while (isDelimiterOrNumber(chr)) {
				c.setPosition(++start,QTextCursor::MoveAnchor);
				c.setPosition(end,QTextCursor::KeepAnchor);
				if (start==end)
					return;
				chr=c.selectedText().at(0);
			}
		} else {
			if (start>0) {
				while (!isDelimiterOrNumber(chr)) {
					c.setPosition(--start,QTextCursor::MoveAnchor);
					c.setPosition(end,QTextCursor::KeepAnchor);
					if (start==0) {
						start--;
						break;
					}
					chr=c.selectedText().at(0);
				}
				start++;
			}
		}
		chr=(c.selectedText()>0?c.selectedText().at(c.selectedText().length()-1):'\0');
		if (isDelimiterOrNumber(chr)) {
			while (isDelimiterOrNumber(chr)) {
				c.setPosition(start,QTextCursor::MoveAnchor);
				c.setPosition(--end,QTextCursor::KeepAnchor);
				if (c.selectedText().length()==0)
					return;
				chr=c.selectedText().at(c.selectedText().length()-1);
			}
		} else {
			if (text->toPlainText().length()>end) {
				while (!isDelimiterOrNumber(chr)) {
					c.setPosition(start,QTextCursor::MoveAnchor);
					c.setPosition(++end,QTextCursor::KeepAnchor);
					if (text->toPlainText().length()==end) {
						end++;
						break;
					}
					chr=c.selectedText().at(c.selectedText().length()-1);
				}
				end--;
			}
		}
		c.setPosition(start,QTextCursor::MoveAnchor);
		c.setPosition(end,QTextCursor::KeepAnchor);
		text->setTextCursor(c);
	}
	void moveSelectionToSentenceBoundaries() {
		moveSelectionToWordBoundaries();
		QTextCursor c=text->textCursor();
		int start=c.selectionStart();
		int end=c.selectionEnd();
		if (start==end)
			return;
		QChar chr=c.selectedText().at(c.selectedText().length()-1);
		assert (!isPunctuationMark(chr));
		if (text->toPlainText().length()>end) {
			while (!isPunctuationMark(chr)) {
				c.setPosition(start,QTextCursor::MoveAnchor);
				c.setPosition(++end,QTextCursor::KeepAnchor);
				chr=c.selectedText().at(c.selectedText().length()-1);
				if (text->toPlainText().length()==end) {
					break;
				}
			}
		}
		chr=c.selectedText().at(0);
		assert (!isPunctuationMark(chr));
		if (start>0) {
			while (!isPunctuationMark(chr)) {
				c.setPosition(--start,QTextCursor::MoveAnchor);
				c.setPosition(end,QTextCursor::KeepAnchor);
				chr=c.selectedText().at(0);
				if (start==0) {
					break;
				}
			}
		}
		while (isDelimiter(chr)) {
			c.setPosition(++start,QTextCursor::MoveAnchor);
			c.setPosition(end,QTextCursor::KeepAnchor);
			chr=c.selectedText().at(0);
		}
		c.setPosition(start,QTextCursor::MoveAnchor);
		c.setPosition(end,QTextCursor::KeepAnchor);
		text->setTextCursor(c);
	}
	void updateGraphDisplay();

public:
	SelectionList tags;

	QString filename, * string;
	QPushButton * tagGenealogy, *unTagGenealogy, *save, *tagName, *unTagName, *modifyGraph;
	QTextBrowser * text, * treeText;
	QCheckBox * forceWordNames;
	QScrollArea * scrollArea, * graphArea;
	QLabel * graph;
#ifdef ERRORS_BIBLE
	QTextBrowser * errors;
	QString * errors_text;
#endif
	QGridLayout * grid;
	int selectedTagIndex;

public:
	void report(int) {}
	void startTaggingText(QString &) {}
	void tag(int , int , QColor , bool ) {} //not used here
	void finishTaggingText(){}
	void setCurrentAction(const QString &) {}
	void resetActionDisplay() {}
	void displayGraph(){
		try{
			system("dot -Tsvg graph.dot -o graph.svg");
			graph->setPixmap(QPixmap("./graph.svg"));
			graphArea->setWidget(graph);
		}
		catch(...)
		{}
	}
	QString getFileName() {
		return filename;
	}

	~BibleTaggerDialog() {
		if (string !=NULL)
			delete string;
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
	}
};

inline QDataStream &operator<<(QDataStream &out, const BibleTaggerDialog::Selection &t) {
	out<<t.main<<t.names<<*t.tree;
	return out;
}

inline QDataStream &operator>>(QDataStream &in, BibleTaggerDialog::Selection &t) {
	t.tree=new GeneTree();
	in>>t.main >> t.names>>*t.tree;
	return in;
}



#endif // BIBLEMANUALTAGGER_H

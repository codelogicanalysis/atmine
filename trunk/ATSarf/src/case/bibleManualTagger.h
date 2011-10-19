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
#include <iostream>
#include <QList>
#include <QPair>
#include <QDataStream>
#include <QCheckBox>
#include <QLabel>
#include <QPixmap>
#include <QMessageBox>

#include "logger.h"
#include "ATMProgressIFC.h"
#include "letters.h"
#include "bibleGeneology.h"
#include "abstractAnnotator.h"


class GeneMainWindow;

class BibleTaggerDialog:public QMainWindow, public ATMProgressIFC,public AbstractAnnotator{
	Q_OBJECT
public:
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
				if (equalNames(Name(text,names[i].first,names[i].second).getString(),s))
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
		int getMainStart() const { return main.first;}
		int getMainEnd() const {return main.second;}
		bool operator <(const Selection & second) const {
			return main<second.main;
		}
		GeneTree * getTree() { return tree;}
		const MainSelectionList & getNamesList() { return names;}
		void removeNameAt(int i) {
			MainSelection s=names[i];
			names.removeAt(i);
			QString name=Name(text,s.first,s.second/*-1*/).getString();
			GeneNode * n=tree->findTreeNode(name,true);
			assert(n!=NULL);
			if (n->parent==NULL) { //is root
				assert(tree->getRoot()==n);
				if (n->spouses.size()>0) {
					if (equalNames(n->toString(),name)) {
						n->name=n->spouses[0];
						n->spouses.removeAt(0);
					} else {
						for (int i=0;i<n->spouses.size();i++) {
							if (equalNames(n->spouses[i].getString(),n->toString())){
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
		static bool updateGraph(QString text, Selection * sel,GeneTree ** tree=NULL, QString *fileText=NULL,BibleTaggerDialog * tagger=NULL) {
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

		bool updateGraph(QString text) {
			return updateGraph(text,this);
		}
		static QString getTreeText(GeneTree * tree) {
			QString *out_text=out.string();
			QString s;
			out.setString(&s);
			tree->outputTree();
			out.setString(out_text);
			return s;
		}
		QString getText() {
			return getTreeText(tree);
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
	};
	typedef QList<Selection> SelectionList;
public:
	BibleTaggerDialog(QString filename);

private slots:
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
	void isGlobalGraph_toggled(bool ) {
		isGlobalGraph_action();
	}
	void resetGlobalGraph_clicked() {
		regenerateGlobalGraph();
		globalGraph->displayTree(this);
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
	void isGlobalGraph_action();
	int getNameIndexInAll(const QString & name);
	void regenerateGlobalGraph();
	int findSubSelection(int tagIndex,int startSubIndex=0, SelectionMode selectionMode=SELECTION_OUTSIDE) ;
	void updateGraphDisplay();

public:
	SelectionList tags;
	GeneTree * globalGraph;

	QString filename, * string;
	QPushButton * tagGenealogy, *unTagGenealogy, *save, *tagName, *unTagName, *modifyGraph,* isGlobalGraph,*resetGlobalGraph;
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
	GeneMainWindow * displayWindow;

public:
	void report(int) {}
	void startTaggingText(QString &) {}
	void tag(int , int , QColor , bool ) {} //not used here
	void finishTaggingText(){}
	void setCurrentAction(const QString &) {}
	void resetActionDisplay() {}
	void displayGraph(void * tree);
	QString getFileName() {	return filename; }
	~BibleTaggerDialog();

	QTextBrowser * getTextBrowser() {return text;}
	int getTagCount() const { return tags.size();}
	int getTagStart(int tagNum) const {return tags.at(tagNum).getMainStart();}
	int getTagEnd(int tagNum) const {return tags.at(tagNum).getMainEnd();}
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

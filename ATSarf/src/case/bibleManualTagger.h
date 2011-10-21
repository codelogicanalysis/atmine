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
		void FillNullTree();
		int getNameIndex(QString s);
		void removeExtraNames();

	public:
		Selection();
		Selection(QString * text,int start,int end);
		void addName(int start,int end);
		void addName( const Name & name);
		int getMainStart() const { return main.first;}
		int getMainEnd() const {return main.second;}
		bool operator <(const Selection & second) const {return main<second.main;}
		GeneTree * getTree() { return tree;}
		const MainSelectionList & getNamesList() { return names;}
		void removeNameAt(int i);
		static bool updateGraph(QString text, Selection * sel,GeneTree ** tree=NULL, QString *fileText=NULL,BibleTaggerDialog * tagger=NULL);
		bool updateGraph(QString text);
		static QString getTreeText(GeneTree * tree);
		QString getText() {	return getTreeText(tree);	}
		void setMainInterval(int start,int end);
		void setTree(GeneTree * tree);
		void setText(QString *text);
		void clear();
		static int mergeNames(QString * text,const MainSelectionList & list1, const MainSelectionList & list2,MainSelectionList & mergedNames);
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

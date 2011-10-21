#ifndef ABSTRACTTWOLEVELANNOTATOR_H
#define ABSTRACTTWOLEVELANNOTATOR_H

#include <QMainWindow>
#include "abstractAnnotator.h"
#include "ATMProgressIFC.h"
#include "twoLevelTaggerSelection.h"

class Name;
class QPushButton;
class QCheckBox;
class QScrollArea;
class QLabel;
class QGridLayout;

class AbstractTwoLevelAnnotator:public QMainWindow, public ATMProgressIFC,public AbstractAnnotator{
	Q_OBJECT
public:
	typedef QList<Selection> SelectionList;
public:
	AbstractTwoLevelAnnotator(QString filename, QString mainStructure);

private slots:
	void tagMain_clicked() {
		tagMain_action();
	}
	void unTagMain_clicked() {
		unTagMain_action();
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
		displayGraph(globalGraph);
	}

private:
	void tagMain_action();
	void unTagMain_action();
	void tagName_action();
	void unTagName_action();
	void save_action();
	void open_action();
	void text_selectionChangedAction();
	void modifyGraph_action();
	void isGlobalGraph_action();
	int getNameIndexInAll(const QString & name);
	virtual void regenerateGlobalGraph()=0;
	int findSubSelection(int tagIndex,int startSubIndex=0, SelectionMode selectionMode=SELECTION_OUTSIDE) ;
	void updateGraphDisplay();

public:
	SelectionList tags;
	AbstractGraph * globalGraph;

	QString filename, * string;
	QPushButton * tagMain, *unTagMain, *save, *tagName, *unTagName, *modifyGraph,* isGlobalGraph,*resetGlobalGraph;
	QTextBrowser * text, * treeText;
	QCheckBox * forceWordNames;
	QScrollArea * scrollArea, * graphArea;
	QLabel * graph;
#ifdef ERRORS_ANNOTATOR
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
	void displayGraph(void * tree)=0;
	QString getFileName() {	return filename; }
	~AbstractTwoLevelAnnotator();

	QTextBrowser * getTextBrowser() {return text;}
	int getTagCount() const { return tags.size();}
	int getTagStart(int tagNum) const {return tags.at(tagNum).getMainStart();}
	int getTagEnd(int tagNum) const {return tags.at(tagNum).getMainEnd();}
};


#endif // ABSTRACTTWOLEVELANNOTATOR_H

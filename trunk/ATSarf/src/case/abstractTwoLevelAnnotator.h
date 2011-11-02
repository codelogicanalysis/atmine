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
class QTreeView;
class QModelIndex;

class AbstractTwoLevelAnnotator:public QMainWindow, public ATMProgressIFC,public AbstractAnnotator{
	Q_OBJECT
public:
	class SelectionList:public QList<TwoLevelSelection> {
	public:
		void readFromStream(QDataStream & in,AbstractGraph * graph);
	};
public:
	AbstractTwoLevelAnnotator(QString filename, QString mainStructure,QString ext=".tags");
	void show();

	int getNameIndexInAll(const QString & name);
	int findSubSelection(int tagIndex,int startSubIndex=0, SelectionMode selectionMode=SELECTION_OUTSIDE) ;

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
		updateGraphDisplay();
	}

protected:
	void tagMain_action();
	void unTagMain_action();
	void tagName_action();
	void unTagName_action();
	void text_selectionChangedAction();
	void modifyGraph_action();
	void isGlobalGraph_action();
	void updateGraphDisplay();
	void refreshTreeModel(QAbstractItemModel * model);
	virtual bool save_action();
	virtual bool open_action();
	virtual void createToolbar();
	virtual void createDocWindows();
	virtual void createMenus();
	virtual void createActions(QString mainStructure);

	virtual void modifiedLocalGraph(){}
	virtual bool mergeMainStructures(TwoLevelSelection * oldSelection, int newTagIndex);
	virtual void regenerateGlobalGraph()=0;
	virtual AbstractGraph * newGraph(bool global=false)=0;
public:
	QTextBrowser * text, * treeText;
	QLabel * graph;
	QToolBar * annotationToolbar, * graphToolbar;
	QAction * forcePunctuationMainBoundaryAct,* tagMainAct, * unTagMainAct, *forceWordNamesAct, * tagNameAct, * unTagNameAct, * saveAct,
			* modifyGraphAct, * globalGraphAct, * resetGlobalGraphAct;
	QMenu *viewMenu;
	QScrollArea * graphArea;
	int selectedTagIndex;
	SelectionList tags;
	AbstractGraph * globalGraph;
	QString filename,ext,mainStructure, * string;
	QAbstractItemModel * treeModel;
	QTreeView *resultTree;

public:
	void report(int) {}
	void startTaggingText(QString &) {}
	void tag(int , int , QColor , bool ) {} //not used here
	void finishTaggingText(){}
	void setCurrentAction(const QString &) {}
	void resetActionDisplay() {}
	QString getFileName() {	return filename; }
	virtual void displayGraph(AbstractGraph * graph);
	virtual ~AbstractTwoLevelAnnotator();

	QTextBrowser * getTextBrowser() {return text;}
	int getTagCount() const { return tags.size();}
	int getTagStart(int tagNum) const {return tags.at(tagNum).getMainStart();}
	int getTagEnd(int tagNum) const {return tags.at(tagNum).getMainEnd();}
};


#endif // ABSTRACTTWOLEVELANNOTATOR_H

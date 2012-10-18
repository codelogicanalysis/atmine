#ifndef HADITHMANUALTAGGER_H
#define HADITHMANUALTAGGER_H

#include <QModelIndex>
#include <QPoint>
#include "abstractTwoLevelAnnotator.h"
#include "narratorEqualityModel.h"

class NarratorNodeIfc;

class QTableView;

class HadithTaggerDialog : public AbstractTwoLevelAnnotator
{
	Q_OBJECT
private:
	QModelIndex * resultTreeMenuMIdxMain;
	QModelIndex * resultTreeMenuMIdxApplied;
	QMenu * resultTreeMainMenu;
	QMenu * resultTreeOperationMenu;
	QAction * colorSelectedNodeInGraphAct;
	bool isMergeAll:1,chosenAction:1;

	//narrators equality:

private:
	typedef NarratorEqualityModel::NarratorMap NarratorMap;

	QTableView * equalityMatrixTable;
	QToolBar * narratorEqualityToolBar;
	QTextEdit * numberExtraNarratorPairs;
	QAction *resetOneEqualityAct, * resetAllEqualityAct, *addEqualityPairsAct, *narratorsEqualAct, *saveAct;
	NarratorEqualityModel * equalityTableModel;
	NarratorMap map;
protected:
	virtual void createActions(QString mainStructure);
	virtual void createDocWindows();
	virtual void createToolbar();
	virtual void createMenus();
	virtual bool open_action();
	virtual bool save_action();

	QModelIndex getEqualitySelectedIndex();

private slots:
	void resultTree_clicked ( const QModelIndex & index );
	void resultTree_contextMenu(const QPoint & point);
	void mergeIndividual();
	void mergeAll();
	void unMerge();
	void applyMerge();
	void cancelMerge();

	void equalNarrator_clicked();
	void resetOneEquality_clicked();
	void resetAllEqualities_clicked();
	void addEqualityEntries_clicked();
	void tableEquality_changedSelection(const QModelIndex & current,const QModelIndex & previous);
private:
	virtual void modifiedLocalGraph();
	virtual void regenerateGlobalGraph();
	virtual AbstractGraph * newGraph(bool global);
	virtual bool mergeMainStructures(TwoLevelSelection * , int ) {return false; } //not supported yet

	void clearCacheFroUpdatedNode(NarratorNodeIfc * node);
	void refreshEqualityTableModel();
public:
	HadithTaggerDialog(QString fileName);
	virtual ~HadithTaggerDialog();
};

#endif // HADITHMANUALTAGGER_H

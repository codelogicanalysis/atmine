#ifndef HADITHMANUALTAGGER_H
#define HADITHMANUALTAGGER_H

#include <QModelIndex>
#include <QPoint>
#include "abstractTwoLevelAnnotator.h"

class NarratorNodeIfc;


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

private slots:
	void resultTree_clicked ( const QModelIndex & index );
	void resultTree_contextMenu(const QPoint & point);
	void mergeIndividual();
	void mergeAll();
	void unMerge();
	void applyMerge();
	void cancelMerge();
private:
	virtual void regenerateGlobalGraph();
	virtual AbstractGraph * newGraph(bool global);
	virtual bool mergeMainStructures(TwoLevelSelection * , int ) {return false; } //not supported yet

	void clearCacheFroUpdatedNode(NarratorNodeIfc * node);
public:
	HadithTaggerDialog(QString fileName);
	virtual ~HadithTaggerDialog();
};

#endif // HADITHMANUALTAGGER_H

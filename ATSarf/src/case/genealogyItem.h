#ifndef GENEALOGYITEM_H
#define GENEALOGYITEM_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "bibleGeneology.h"

using namespace std;

class QTreeView;


class GeneItemModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	static const int COL_NAME, COL_RELATION, COL_EDGE, COL_CHILD_COUNT, COL_GENDER, COL_HEIGHT, COLUMNS;
public:
	GeneTree * tree;
public:
	GeneItemModel (GeneTree * aTree) :tree(aTree) {    }
	QVariant headerData(int section, Qt::Orientation orientation,
	int role = Qt::DisplayRole) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const; //number of children
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QModelIndex index(int row, int column,
	const QModelIndex &parent = QModelIndex()) const;

};

#endif // GENEALOGYITEM_H

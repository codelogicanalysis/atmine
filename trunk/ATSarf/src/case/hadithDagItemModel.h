#ifndef HADITHDAGITEMMODEL_H
#define HADITHDAGITEMMODEL_H

#include <QAbstractItemModel>
#include <QList>

class HadithDagGraph;
class NarratorNodeIfc;

class HadithDagItemModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	static const int COL_NARRATOR, COL_CHILDREN, COLUMNS;
public:
	QList<NarratorNodeIfc *> nodes;
public:
	HadithDagItemModel (HadithDagGraph * graph);
	QVariant headerData(int section, Qt::Orientation orientation,
	int role = Qt::DisplayRole) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const; //number of children
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QModelIndex index(int row, int column,
	const QModelIndex &parent = QModelIndex()) const;

public:
	HadithDagItemModel();
};

#endif // HADITHDAGITEMMODEL_H

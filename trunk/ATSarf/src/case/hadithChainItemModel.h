#ifndef HADITHCHAINITEMMODEL_H
#define HADITHCHAINITEMMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class HadithChainGraph;

class HadithChainItemModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	static const int COL_NARRATOR, COL_HEIGHT, COLUMNS;
public:
	HadithChainGraph * graph;
public:
	HadithChainItemModel (HadithChainGraph * aGraph) :graph(aGraph) {    }
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
    HadithChainItemModel();
};

#endif // HADITHCHAINITEMMODEL_H

#ifndef NARRATOREQUALITYMODEL_H
#define NARRATOREQUALITYMODEL_H

#include <QAbstractTableModel>

#include<QPair>

class NarratorEqualityModel : public QAbstractTableModel
{
public:
	static const int COL_1, COL_2, COL_EQUALITY, COLUMNS;

	typedef QPair<QString,QString> NarratorPair;
	typedef QMap<NarratorPair, bool> NarratorMap;
private:
	NarratorMap & map;
public:
	NarratorEqualityModel(NarratorMap & equalityMap):map(equalityMap) {};
	virtual int rowCount(const QModelIndex &parent) const;
	virtual int columnCount(const QModelIndex &parent) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value,int role = Qt::EditRole);
	virtual Qt::ItemFlags flags(const QModelIndex &index) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
};

#endif // NARRATOREQUALITYMODEL_H

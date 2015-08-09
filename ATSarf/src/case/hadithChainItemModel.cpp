#include "hadithChainItemModel.h"
#include "hadithChainGraph.h"

const int	HadithChainItemModel::COL_NARRATOR=0,
			HadithChainItemModel::COL_HEIGHT=1,
			HadithChainItemModel::COLUMNS=2;


QVariant HadithChainItemModel::headerData(int section, Qt::Orientation orientation, int role ) const{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch(section) {
		case COL_NARRATOR:
			return QVariant("Narrator");
		case COL_HEIGHT:
			return QVariant("Height");
		};
	}
	return QVariant();
}

int HadithChainItemModel::columnCount(const QModelIndex &/*parent*/ ) const{
	return COLUMNS;
}

int HadithChainItemModel::rowCount(const QModelIndex &parent ) const {
	if (!parent.isValid())
		return 1;
	int id=parent.internalId();
	if (id==graph->chain.m_chain.size()-1)
		return 0;
	return 1;
}

QModelIndex HadithChainItemModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();
	int i=index.internalId()-1;
	assert(i<graph->chain.m_chain.size());
	if (i>=0)
		return createIndex(0, 0,i);
	else
		return QModelIndex();
}

Qt::ItemFlags HadithChainItemModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


QVariant HadithChainItemModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();
	if (index.column() > COLUMNS)
		return QVariant();

	int i=index.internalId();
	assert(i>=0);
	Narrator * narr=dynamic_cast<Narrator* >(graph->chain.m_chain[i]);
	switch(index.column()) {
	case COL_NARRATOR:
		return QVariant(narr->getString());
		break;
	case COL_HEIGHT:
		return graph->chain.m_chain.size()-i-1;
		break;
	}
	return QVariant();
}

QModelIndex HadithChainItemModel::index(int row, int column, const QModelIndex &parent ) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	if (column > COLUMNS)
		return QModelIndex();

	if (!parent.isValid()) {
		if (graph->chain.m_chain.size()>0)
			return createIndex(0, column, quintptr(0));
		else
			return QModelIndex();
	}
	int parentId=parent.internalId();
	int id=parentId+1;
	if (id<graph->chain.m_chain.size() && id>=0)
		return createIndex(row, column,id);
	return QModelIndex();
}


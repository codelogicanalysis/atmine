#include "hadithDagItemModel.h"
#include "hadithDagGraph.h"

const int	HadithDagItemModel::COL_NARRATOR=0,
			HadithDagItemModel::COL_CHILDREN=1,
			HadithDagItemModel::COLUMNS=2;

HadithDagItemModel::HadithDagItemModel(HadithDagGraph *graph) {
	for (int i=0;i<graph->graph->all_nodes.size();i++) {
		NarratorNodeIfc * node=graph->graph->all_nodes[i];
		if (node !=NULL && node->isActualNode()) {
			nodes.append(node);
		}
	}
}

QVariant HadithDagItemModel::headerData(int section, Qt::Orientation orientation, int role ) const{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		switch(section) {
		case COL_NARRATOR:
			return QVariant("Narrator");
		case COL_CHILDREN:
			return QVariant("Children");
		};
	}
	return QVariant();
}

int HadithDagItemModel::columnCount(const QModelIndex &/*parent*/ ) const{
	return COLUMNS;
}

int HadithDagItemModel::rowCount(const QModelIndex &parent ) const {
	if (!parent.isValid())
		return nodes.size();
	NarratorNodeIfc * node=(NarratorNodeIfc*)parent.internalPointer();
	if (node->isChainNode())
		return 0; //no merged Nodes
	NodeIterator itr=node->begin();
	int count=0;
	for (;!itr.isFinished();++itr) {
		count++;
	}
	return count;
}

QModelIndex HadithDagItemModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();
	NarratorNodeIfc * node=(NarratorNodeIfc *)index.internalPointer();
	if (node->isActualNode()) {
		return QModelIndex();
	}
	NarratorNodeIfc * nodeParent=&node->getCorrespondingNarratorNode();
	int i=nodes.indexOf(nodeParent);
	assert(i>=0);
	return createIndex(i, 0,nodeParent);
}

Qt::ItemFlags HadithDagItemModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


QVariant HadithDagItemModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();
	if (index.column() > COLUMNS)
		return QVariant();

	NarratorNodeIfc * node=(NarratorNodeIfc *)index.internalPointer();
	switch(index.column()) {
	case COL_NARRATOR:
		return QVariant(node->CanonicalName());
		break;
	case COL_CHILDREN:
		if (node->isGraphNode()) {
			NodeIterator itr=node->begin();
			int count=0;
			for (;!itr.isFinished();++itr) {
				count++;
			}
			return QVariant(count);
		} else
			return QVariant(0);
		break;
	}
	return QVariant();
}

QModelIndex HadithDagItemModel::index(int row, int column, const QModelIndex &parent ) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	if (column > COLUMNS)
		return QModelIndex();

	if (!parent.isValid()) {
		if (0<=row && row<nodes.size())
			return createIndex(row, column,nodes[row]);
		else
			return QModelIndex();
	}
	NarratorNodeIfc * parentNode=(NarratorNodeIfc *)parent.internalPointer();
	if (parentNode->isGraphNode()) {
		NodeIterator itr=parentNode->begin();
		int count=0;
		for (;!itr.isFinished();++itr) {
			if (count==row) {
				NarratorNodeIfc * node=itr.getNode();
				return createIndex(row,column,node);
			}
			count++;
		}
	}
	return QModelIndex();
}


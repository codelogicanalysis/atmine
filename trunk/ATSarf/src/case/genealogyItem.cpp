#include "genealogyItem.h"

const int	GeneItemModel::COL_NAME=0,
			GeneItemModel::COL_RELATION=2,
			GeneItemModel::COL_EDGE=5,
			GeneItemModel::COL_CHILD_COUNT=3,
			GeneItemModel::COL_SEX=1,
			GeneItemModel::COL_HEIGHT=4,
			GeneItemModel::COLUMNS=6;


QVariant GeneItemModel::headerData(int section, Qt::Orientation orientation, int role ) const{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch(section) {
		case COL_NAME:
			return QVariant("Name");
		case COL_RELATION:
			return QVariant("Inferred");
		case COL_EDGE:
			return QVariant("Edge Label");
		case COL_CHILD_COUNT:
			return QVariant("Children");
		case COL_SEX:
			return QVariant("Sex");
		case COL_HEIGHT:
			return QVariant("Height");
		};
	}
	return QVariant();
}

int GeneItemModel::columnCount(const QModelIndex &/*parent*/ ) const{
	return COLUMNS;
}

int
GeneItemModel::rowCount(const QModelIndex &parent ) const {
	if (!parent.isValid())
		return 1;
	AbstractGeneNode * node = (AbstractGeneNode *)parent.internalId();
	if (node == NULL) {
		return 0;
	}
	return node->getDirectGraphChildrenCount();
}

QModelIndex GeneItemModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();
	AbstractGeneNode * node = (AbstractGeneNode*)index.internalId();
	if (node == NULL){
		return QModelIndex();
	}
	AbstractGeneNode * parent = node->getParent();
	int i=0;
	if (parent != NULL) {
		GeneNode * ancestor = dynamic_cast<GeneNode*>(parent->getParent());
		if (ancestor != NULL) {
			int j = 0;
			//for (;j < ancestor->spouses.size(); j++) // can never happen
			for (;j < ancestor->children.size(); j++) {
				if (ancestor->children[j]==parent) {
					i=j+ancestor->spouses.size();
					break;
				}
			}
			assert (j<ancestor->children.size());
		}
	}
	void * p=(void *)parent;
	if (p==NULL)
		return QModelIndex();
	return createIndex(i, 0, p);
}

Qt::ItemFlags GeneItemModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


QVariant GeneItemModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();
	if (index.column() > COLUMNS)
		return QVariant();

	AbstractGeneNode * node = (AbstractGeneNode*)index.internalId();
	if (node == NULL)
		return QVariant();
	switch(index.column()) {
	case COL_NAME:
		return QVariant(node->getString());
		break;
	case COL_RELATION:
		if (node->isName())//TODO
			return QVariant("Spouse");
		else if (node->getParent()!=NULL)
			return QVariant("Child");
		else
			return QVariant();
		break;
	case COL_EDGE:
		return QVariant(node->getEdgeText());
		break;
	case COL_CHILD_COUNT:
		return node->getDescendentCount();
		break;
	case COL_SEX:
		return QVariant(node->getSex());
		break;
	case COL_HEIGHT:
		return node->getGraphHeight();
		break;
	}
	return QVariant();
}

QModelIndex GeneItemModel::index(int row, int column,
		const QModelIndex &parent ) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	if (column > COLUMNS)
		return QModelIndex();

	if (!parent.isValid()) {
		void * p = (void*) tree->getRoot();
		if (p!=NULL)
			return createIndex(row, column,p );
		else
			return QModelIndex();
	}

	GeneNode * parentNode = dynamic_cast<GeneNode*>((AbstractGeneNode*)parent.internalId());
	if (row<parentNode->spouses.size()) {
		void * p=(void *)&parentNode->spouses[row];
		return createIndex(row, column, p);
	} else if (row<parentNode->spouses.size()+parentNode->children.size()) {
		void * p=(void *)parentNode->children[row-parentNode->spouses.size()];
		return createIndex(row, column, p);
	} else {
		return QModelIndex();
	}
	return QModelIndex();
}

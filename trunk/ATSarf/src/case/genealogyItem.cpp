#include "genealogyItem.h"

QVariant GeneItemModel::headerData(int section, Qt::Orientation orientation, int role ) const{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		if (section == 0)
			return QVariant("Name");
		if (section == 1)
			return QVariant("Inferred");
		if (section == 2)
			return QVariant("Edge Label");
		if (section == 3)
			return QVariant("Children Count");
	}
	return QVariant();
}

int GeneItemModel::columnCount(const QModelIndex &parent ) const{
	return 4;
}

int
GeneItemModel::rowCount(const QModelIndex &parent ) const {
	if (!parent.isValid())
		return 1;
	GeneNode * node = (GeneNode *)parent.internalId();
	if (node == NULL) {
		// this means that it is a pre/post or behavior that does not exist!
		return 0;
	}
	return node->children.size()+node->spouses.size();
}

QModelIndex GeneItemModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();
	GeneNode * node = (GeneNode*)index.internalId();
	if (node == NULL){
		return QModelIndex();
	}
	GeneNode * parent = node->parent;
	int i=0;
	if (parent != NULL) {
		GeneNode * ancestor = parent->parent;
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
	if (index.column() > 3)
		return QVariant();

	GeneNode * node = (GeneNode*)index.internalId();
	if (node == NULL)
		return QVariant();
	switch(index.column()) {
	case 0:
		return QVariant(node->name.getString());
		break;
	case 1:
		if (true/*node->isSpouse()*/)//TODO
			return QVariant("Spouse");
		else
			return QVariant("Child");
		break;
	case 2:
		return QVariant(node->name.edgeText);
		break;
	case 3:
		return node->children.size()+node->spouses.size();
		break;
	}
	return QVariant();
}

QModelIndex GeneItemModel::index(int row, int column,
		const QModelIndex &parent ) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	if (column >= 3)
		return QModelIndex();

	if (!parent.isValid()) {
		void * p = (void*) tree->getRoot();
		if (p!=NULL)
			return createIndex(row, column,p );
		else
			return QModelIndex();
	}

	GeneNode * parentNode = (GeneNode*)parent.internalId();
#if 0
	if (node == NULL) {
		if (row < cSpecMW.stMap.size()) {
			void * p = (void*) getSpecTriple(row);
			return createIndex(row, column, p);
		}
		return QModelIndex();
	}
#endif
	if (row<parentNode->spouses.size()) {
		void * p=(void *)NULL/*parentNode->spouses[row]*/; //TODO
		return createIndex(row, column, p);
	} else if (row<parentNode->spouses.size()+parentNode->children.size()) {
		void * p=(void *)parentNode->children[row-parentNode->spouses.size()];
		return createIndex(row, column, p);
	} else {
		return QModelIndex();
	}
	return QModelIndex();
}

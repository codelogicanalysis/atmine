#include "narratorEqualityModel.h"
#include <QCheckBox>

const int	NarratorEqualityModel::COL_1=0,
			NarratorEqualityModel::COL_2=1,
			NarratorEqualityModel::COL_EQUALITY=2,
			NarratorEqualityModel::COLUMNS=3;

int NarratorEqualityModel::rowCount(const QModelIndex &parent) const {
	/*if (parent.isValid())
		return 0;*/
	int s=map.size();
	return s;
}
int NarratorEqualityModel::columnCount(const QModelIndex &parent) const {
	return COLUMNS;
}

QVariant NarratorEqualityModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();
	if (index.column() > COLUMNS)
		return QVariant();
	int row=index.row();
	NarratorMap::iterator itr=map.begin()+row;
	switch(index.column()) {
	case COL_1:
		return QVariant(itr.key().first);
		break;
	case COL_2:
		return QVariant(itr.key().second);
		break;
	case COL_EQUALITY:
		bool v=*itr;
		return QVariant(v);
		break;
	}
	return QVariant();

}

Qt::ItemFlags NarratorEqualityModel::flags(const QModelIndex &index) const {
	if (!index.isValid())
		return 0;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
QVariant NarratorEqualityModel::headerData(int section, Qt::Orientation orientation, int role) const{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		switch(section) {
		case COL_1:
			return QVariant("Narrator 1");
		case COL_2:
			return QVariant("Narrator 2");
		case COL_EQUALITY:
			return QVariant("Equal?");
		};
	}
	return QVariant();
}

bool NarratorEqualityModel::setData(const QModelIndex &index, const QVariant &value, int role) {
	if (!index.isValid())
		return false;
	if (index.column()!=COL_EQUALITY)
		return false;
	bool v=value.toBool();
	int row=index.row();
	NarratorMap::iterator itr=map.begin()+row;
	bool & b=*itr;
	b=v;
	reset();
	return true;
}


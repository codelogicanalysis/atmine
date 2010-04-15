#ifndef STEMNODE_H
#define STEMNODE_H

#include <QString>
#include <QList>
#include <QVector>
#include "common_structures/common.h"

class StemNode {
public:
	typedef QVector<long> CategoryVector;

	QString key;
	unsigned long long stem_id;
private:
	CategoryVector category_ids;
public:
	void add_info(long cat_id);
	bool exists(long cat_id);
	#ifdef REDUCE_THRU_DIACRITICS
	public:
		typedef QList<QString> RawDatasEntry;
		typedef QVector<RawDatasEntry> RawDatas;

		void add_info(long cat_id,QString raw_data);
		void add_info(long cat_id,RawDatasEntry raw_data_entry);
		bool exists(long cat_id, QString raw_data);
		bool get(long cat_id, RawDatasEntry &raw_datas_entry);
	public://private:
		RawDatas raw_datas;
	#endif
};

#endif // STEMNODE_H

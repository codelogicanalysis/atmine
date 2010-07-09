#ifndef ITEM_STRUCTURES_H
#define ITEM_STRUCTURES_H
#if 0
#include "common.h"
#include <QVector>
#include <QString>

class Item
{
public:
	typedef QVector<QString> StringVector;

	item_types type;
	QString key;
	unsigned long long item_id;

	Item(item_types type,QString key, unsigned long long item_id);
	void add_info(long cat_id)=0;
	void add_info(long cat_id,QString raw_data)=0;
	void add_info(long cat_id,QString raw_data,bitset<max_sources> abstract_categories)=0;
	void add_info(long cat_id,QString raw_data,bitset<max_sources> abstract_categories, QString POS)=0;
	void add_info(long cat_id,QString raw_data,bitset<max_sources> abstract_categories, QString POS, QString lemma_ID)=0;
	bool exists(long cat_id)=0;
	bool exists(long cat_id, QString raw_data)=0;
	bool exists(long cat_id,QString raw_data,bitset<max_sources> abstract_categories)=0;
	bool exists(long cat_id,QString raw_data,bitset<max_sources> abstract_categories, QString POS)=0;
	bool exists(long cat_id,QString raw_data,bitset<max_sources> abstract_categories, QString POS, QString lemma_ID)=0;
	bool get(long & cat_id);
	bool get(long cat_id, StringVector &raw_datas_entry);
};

class Item_1
{
public:
	typedef QVector<long> CategoryVector;


private:
	CategoryVector category_id;
public:
	Item_1(item_types type,long category_id);
	void add_info(long cat_id);
	bool exists(long cat_id);
};

class Item_1: public Item
{
public:
	typedef QList<QString> RawDatasEntry;
	typedef QVector<RawDatasEntry> RawDatas;

	void add_info(long cat_id,QString raw_data);
	void add_info(long cat_id,RawDatasEntry raw_data_entry);
	bool exists(long cat_id, QString raw_data);
	bool get(long cat_id, RawDatasEntry &raw_datas_entry);
private:
	RawDatas raw_datas;
};

typedef struct minimal_item_info_
{
	item_types type;
	long category_id;
	bitset<max_sources> abstract_categories; //only for STEMS
	QString raw_data;
	QString description;
	QString POS;
} minimal_item_info;
typedef struct all_item_info_
{

		unsigned long long item_id;
		long category_id;
		bitset<max_sources> abstract_categories; //only for STEMS
		bitset<max_sources> sources;
		QString raw_data;
		QString description;
		QString POS;
		QString lemma_ID; //only for STEMs
} all_item_info;
#endif
#endif // ITEM_STRUCTURES_H

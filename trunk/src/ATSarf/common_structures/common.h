#ifndef _COMMON_H
#define _COMMON_H

#include <QString>
#include <bitset>

using namespace std;

#define max_sources 128//256

extern bitset<max_sources> INVALID_BITSET;

enum rules { AA,AB,AC,BC,CC };
enum item_types { PREFIX, STEM, SUFFIX};

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

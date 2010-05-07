#ifndef _COMMON_H
#define _COMMON_H

#include <QString>
#include <bitset>

#define USE_TRIE
#define TRIE_FROM_FILE
#define USE_TRIE_WALK
//#define QUEUE
//#define MEMORY_EXHAUSTIVE
#define REDUCE_THRU_DIACRITICS


#ifndef USE_TRIE
#undef USE_TRIE_WALK
#undef TRIE_FROM_FILE
#endif
#ifndef QUEUE
#define PARENT
#endif
#ifdef REDUCE_THRU_DIACRITICS
#undef MEMORY_EXHAUSTIVE
#endif



using namespace std;

#define max_sources 256//128

extern bitset<max_sources> INVALID_BITSET;
extern QString delimiters;
extern QVector<QChar> alefs;

void initializa_variables(); //must be called at the start

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

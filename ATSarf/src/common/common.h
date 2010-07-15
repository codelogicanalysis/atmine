#ifndef _COMMON_H
#define _COMMON_H

#include <QString>
#include "dbitvec.h"

#define USE_TRIE
#define TRIE_FROM_FILE
#define USE_TRIE_WALK
//#define QUEUE
//#define MEMORY_EXHAUSTIVE //not yet completely implemented
#define REDUCE_THRU_DIACRITICS

//#define DEBUG

#define IGNORE_EXEC_TIMESTAMP

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
#define TIMESTAMP


using namespace std;

#define max_sources 256

extern bool filling;

extern dbitvec INVALID_BITSET;
extern QString delimiters;
extern QVector<QChar> alefs;

void initialize_variables(); //must be called at the start

enum rules { AA,AB,AC,BC,CC, RULES_LAST_ONE };
enum item_types { PREFIX, STEM, SUFFIX, ITEM_TYPES_LAST_ONE};

typedef class minimal_item_info_
{
public:
	item_types type;
	long category_id;
	dbitvec abstract_categories; //only for STEMS
	QString raw_data;
	long description_id;
	QString POS;

	QString description() const;

	minimal_item_info_()
	{
		abstract_categories.resize(max_sources);
	}
} minimal_item_info;
typedef class all_item_info_
{
public:
	unsigned long long item_id;
	long category_id;
	dbitvec abstract_categories; //only for STEMS
	dbitvec sources;
	QString raw_data;
	long description_id;
	QString POS;
	QString lemma_ID; //only for STEMs

	QString description() const;

	all_item_info_()
	{
		abstract_categories.resize(max_sources);
		sources.resize(max_sources);
	}
} all_item_info;

typedef struct text_info_
{
	QString *text;
	long long start,finish;
}text_info;

#endif

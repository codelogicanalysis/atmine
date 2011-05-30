#ifndef _COMMON_H
#define _COMMON_H

#include <QString>
#include <QStringList>
#include "hadith.h"
#include "dbitvec.h"

#define USE_TRIE
#define LOAD_FROM_FILE
#define USE_TRIE_WALK
//#define QUEUE
//#define MEMORY_EXHAUSTIVE //not yet completely implemented
#define REDUCE_THRU_DIACRITICS
#define MULTIPLICATION

//#define DEBUG

#define IGNORE_EXEC_TIMESTAMP

#ifndef USE_TRIE
#undef USE_TRIE_WALK
#undef LOAD_FROM_FILE
#endif
#ifndef QUEUE
#define PARENT
#endif
#ifdef REDUCE_THRU_DIACRITICS
#undef MEMORY_EXHAUSTIVE
#endif
#define TIMESTAMP

#define RUNON_WORDS
#define ENABLE_RUNON_WORD_INSIDE_COMPOUND_WORD
#define REMOVE_ONE_LETTER_ABBREVIATIONS_FROM_BEING_IN_RUNONWORDS


using namespace std;

#define max_sources 256

extern bool filling;

extern dbitvec INVALID_BITSET;

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
	long start,finish;
	QString getString(){ return text->mid(start, finish-start+1);}//TODO: solve bug in inconsistent start when diacritics exist
}text_info;

class multiply_params
{
public:
	bool raw_data:1;
	bool description:1;
	bool POS:1;
	bool abstract_category:1;

	void setAll()
	{
		raw_data=true;
		description=true;
		POS=true;
		abstract_category=true;
	}
	bool raw_dataONLY()
	{
		return (raw_data && !description && !POS && !abstract_category);
	}
	bool NONE()
	{
		return (!raw_data && !description && !POS && !abstract_category);
	}
	bool ALL()
	{
		return (raw_data && description && POS && abstract_category);
	}
	multiply_params()
	{
		setAll();
	}
};

static multiply_params M_ALL;

#ifndef SUBMISSION
static const QString databaseFileName="../../src/sql design/atm_filled.sql";
#else
static const  QString databaseFileName=".atm_filled.sql";
#endif

#ifdef LOAD_FROM_FILE
static const QString trie_path=".stem_trie.dat";
static const QString trie_list_path=".stem_list.dat";
static const QString compatibility_rules_path= ".compatibility.dat";
static const QString prefix_tree_path=".prefix_tree.dat";
static const QString suffix_tree_path=".suffix_tree.dat";
static const QString description_path=".descriptions.dat";
static const QString prefix_info_path=".prefix_info.dat";
static const QString suffix_info_path=".suffix_info.dat";
static const QString stem_info_path=".stem_info.dat";
#endif

//reverse_descriotion
#define REVERSEDESCBIT max_sources-2
#define setReverseDirection(abstractCat) abstractCat.setBit(REVERSEDESCBIT,true)
#define resetReverseDirection(abstractCat) abstractCat.setBit(REVERSEDESCBIT,false)
#define isReverseDirection(abstractCat) abstractCat.getBit(REVERSEDESCBIT)

#endif

#ifndef HADITH_H
#define HADITH_H

#include <QString>
#include "common.h"
#include "ATMProgressIFC.h"


//#define SUBMISSION //Dont forget to do so when u want to compile for submission to account for change in filename path

//#define DEBUG_BFS_TRAVERSAL
//#define DEBUG_DFS_TRAVERSAL
//#define DEBUG_BUILDGRAPH
#define GENERAL_HADITH
//#define FORCE_RANKS
#define DISPLAY_GRAPHNODES_CONTENT
//#define DISPLAY_NODES_BEING_BROKEN
//#define SHOW_RANKS
//#define SHOW_VERBOSE_RANKS
//#define STATS
#define TAG_HADITH
//#define HADITHDEBUG  //just for additional debugging statements displayed
#define PUNCTUATION
#define TRYTOLEARN
//#define ADD_ENARRATOR_NAMES
#define IBN_START
#define REFINEMENTS
#define PREPROCESS_DESCRIPTIONS
#define DISPLAY_HADITH_OVERVIEW
#define CHAIN_BUILDING
#define TEST_NARRATOR_GRAPH
#define EQUALITY_REFINEMENTS
//#define EQUALITYDEBUG
//#define COMPARE_TO_BUCKWALTER
//#define COMPARE_WITHOUT_ABSCAT
#define PROGRESSBAR
//#define JUST_BUCKWALTER
//#define TEST_WITHOUT_SKIPPING
//#define GET_AFFIXES_ALSO
//#define COUNT_AVERAGE_SOLUTIONS
//#define BUCKWALTER_INTERFACE
//#define OPTIMIZED_BUCKWALTER_TEST_CASE
//#define IMAN_CODE
//#define COUNT_RUNON

//#define NARRATORDEBUG
//#define COUNT_RASOUL
#define TEST_BIOGRAPHIES
//#define NARRATORHASH_DEBUG
#define HASH_TOTAL_VALUES
#define COLOR_ALL
#define EQUAL_NEW
#define PROGRESS_SERIALZATION
#define SEGMENT_BIOGRAPHY_USING_POR
//#define DONT_DISPLAY_BIOGRAPHY_GRAPHY

int hadithHelper(QString input_str,ATMProgressIFC *prgs);
void hadith_initialize();


#endif // HADITH_H

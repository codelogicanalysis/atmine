#ifndef HADITH_H
#define HADITH_H

#include <QString>
#include "common.h"
#include "ATMProgressIFC.h"


//#define SUBMISSION //Dont forget to do so when u want to compile for submission to account for change in filename path
#define WRITE_POR
#if 1
#define BIOGRAPHY_SEGMENT
//#define SEGMENT_BIO_ALGORITHM
#define SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
#define FAST_NEIGHBOR_CHECK
//#define BIOGRAPHY_ALL_MATCHING_NODES
#define SEGMENT_BIOGRAPHY_USING_POR
//#define BOUNDING_PARAGRAPH
#endif

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

//#define NARRATORDEBUG
//#define COUNT_RASOUL
#define TEST_BIOGRAPHIES
//#define NARRATORHASH_DEBUG
//#define DEFAULT_MERGE_NODES
#define HASH_TOTAL_VALUES
#define COLOR_ALL
#define EQUAL_NEW
#define PROGRESS_SERIALZATION
#define HASH_GRAPH_BUILDING
#define TAG_BIOGRAPHY

#define DISPLAY_BIOGRAPHY_GRAPH
#define MAX_DISPLAYABLE_SIZE 3000
#define TAG_HADITH

#define SAVE_MERGED_GRAPH_ONLY
#define DESERIALIZE_POR
//#define DEBUG_BIOGRAPHY_CLUSTERING

#define NONCONTEXT_LEARNING
#define INTER_ANNOTATION_AGREEMENT


int hadithHelper(QString input_str,ATMProgressIFC *prgs);
void hadith_initialize();

#endif // HADITH_H

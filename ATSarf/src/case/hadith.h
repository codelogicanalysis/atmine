#ifndef HADITH_H
#define HADITH_H

#include <QString>
#include "common.h"
#include "ATMProgressIFC.h"


#define WRITE_POR
#define BIOGRAPHY_SEGMENT
#define SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
#define FAST_NEIGHBOR_CHECK
#define SEGMENT_BIOGRAPHY_USING_POR

#define GENERAL_HADITH
#define DISPLAY_GRAPHNODES_CONTENT
//#define DISPLAY_NODES_BEING_BROKEN
//#define SHOW_RANKS
//#define SHOW_VERBOSE_RANKS
#define TRYTOLEARN
//#define ADD_ENARRATOR_NAMES
#define IBN_START
#define PREPROCESS_DESCRIPTIONS
#define DISPLAY_HADITH_OVERVIEW
#define CHAIN_BUILDING
#define TEST_NARRATOR_GRAPH
//#define COMPARE_WITHOUT_ABSCAT
#define PROGRESSBAR
//#define JUST_BUCKWALTER
//#define TEST_WITHOUT_SKIPPING
//#define GET_AFFIXES_ALSO
//#define COUNT_AVERAGE_SOLUTIONS
//#define OPTIMIZED_BUCKWALTER_TEST_CASE

#define TEST_BIOGRAPHIES
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

#define NONCONTEXT_LEARNING
#define INTER_ANNOTATION_AGREEMENT


int hadithHelper(QString input_str, ATMProgressIFC *prgs);
void hadith_initialize();

#endif

#ifndef HADITH_H
#define HADITH_H

#include <QString>
#include "common.h"
#include "ATMProgressIFC.h"

#define BIOGRAPHY_SEGMENT
#define SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
#define FAST_NEIGHBOR_CHECK
#define SEGMENT_BIOGRAPHY_USING_POR

#define GENERAL_HADITH
#define DISPLAY_GRAPHNODES_CONTENT
#define PREPROCESS_DESCRIPTIONS
#define DISPLAY_HADITH_OVERVIEW
#define CHAIN_BUILDING
#define PROGRESSBAR
//#define JUST_BUCKWALTER
//#define GET_AFFIXES_ALSO

#define TEST_BIOGRAPHIES
#define HASH_TOTAL_VALUES
#define COLOR_ALL
#define EQUAL_NEW

#define MAX_DISPLAYABLE_SIZE 3000

int hadithHelper(QString input_str, ATMProgressIFC *prgs);
void hadith_initialize();

#endif

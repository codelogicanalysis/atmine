#ifndef HADITH_H
#define HADITH_H

#include <QString>
#include "common.h"
#include "ATMProgressIFC.h"

#define BIOGRAPHY_SEGMENT
#define FAST_NEIGHBOR_CHECK
#define SEGMENT_BIOGRAPHY_USING_POR

#define DISPLAY_GRAPHNODES_CONTENT
#define DISPLAY_HADITH_OVERVIEW
//#define JUST_BUCKWALTER
//#define GET_AFFIXES_ALSO

#define MAX_DISPLAYABLE_SIZE 3000

int hadithHelper(QString input_str, ATMProgressIFC *prgs);
void hadith_initialize();

#endif

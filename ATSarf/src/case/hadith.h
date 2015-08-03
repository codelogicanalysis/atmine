#ifndef HADITH_H
#define HADITH_H

#include <QString>
#include "common.h"
#include "ATMProgressIFC.h"

//#define JUST_BUCKWALTER
//#define GET_AFFIXES_ALSO

#define MAX_DISPLAYABLE_SIZE 3000

int hadithHelper(QString input_str, ATMProgressIFC *prgs);
void hadith_initialize();

#endif

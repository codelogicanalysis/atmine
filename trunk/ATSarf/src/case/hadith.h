#ifndef HADITH_H
#define HADITH_H

#include <QString>
#include "common.h"
#include "ATMProgressIFC.h"
#define GENERAL_HADITH
//#define STATS
//#define HADITHDEBUG  //just for additional debugging statements displayed
//#define TENTATIVE //are decisions that we are not sure about
#define REFINEMENTS
#define PREPROCESS_DESCRIPTIONS
//#define COMPARE_TO_BUCKWALTER

int hadith(QString input_str,ATMProgressIFC *prgs);
void hadith_initialize();

#endif // HADITH_H

#ifndef HADITH_H
#define HADITH_H

#include <QString>
#include <QHash>
#include "common.h"
#include "ATMProgressIFC.h"
#define GENERAL_HADITH
//#define STATS
//#define HADITHDEBUG  //just for additional debugging statements displayed
//#define TENTATIVE //are decisions that we are not sure about
//#define REFINEMENTS
#define PREPROCESS_DESCRIPTIONS
#define DISPLAY_HADITH_OVERVIEW
#define CHAIN_BUILDING
//#define TEST_EQUAL_NARRATORS
//#define COMPARE_TO_BUCKWALTER
//#define COMPARE_WITHOUT_ABSCAT
#define PROGRESSBAR

#ifdef PREPROCESS_DESCRIPTIONS
extern QHash<long,bool> NMC_descriptions;
#endif

int hadith(QString input_str,ATMProgressIFC *prgs);
void hadith_initialize();

#endif // HADITH_H

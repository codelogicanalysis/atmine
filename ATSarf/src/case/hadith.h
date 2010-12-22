#ifndef HADITH_H
#define HADITH_H

#include <QString>
#include <QHash>
#include "common.h"
#include "ATMProgressIFC.h"
#define GENERAL_HADITH
//#define STATS
#define HADITHDEBUG  //just for additional debugging statements displayed
//#define TENTATIVE //are decisions that we are not sure about
#define IBN_START
#define REFINEMENTS
#define PREPROCESS_DESCRIPTIONS
#define DISPLAY_HADITH_OVERVIEW
#define CHAIN_BUILDING
#define TEST_EQUAL_NARRATORS
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

#ifdef PREPROCESS_DESCRIPTIONS
extern QHash<long,bool> NMC_descriptions;
#endif
extern int bit_NAME, bit_POSSESSIVE, bit_PLACE,bit_CITY,bit_COUNTRY;

int hadith(QString input_str,ATMProgressIFC *prgs);
void hadith_initialize();

#endif // HADITH_H

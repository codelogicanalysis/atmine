#ifndef HADITH_H
#define HADITH_H

#include <QString>
#include <QStringList>
#include <QHash>
#include "common.h"
#include "ATMProgressIFC.h"
//#define LINEAR_CHECK_FOR_VISITED
//#define DEBUG_BFS_TRAVERSAL
//#define DEBUG_DFS_TRAVERSAL
#define GENERAL_HADITH
#define FORCE_RANKS
//#define SHOW_RANKS
//#define SHOW_VERBOSE_RANKS
//#define STATS
//#define HADITHDEBUG  //just for additional debugging statements displayed
//#define TENTATIVE //are decisions that we are not sure about
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

#ifdef PREPROCESS_DESCRIPTIONS
extern QHash<long,bool> IBN_descriptions;
#endif

#ifdef REFINEMENTS
extern QStringList rasoul_words;
extern QString alrasoul;
#endif

extern int bit_NAME, bit_POSSESSIVE, bit_PLACE,bit_CITY,bit_COUNTRY;

int hadith(QString input_str,ATMProgressIFC *prgs);
void hadith_initialize();

class HadithParameters
{
public:
	unsigned int nmc_max:5;
	unsigned int nrc_max:5;
	unsigned int narr_min:5;
	unsigned int equality_radius:5;
	bool display_chain_num:1;
	unsigned int unused_int:11;
	double equality_delta;
	double equality_threshold;
	HadithParameters()
	{
		nmc_max=3;
		narr_min=3;
		nrc_max=5;
		equality_radius=3;
		equality_delta=0.4;
		equality_threshold=1;
		display_chain_num=true;
	}
};

extern HadithParameters parameters;

#endif // HADITH_H
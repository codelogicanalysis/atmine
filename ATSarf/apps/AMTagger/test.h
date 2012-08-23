#ifndef TEST_H
#define TEST_H

#include <QString>
#include "common.h"
#include "ATMProgressIFC.h"

#define AUGMENT_DICTIONARY
#define JUST_BUCKWALTER
//#define INSERT_ONLY_TIME
//#define INSERT_ONLY_NAMES
//#define INSERT_ONLY_PREFIXES
//#define INSERT_ONLY_SUFFIXES
//#define INSERT_ONLY_AFFIXES

//#define AUGMENT_ORIGINAL
//#define SAMA
#define MORPHEME_TOKENIZE
//#define DIACRITIC_DISAMBIGUATION
#define REDUCED_EQ_AMBIGUIOUS
//#define ATB
//#define ATB2
//#define ATB_DIACRITIC
#define REPETITIONS 1



int morphology(QString input_str,ATMProgressIFC * prg);
#endif // TEST_H

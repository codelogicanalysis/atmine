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
#define SAMA
#define MORPHEME_TOKENIZE
#define DIACRITIC_DISAMBIGUATION
//#define ATB
//#define ATB2
//#define ATB_DIACRITIC
#define REPETITIONS 1



int morphology(QString input_str,ATMProgressIFC * prg);
int hadith(QString input_str,ATMProgressIFC * prg);
int test(QString input_str,ATMProgressIFC * prg);
int verify(QString input_str,ATMProgressIFC * prg);
int breakAffix(QString input_str, ATMProgressIFC * prg);
int timeRecognize(QString input_str, ATMProgressIFC * prg);
int biographyHelper(QString input_str,ATMProgressIFC *prg);
int genealogy(QString input_str,ATMProgressIFC *prg);
int biography(QString input_str,ATMProgressIFC *prg);
int simple_annotation(QString input_str,ATMProgressIFC *prg);
int bible_annotation(QString input_str,ATMProgressIFC *prg);
int hadith_annotation(QString input_str,ATMProgressIFC *prg);
int hadith_name_annotation(QString input_str,ATMProgressIFC *prg);
int hadith_agreement(QString input_str,ATMProgressIFC *prg);
int genealogy_agreement(QString input_str,ATMProgressIFC *prg);
int narrator_annotation(QString input_str,ATMProgressIFC *prg);
int narrator_equality_comparision(QString input_str, ATMProgressIFC *prg);

#endif // TEST_H

#ifndef TEST_H
#define TEST_H

#include <QString>
#include "common.h"
#include "ATMProgressIFC.h"

#define AUGMENT_DICTIONARY
#define JUST_BUCKWALTER
#define REPETITIONS 1

int morphology(QString input_str,ATMProgressIFC * prg);
int hadith(QString input_str,ATMProgressIFC * prg);
int test(QString input_str,ATMProgressIFC * prg);
int verify(QString input_str,ATMProgressIFC * prg);
int breakAffix(QString input_str, ATMProgressIFC * prg);
int timeRecognize(QString input_str, ATMProgressIFC * prg);

#endif // TEST_H

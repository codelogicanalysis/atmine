#ifndef TEST_H
#define TEST_H

#include <QString>
#include "common.h"
#include "ATMProgressIFC.h"

//#define AUGMENT_DICTIONARY
//#define JUST_BUCKWALTER
#define REPETITIONS 1
//starting point
int start(QString input_str, QString &output_str, QString &error_str, QString & hadith_str, bool hadith,ATMProgressIFC *m_ui);

//starting point
int test(QString input_str, QString &output_str, QString &error_str, QString & hadith_str, bool hadith,ATMProgressIFC *m_ui);

#endif // TEST_H

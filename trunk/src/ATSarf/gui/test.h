#ifndef TEST_H
#define TEST_H

#include <QString>
#include "../common_structures/common.h"
#include "../logger/ATMProgressIFC.h"

//#define AUGMENT_DICTIONARY

//starting point
int start(QString input_str, QString &output_str, QString &error_str, QString & hadith_str, bool hadith,ATMProgressIFC *m_ui);

#endif // TEST_H

#ifndef TEST_H
#define TEST_H

#include <QString>
#include "../common_structures/common.h"

//careful just one of the below precompiler definitions should be left uncommented
//if more than one is defined than HADITH takes priority then WORD_SARF then AUGMENT_DICTIONARY
//#define AUGMENT_DICTIONARY
//#define HADITH
//#define WORD_SARF

//starting point
#ifdef GUI_SPECIFIC
#include "../gui/mainwindow.h"
int start(QString input_str, QString &output_str, QString &error_str, QString & hadith_str, bool hadith,Ui::MainWindow *m_ui);
#else
int start(QString input_str, QString &output_str, QString &error_str, QString & hadith_str, bool hadith);
#endif

#endif // TEST_H

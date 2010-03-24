#ifndef TEST_H
#define TEST_H

#include <QString>

enum wordState { IKHBAR, KAWL, AAN, NAME,OTHER};

//starting point
int start(QString input_str, QString &output_str, QString &error_str);
int getSanadBeginning(QString word);
#endif // TEST_H

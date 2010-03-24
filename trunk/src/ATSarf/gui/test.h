#ifndef TEST_H
#define TEST_H

#include <QString>

enum wordType { IKHBAR, KAWL, AAN, NAME, NAME_NABI,OTHER};


//starting point
int start(QString input_str, QString &output_str, QString &error_str);
int getSanadBeginning(QString word);
wordType getWordType(QString word);
bool isValidTransition(int previousState,wordType currentState);


#endif // TEST_H

#ifndef TEST_H
#define TEST_H

#include <QString>

enum wordType { IKHBAR, KAWL, AAN, NAME, NAME_NABI,OTHER};

const QChar alef_hamza_above= QChar(0x0623);
const QChar ya2=QChar(0x064A);
const QChar waw=QChar(0x0648);
const QChar kha2=QChar(0x062E);
const QChar ba2=QChar(0x0628);
const QChar ra2=QChar(0x0628);
const QChar noon=QChar(0x0646);
const QChar alef=QChar(0x0627);
const QChar sa2=QChar(0x0633);
const QChar meem=QChar(0x0645);
const QChar ayn=QChar(0x0639);
const QChar ta2=QChar(0x062A);
const QChar qaf=QChar(0x0642);


QString a5barani(alef_hamza_above);
//a5barani.append(kha2);
//.append.(ba2).append(ra2).append(noon).append(ya2);



//starting point
int start(QString input_str, QString &output_str, QString &error_str);
int getSanadBeginning(QString word);
wordType getWordType(QString word);
bool isValidTransition(int previousState,wordType currentState);


#endif // TEST_H

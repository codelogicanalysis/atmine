#ifndef _DIACRITICS_H
#define	_DIACRITICS_H

#include <QString>
#include <QChar>

//constantletters
const QChar ya2=QChar(0x064A);
const QChar alef=QChar(0x0627);
const QChar alef_madda_above= QChar(0x0622);
const QChar alef_hamza_above= QChar(0x0623);
const QChar alef_hamza_below= QChar(0x0625);
const QChar ta2_marbouta=QChar(0x0629);
const QChar waw=QChar(0x0648);
const QChar shadde=QChar(0x0651);
const QChar fatha=QChar(0x064E);
const QChar damma=QChar(0x064F);
const QChar kasra=QChar(0x0650);
const QChar sukun=QChar(0x0652);
const QChar lam=QChar(0x0644);
const QChar kasratayn=QChar(0x064D);
const QChar dammatayn=QChar(0x064C);
const QChar fathatayn=QChar(0x064B);
const QChar kha2=QChar(0x062E);
const QChar ba2=QChar(0x0628);
const QChar ra2=QChar(0x0628);
const QChar noon=QChar(0x0646);
const QChar seen=QChar(0x0633);
const QChar meem=QChar(0x0645);
const QChar ayn=QChar(0x0639);
const QChar ta2=QChar(0x062A);
const QChar qaf=QChar(0x0642);
const QChar _7a2=QChar(0x062D);
const QChar dal=QChar(0x062F);
const QChar tha2=QChar(0x062B);
const QChar fasila=QChar(0x060C);
const QChar aleft_superscript=QChar(0x0670);
const QChar ha2 =QChar(0x0647);

bool isConsonant(QChar letter);
bool isDiacritic(QChar letter); //TODO: add the madda
QString removeDiacritics(QString /*&*/text);
int getLastLetter_index(QString word); //last non-Diacritical letter, -1 means that all letters are diactrics
QChar getLastLetter(QString word, int pos); //helper function for last non-Diacritic letter
QChar getLastLetter(QString word); //last non-Diacritical letter
QString removeLastLetter(QString word); //last non-Diacritical letter
QString removeLastDiacritic(QString word); //removes last consecutive diactrics until a normal letter is reached
QString getDiacriticword(int position,int startPos,QString diacritic_word);

#endif	/* _DIACRITICS_H */


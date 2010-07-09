#ifndef _DIACRITICS_H
#define	_DIACRITICS_H

#include <QString>
#include <QChar>
#include "letters.h"

bool isConsonant(QChar letter);
bool isDiacritic(QChar letter); //TODO: add the madda
QString removeDiacritics(QString /*&*/text);
int getLastLetter_index(QString word); //last non-Diacritical letter, -1 means that all letters are diactrics
QChar getLastLetter(QString word, int pos); //helper function for last non-Diacritic letter
QChar getLastLetter(QString word); //last non-Diacritical letter
QString removeLastLetter(QString word); //last non-Diacritical letter
QString removeLastDiacritic(QString word); //removes last consecutive diactrics until a normal letter is reached
QString getDiacriticword(int position,int startPos,QString diacritic_word);
QString addlastDiacritics(int start, int position, QString * diacritic_word, int &last_position);
QString addlastDiacritics(int start, int position, QString * diacritic_word);
#endif	/* _DIACRITICS_H */


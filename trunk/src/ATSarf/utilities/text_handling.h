#ifndef _TEXT_HANDLING_H
#define _TEXT_HANDLING_H

#include <QString>
#include <QChar>

QString get_Possessive_form(QString word);
bool equal(QChar c1, QChar c2);
bool equal_ignore_diacritics(QString &word1,QString &word2);
bool equal(QString &word1,QString &word2);// is diacritics tolerant
bool startsWithAL( QString word); //does not take in account cases were Diacritics may be present on the alef and lam of "al"
bool removeAL( QString &word); //does not take in account cases were Diacritics may be present on the alef and lam of "al"

#endif // _TEXT_HANDLING_H

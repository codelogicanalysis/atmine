#ifndef _TEXT_HANDLING_H
#define _TEXT_HANDLING_H

#include <QString>
#include <QChar>

QString get_Possessive_form(QString word);
bool equal(const QChar & c1, const QChar & c2);
bool equal_ignore_diacritics(const QString &word1,const QString &word2);
bool equal(const QString &word1,const QString &word2);// is diacritics tolerant
bool startsWithAL(const QString & word); //does not take in account cases were Diacritics may be present on the alef and lam of "al"
bool removeAL( QString &word); //does not take in account cases were Diacritics may be present on the alef and lam of "al"

#endif // _TEXT_HANDLING_H

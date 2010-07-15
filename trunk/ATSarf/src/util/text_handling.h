#ifndef _TEXT_HANDLING_H
#define _TEXT_HANDLING_H

#include <QString>
#include <QChar>
#include <QVector>
#include "letters.h"
#include "common.h"

QString get_Possessive_form(QString word);
inline bool equal(const QChar & c1, const QChar & c2)
{
	if (c1==c2)
		return true;
	if (alefs.contains(c1) && alefs.contains(c2))
		return true;
	if ((c1==veh && c2==feh) || (c2==veh && c1==feh))
		return true;
	return false;
}
bool equal_ignore_diacritics(const QString &word1,const QString &word2);
bool equal(const QString &word1,const QString &word2);// is diacritics tolerant
bool equal(const QStringRef &word1,const QStringRef &word2); // is diacritics tolerant
bool equal(const QStringRef &word1,const QString &word2); // is diacritics tolerant

inline bool startsWithAL(const QString & word)//does not take in account cases were Diacritics may be present on the alef and lam of "al"
{
	if (word.length()<=2)
		return false;
	if (word[0]==alef && word[1]==lam)//TODO: whenever it is changed to include diactrics in-between it must notice that only some combinations of diactrics imply AL (definite article) else is part of the word
		return true;
	return false;
}
inline bool removeAL( QString &word)//does not take in account cases were Diacritics may be present on the alef and lam of "al"
{
	if (!startsWithAL(word))
		return false;
	word=word.right(word.length()-2);
	return true;
}

#endif // _TEXT_HANDLING_H

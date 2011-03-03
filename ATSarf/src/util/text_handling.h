#ifndef _TEXT_HANDLING_H
#define _TEXT_HANDLING_H

#include <QString>
#include <QChar>
#include <QVector>
#include "letters.h"
#include "diacritics.h"
#include "common.h"
#include "logger.h"

inline QString get_Possessive_form(QString word)
{
	if (word.length()>=2)
	{
		int last_index=getLastLetter_index(word);
		QChar last=getLastLetter(word,last_index);
		QChar before=getLastLetter(word.left(last_index));
		if (last==alef && isConsonant(before))
			return removeLastDiacritic(word).append(waw).append(ya2);
		else if (last==alef && before==waw )
			return removeLastDiacritic(removeLastLetter(word)).append(ya2);
		else if (last==alef && before==ya2 )
			return removeLastDiacritic(removeLastLetter(word)).append(shadde);
		else if (last==ta2_marbouta && isConsonant(before))
			return removeLastDiacritic(removeLastLetter(word)).append(ya2);
		else if (last==ya2)
			return removeLastDiacritic(word).append(shadde);
		else if (isConsonant(last) || last==waw)
			return removeLastDiacritic(word).append(ya2);
		else
		{
			out << "Unknown Rule for Possessive form\n";
			return QString::null;
		}
	}
	else
		return word.append(ya2);
}
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
inline bool equal_ignore_diacritics(const QString &word1,const QString &word2)
{
	int length1=word1.length(),length2=word2.length();
	int i=0,j=0;
	while(i<length1 && j<length2)
	{
		if (isDiacritic(word1[i]))
			i++;
		if (isDiacritic(word2[j]))
			j++;
		if (i<length1 && j<length2 && word1[i]!=word2[j])
			return false;
		i++;
		j++;
	}
	if (length1-(i+1)==0)
	{
		for (int f=j+1;f<length2;f++)
			if (!isDiacritic(word2[f]))
				return false;
	}
	else
	{
		for (int f=i+1;f<length1;f++)
			if (!isDiacritic(word1[f]))
				return false;
	}
	return true;
}
bool checkIfSmallestIsPrefixOfLargest(const QStringRef &word1,const QStringRef &word2, int & i1, int & i2); //modifies value of i1 and i2
inline bool equal(const QStringRef &word1,const QStringRef &word2) // is diacritics tolerant and ignores punctuation
{
	int i1,i2;
	if (checkIfSmallestIsPrefixOfLargest(word1,word2,i1,i2))
		return true;
	int length1=word1.length(),
		length2=word2.length();
	if (length1-(i1+1)==0)
	{
		for (int i=i2+1;i<length2;i++)
			if (!isDiacritic(word2.at(i)) && !isPunctuationMark(word2.at(i)))
				return false;
	}
	else
	{
		for (int i=i1+1;i<length1;i++)
			if (!isDiacritic(word1.at(i)) && !isPunctuationMark(word1.at(i)))
				return false;
	}
	return true;
}
inline bool equal(const QString &word1,const QString &word2)// is diacritics tolerant
{
	return equal(word1.rightRef(-1),word2.rightRef(-1));//rightRef of <0 returns whole string
}
inline bool equal(const QStringRef &word1,const QString &word2) // is diacritics tolerant
{
	return equal(word1,word2.rightRef(-1));//rightRef of <0 returns whole string
}

inline bool startsWith(const QStringRef &text,const QString &substring, int & finish_pos) // is diacritics tolerant checks if a word ends with a delimeter terminated 'subset'
{
	int text_length=text.length(), substring_length=substring.length();
	if(text_length<substring_length)
		return false;
	int i2;
	if (checkIfSmallestIsPrefixOfLargest(text,substring.rightRef(-1),finish_pos,i2))
	{
		finish_pos++;
		while(finish_pos<text_length && isDiacritic(text.at(finish_pos)))
			finish_pos++;
		if (finish_pos==text_length)
			return true;
		if (isDelimiter(text.at(finish_pos))) //subset but not part of a word
		{
			finish_pos--;
			return true;
		}
		return false;
	}
	return false;
}
inline bool startsWith(const QString &text,const QString &substring) // is diacritics tolerant checks if a word ends with a delimeter terminated 'subset'
{
	int dummy_int;
	return startsWith(text.rightRef(-1),substring,dummy_int);
}

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

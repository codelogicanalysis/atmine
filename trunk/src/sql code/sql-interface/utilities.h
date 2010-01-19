#ifndef UTILITIES_H
#define UTILITIES_H

#include "sql-interface.h"

//constantletters
const QChar ya2=QChar(0x064A);
const QChar alef=QChar(0x0627);
const QChar ta2_marbouta=QChar(0x0629);
const QChar waw=QChar(0x0648);
const QChar shadde=QChar(0x0651);
const QChar fatha=QChar(0x064E);
const QChar damma=QChar(0x064F);
const QChar kasra=QChar(0x0650);
const QChar sukun=QChar(0x0652);
const QChar lam=QChar(0x0644);

//utility functions
inline bool isConsonant(QChar letter)
{
	if (letter !=ya2 && letter !=waw && letter !=alef) //not a very firm condition to assume consonant but might work here
		return true;
	else
		return false;
}
inline bool isDiactric(QChar letter)
{
	if (letter==shadde || letter==fatha || letter==damma || letter==kasra || letter==sukun)
		return true;
	else
		return false;
}
inline QString removeDiactrics(QString /*&*/text) //in-efficient, change later
{
	QRegExp exp(QString("[")+shadde+fatha+damma+sukun+kasra+QString("]"));
	/*QString changed=*/return text.remove(exp);
	/*int letters_removed=text.length()-changed.length();
	text=changed;
	return letters_removed;*/
}
inline int getLastLetter_index(QString word) //last non-diactrical letter
{
	int length=word.length();
	if (length==0)
		return -1;
	int i=length-1;
	while (i>=0 && isDiactric(word[i]))
		i--;
	return i;
}
inline QChar getLastLetter(QString word, int pos) //helper function for last non-diactric letter
{
	if (pos>=0 && pos < word.length())
		return word[pos];
	else
		return '\0';
}
inline QChar getLastLetter(QString word) //last non-diactrical letter
{
	int pos=getLastLetter_index(word);
	return getLastLetter(word,pos);
}
inline QString removeLastLetter(QString word) //last non-diactrical letter
{
	return word.left(getLastLetter_index(word));
}
inline QString removeLastDiactric(QString word) //only one Diactric is removed
{
	if (word.length()==0)
		return word;
	if (isDiactric(word[word.length()-1]))
		return word.left(word.length()-1);
	return word;
}
QString get_Possessive_form(QString word)
{
	if (word.length()>=2)
	{
		int last_index=getLastLetter_index(word);
		QChar last=getLastLetter(word,last_index);
		QChar before=getLastLetter(word.left(last_index));
		if (last==alef && isConsonant(before))
			return removeLastDiactric(word).append(waw).append(ya2);
		else if (last==alef && before==waw )
			return removeLastDiactric(removeLastLetter(word)).append(ya2);
		else if (last==alef && before==ya2 )
			return removeLastDiactric(removeLastLetter(word)).append(shadde);
		else if (last==ta2_marbouta && isConsonant(before))
			return removeLastDiactric(removeLastLetter(word)).append(ya2);
		else if (last==ya2)
			return removeLastDiactric(word).append(shadde);
		else if (isConsonant(last) || last==waw)
			return removeLastDiactric(word).append(ya2);
		else
		{
			out << "Unknown Rule for Possessive form\n";
			return QString::null;
		}
	}
	else
		return word.append(ya2);
}
/*
alef 				+consonant before			-wey					sayda --> saydawey		or		7alba --> 7albawey
alef				+waw before					-ya2 instead of alef	nikaragwa -->nikaragwiy
alef				+ya2 before					-shadde instead of alef	souriya--> souriy
consonant or waw								-ya2					lubnan -> lubnaniy		or		hunululo  -->	hunululoy (???)
ta2	marbouta		+consonant before			-ya2 instead of ta2		makka --> makkiy
ya2												-add a shadde on ya2	jibouti --> jiboutiy

Notes:
-remove ta3reef if it is there, except for almanya, albanya, ...
-asma2 el mourakaba according to last word
  */

bool startsWithAL( QString word) //does not take in account cases were diactrics may be present on the alef and lam of "al"
{
	if (word.length()<=2)
		return false;
	if (word[0]==alef && word[1]==lam)
		return true;
	return false;
}

bool removeAL( QString &word) //does not take in account cases were diactrics may be present on the alef and lam of "al"
{
	if (!startsWithAL(word))
		return false;
	word=word.right(word.length()-2);
	return true;
}

#endif // UTILITIES_H

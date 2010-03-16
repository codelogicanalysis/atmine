#ifndef UTILITIES_H
#define UTILITIES_H

//#include "sql-interface.h"
#include <QString>
#include <QChar>
#include <QList>
#include <QVector>

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

//utility functions
inline bool isConsonant(QChar letter)
{
	if (letter !=ya2 && letter !=waw && letter !=alef) //not a very firm condition to assume consonant but might work here
		return true;
	else
		return false;
}
inline bool isDiacritic(QChar letter) //TODO: add the madda
{
	if (letter==shadde || letter==fatha || letter==damma || letter==kasra || letter==sukun || letter==kasratayn || letter==dammatayn || letter==fathatayn)
		return true;
	else
		return false;
}
inline QString removeDiacritics(QString /*&*/text)
{
	QRegExp exp(QString("[")+shadde+fatha+damma+sukun+kasra+kasratayn+fathatayn+dammatayn+QString("]"));
	/*QString changed=*/return text.remove(exp);
	/*int letters_removed=text.length()-changed.length();
	text=changed;
	return letters_removed;*/
}
inline int getLastLetter_index(QString word) //last non-Diacritical letter, -1 means that all letters are diactrics
{
	int length=word.length();
	if (length==0)
		return -1;
	int i=length-1;
	while (i>=0 && isDiacritic(word[i]))
		i--;
	return i; //even if -1 is returned it shows that all characters are diactrics
}
inline QChar getLastLetter(QString word, int pos) //helper function for last non-Diacritic letter
{
	if (pos>=0 && pos < word.length())
		return word[pos];
	else
		return '\0';
}
inline QChar getLastLetter(QString word) //last non-Diacritical letter
{
	int pos=getLastLetter_index(word);
	return getLastLetter(word,pos);
}
inline QString removeLastLetter(QString word) //last non-Diacritical letter
{
	return word.left(getLastLetter_index(word));
}
inline QString removeLastDiacritic(QString word) //removes last consecutive diactrics until a normal letter is reached
{
	/*if (word.length()==0)
		return word;
	int i=0;//number of letters to remove
	while (isDiacritic(word[word.length()-i-1]))
		i++;
	return word.left(word.length()-i);*/
	return word.left(getLastLetter_index(word)+1);
}
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

QString getDiacriticword(int position,int startPos,QString diacritic_word)
{
	int diacritic_starting_pos;
	int length=diacritic_word.count();
		//start by finding the number of letters that are equivalent to startPos
		int num_letters=startPos+1;
		int count=0;
		int i;
		for (i=0;i<length;i++)
			if (!isDiacritic(diacritic_word[i]))
			{
				count++;
				if (count==num_letters)
					break;
			}
		diacritic_starting_pos=i;
	num_letters=position-startPos+2;//num_letters = the actual non_Diacritic letters in the match +1
	count=0;
	for (i=diacritic_starting_pos;i<length;i++)
		if (!isDiacritic(diacritic_word[i]))
		{
			count++;
			if (count==num_letters)
				break;
		}
	return diacritic_word.mid(diacritic_starting_pos<0?0:diacritic_starting_pos,i-diacritic_starting_pos);
}

inline bool equal_strict(QList<QChar> list1,QList<QChar> list2)
{
	int l1=list1.count(),l2=list2.count();
	if (l1!=l2)
		return false;
	for (int i=0;i<l1;i++)
		if (list1[i]!=list2[i])
			return false;
	return true;
}
inline bool equal(QChar c1, QChar c2)
{
	if (c1==c2)
		return true;
	QVector<QChar> alefs(4);
	alefs.append(alef);
	alefs.append(alef_hamza_above);
	alefs.append(alef_hamza_below);
	alefs.append(alef_madda_above);
	if (alefs.contains(c1) && alefs.contains(c2))
		return true;
	return false;
}
inline bool equal(QString word1,QString word2)// is diacritics tolerant
{
	int length1=word1.count();
	int length2=word2.count();
	int i1=-1,i2=-1;
	QList<QChar> diacritics1,diacritics2;
	QChar letter1,letter2;
	while (i1+1<length1 && i2+1<length2)
	{
		i1++;
		i2++;
		diacritics1.clear();
		diacritics2.clear();
		while (isDiacritic(word1[i1]))
		{
			if (word1[i1]!=shadde)
				diacritics1.append(word1[i1]);
			i1++;
		}
		if (i1<length1)
			letter1=word1[i1];
		else
			letter1='\0';
		while (isDiacritic(word2[i2]))
		{
			if (word2[i2]!=shadde)
				diacritics2.append(word2[i2]);
			i2++;
		}
		if (i2<length2)
			letter2=word2[i2];
		else
			letter2='\0';
		//now comparison first diacritics then next_letter
		if (diacritics1.count()==0 || diacritics2.count()==0 || equal_strict(diacritics1,diacritics2))
		{
			if (equal(letter1,letter2))
				continue;
			else
				return false;
		}
		return false;
	}
	if (length1-(i1+1)==0 && length2-(i2+1)==0)
		return true;
	if (length1-(i1+1)==0)
	{
		for (int i=i2+1;i<length2;i++)
			if (!isDiacritic(word2[i]))
				return false;
	}
	else
	{
		for (int i=i1+1;i<length1;i++)
			if (!isDiacritic(word1[i]))
				return false;
	}
	return true;
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

inline bool startsWithAL( QString word) //does not take in account cases were Diacritics may be present on the alef and lam of "al"
{
	if (word.length()<=2)
		return false;
	if (word[0]==alef && word[1]==lam)//TODO: whenever it is changed to include diactrics in-between it must notice that only some combinations of diactrics imply AL (definite article) else is part of the word
		return true;
	return false;
}
inline bool removeAL( QString &word) //does not take in account cases were Diacritics may be present on the alef and lam of "al"
{
	if (!startsWithAL(word))
		return false;
	word=word.right(word.length()-2);
	return true;
}

#endif // UTILITIES_H

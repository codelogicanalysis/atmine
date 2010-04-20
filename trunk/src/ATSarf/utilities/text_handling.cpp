#include "text_handling.h"

#include "logger/logger.h"
#include "diacritics.h"
#include <QString>
#include <QChar>
#include <QList>
#include <QVector>

QString get_Possessive_form(QString word)
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

bool equal_strict(QList<QChar> list1,QList<QChar> list2)
{
	int l1=list1.count(),l2=list2.count();
	if (l1!=l2)
		return false;
	for (int i=0;i<l1;i++)
		if (list1[i]!=list2[i])
			return false;
	return true;
}
bool equal(QChar c1, QChar c2)
{
	if (c1==c2)
		return true;
	QVector<QChar> alefs(4);
	alefs.append(alef);
	alefs.append(alef_hamza_above);
	alefs.append(alef_hamza_below);
	alefs.append(alef_madda_above);
	alefs.append(alef_wasla);
	if (alefs.contains(c1) && alefs.contains(c2))
		return true;
	if ((c1==veh && c2==feh) || (c2==veh && c1==feh))
		return true;
	return false;
}
bool equal(QString word1,QString word2)// is diacritics tolerant
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
			if (word1[i1]!=shadde && word1[i1]!=aleft_superscript)
				diacritics1.append(word1[i1]);
			i1++;
		}
		if (i1<length1)
			letter1=word1[i1];
		else
			letter1='\0';
		while (isDiacritic(word2[i2]))
		{
			if (word2[i2]!=shadde && word2[i2]!=aleft_superscript)
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
bool startsWithAL( QString word) //does not take in account cases were Diacritics may be present on the alef and lam of "al"
{
	if (word.length()<=2)
		return false;
	if (word[0]==alef && word[1]==lam)//TODO: whenever it is changed to include diactrics in-between it must notice that only some combinations of diactrics imply AL (definite article) else is part of the word
		return true;
	return false;
}
bool removeAL( QString &word) //does not take in account cases were Diacritics may be present on the alef and lam of "al"
{
	if (!startsWithAL(word))
		return false;
	word=word.right(word.length()-2);
	return true;
}

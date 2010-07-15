#include "text_handling.h"
#include "common.h"
#include "logger.h"
#include "diacritics.h"
#include <QString>
#include <QChar>
#include <QList>
#include <QVector>
#include <QDebug>

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

bool equal_strict(QList<QChar> & list1,QList<QChar> & list2)
{
	int l1=list1.count(),l2=list2.count();
	if (l1!=l2)
		return false;
	for (int i=0;i<l1;i++)
		if (list1[i]!=list2[i])
			return false;
	return true;
}

//equal functions are just copy-paste of each other
bool equal(const QString &word1,const QString &word2) // is diacritics tolerant
{
	//qDebug() << word1<<"-"<<word2;
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
		while (i1<length1 && isDiacritic(word1[i1]))
		{
			if (word1[i1]!=shadde && word1[i1]!=aleft_superscript)
				diacritics1.append(word1[i1]);
			i1++;
		}
		if (i1<length1)
			letter1=word1[i1];
		else
			letter1='\0';
		while (i2<length2 && isDiacritic(word2[i2]))
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

bool equal(const QStringRef &word1,const QStringRef &word2) // is diacritics tolerant
{
	//qDebug() << word1<<"-"<<word2;
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
		while (i1<length1 && isDiacritic(word1.at(i1)))
		{
			if (word1.at(i1)!=shadde && word1.at(i1)!=aleft_superscript)
				diacritics1.append(word1.at(i1));
			i1++;
		}
		if (i1<length1)
			letter1=word1.at(i1);
		else
			letter1='\0';
		while (i2<length2 && isDiacritic(word2.at(i2)))
		{
			if (word2.at(i2)!=shadde && word2.at(i2)!=aleft_superscript)
				diacritics2.append(word2.at(i2));
			i2++;
		}
		if (i2<length2)
			letter2=word2.at(i2);
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
			if (!isDiacritic(word2.at(i)))
				return false;
	}
	else
	{
		for (int i=i1+1;i<length1;i++)
			if (!isDiacritic(word1.at(i)))
				return false;
	}
	return true;
}

bool equal(const QStringRef &word1,const QString &word2) // is diacritics tolerant
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
		while (i1<length1 && isDiacritic(word1.at(i1)))
		{
			if (word1.at(i1)!=shadde && word1.at(i1)!=aleft_superscript)
				diacritics1.append(word1.at(i1));
			i1++;
		}
		if (i1<length1)
			letter1=word1.at(i1);
		else
			letter1='\0';
		while (i2<length2 && isDiacritic(word2.at(i2)))
		{
			if (word2.at(i2)!=shadde && word2.at(i2)!=aleft_superscript)
				diacritics2.append(word2.at(i2));
			i2++;
		}
		if (i2<length2)
			letter2=word2.at(i2);
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
			if (!isDiacritic(word2.at(i)))
				return false;
	}
	else
	{
		for (int i=i1+1;i<length1;i++)
			if (!isDiacritic(word1.at(i)))
				return false;
	}
	return true;
}

bool equal_ignore_diacritics(const QString &word1,const QString &word2)
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

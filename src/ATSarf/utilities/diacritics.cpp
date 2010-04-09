#include "diacritics.h"

#include <QString>
#include <QChar>
#include <QList>
#include <QVector>
#include <QRegExp>

//utility functions
bool isConsonant(QChar letter)
{
	if (letter !=ya2 && letter !=waw && letter !=alef) //not a very firm condition to assume consonant but might work here
		return true;
	else
		return false;
}
bool isDiacritic(QChar letter) //TODO: add the madda
{
	if (letter==shadde || letter==fatha || letter==damma || letter==kasra || letter==sukun || letter==kasratayn || letter==dammatayn || letter==fathatayn || letter ==aleft_superscript)
		return true;
	else
		return false;
}
QString removeDiacritics(QString /*&*/text)
{
	QRegExp exp(QString("[")+shadde+fatha+damma+sukun+kasra+kasratayn+fathatayn+dammatayn+aleft_superscript+QString("]"));
	/*QString changed=*/return text.remove(exp);
	/*int letters_removed=text.length()-changed.length();
	text=changed;
	return letters_removed;*/
}
int getLastLetter_index(QString word) //last non-Diacritical letter, -1 means that all letters are diactrics
{
	int length=word.length();
	if (length==0)
		return -1;
	int i=length-1;
	while (i>=0 && isDiacritic(word[i]))
		i--;
	return i; //even if -1 is returned it shows that all characters are diactrics
}
QChar getLastLetter(QString word, int pos) //helper function for last non-Diacritic letter
{
	if (pos>=0 && pos < word.length())
		return word[pos];
	else
		return '\0';
}
QChar getLastLetter(QString word) //last non-Diacritical letter
{
	int pos=getLastLetter_index(word);
	return getLastLetter(word,pos);
}
QString removeLastLetter(QString word) //last non-Diacritical letter
{
	return word.left(getLastLetter_index(word));
}
QString removeLastDiacritic(QString word) //removes last consecutive diactrics until a normal letter is reached
{
	/*if (word.length()==0)
		return word;
	int i=0;//number of letters to remove
	while (isDiacritic(word[word.length()-i-1]))
		i++;
	return word.left(word.length()-i);*/
	return word.left(getLastLetter_index(word)+1);
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


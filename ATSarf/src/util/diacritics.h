#ifndef _DIACRITICS_H
#define	_DIACRITICS_H

#include <QString>
#include <QChar>
#include <QRegExp>
#include "letters.h"

inline bool isConsonant(const QChar & letter) {
	if (letter !=ya2 && letter !=waw && letter !=alef) //not a very firm condition to assume consonant but might work here
		return true;
	else
		return false;
}
inline bool isDiacritic(const QChar & letter) {
	if (letter==shadde || letter==shadde || letter==fatha || letter==damma || letter==kasra ||
			letter==sukun || letter==kasratayn || letter==dammatayn || letter==fathatayn ||
			letter==aleft_superscript|| letter==shadde_alef_above||letter==madda)
		return true;
	else
		return false;
}
inline QString removeDiacritics(QString /*&*/text) {
	QRegExp exp(QString("[")+shadde+fatha+damma+sukun+kasra+kasratayn+fathatayn+dammatayn+aleft_superscript+QString("]"));
	return text.remove(exp);
}
inline int getLastLetter_index(const QString & word, int pos) {//last non-Diacritical letter, -1 means that all letters are diacritics
	int length=word.length();
	if (length<=pos)
		pos=length-1;
	while (pos>=0 && isDiacritic(word[pos]))
		pos--;
	return pos; //even if -1 is returned it shows that all characters are diactrics
}
inline QChar _getLastLetter(const QString &word, int pos) {//helper function for last non-Diacritic letter
	if (pos>=0 && pos < word.length())
		return word[pos];
	else
		return '\0';
}
inline QChar getLastLetter(const QString & word) {//last non-Diacritical letter
	int pos=getLastLetter_index(word,word.length()-1);
	return _getLastLetter(word,pos);
}
inline QString removeLastLetter(const QString & word) {//last non-Diacritical letter
	return word.left(getLastLetter_index(word,word.length()-1));
}
inline QString removeLastDiacritics(const QString &word) { //removes last consecutive diactrics until a normal letter is reached
	return word.left(getLastLetter_index(word,word.length()-1)+1);
}
QString getDiacriticword(int position,int startPos,QString diacritic_word);
inline int getLastDiacritic(int position, QString * diacritic_word) { //get letter position after last diacritic starting from 'position+1', if no diacritic is found returns 'position+1'
	if (position<-1)
		return position+1;
	QChar nextletter;
	do{
		position++;
		if (position ==diacritic_word->length())
			break;
		nextletter=diacritic_word->at(position);
	}while (isDiacritic(nextletter));
	return position;
}
inline QStringRef addlastDiacritics(int start, int position, QString * diacritic_word, int &last_position) {
	last_position=getLastDiacritic(position,diacritic_word);
	return diacritic_word->midRef(start,last_position-start);
}
inline QStringRef addlastDiacritics(int start, int position, QString * diacritic_word) {
	int last;
	return addlastDiacritics(start, position, diacritic_word, last);
}
inline QStringRef getDiacriticsBeforePosition(int pos,QString * text) {
	static const QString empty="";
	if (pos<1 && pos>text->size())
		return QStringRef(&empty);
	if (!isDiacritic(text->at(pos-1)))
		return QStringRef(&empty);
	else {
		int start=pos-1;
		while (start>0 && isDiacritic(text->at(start))) {
			start--;
		}
		return text->midRef(start+1,pos-start-1);
	}
}

inline int getPositionOneLetterBackward(int pos,QString * text) {
	int length=text->length();
	if (pos>=length)
		return length-1;
	return getLastLetter_index(*text,pos)-1;
}


#endif	/* _DIACRITICS_H */


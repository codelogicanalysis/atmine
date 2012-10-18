#ifndef _DIACRITICS_H
#define	_DIACRITICS_H

#include <QString>
#include <QChar>
#include <QRegExp>
#include "letters.h"

enum Diacritic {FATHA,KASRA,DAMMA,SHADDA,SUKUN,FATHATAYN,KASRATAYN,DAMMATAYN,ALEF_SUPERSCRIPT,UNDEFINED_DIACRITICS};

inline QChar interpret_diacritic(Diacritic d) {
	switch(d) {
	case FATHA:
		return fatha;
	case KASRA:
		return kasra;
	case DAMMA:
		return damma;
	case DAMMATAYN:
		return dammatayn;
	case FATHATAYN:
		return fathatayn;
	case KASRATAYN:
		return kasratayn;
	case SHADDA:
		return shadde;
	case SUKUN:
		return sukun;
	case ALEF_SUPERSCRIPT:
		return aleft_superscript;
	default:
		return '\0';
	}
}

inline Diacritic interpret_diacritic(QChar d) {
	if (d==fatha)
		return FATHA;
	if (d==kasra)
		return KASRA;
	if (d==damma)
		return DAMMA;
	if (d==dammatayn)
		return DAMMATAYN;
	if (d==fathatayn)
		return FATHATAYN;
	if (d==kasratayn)
		return KASRATAYN;
	if (d==shadde)
		return SHADDA;
	if (d==sukun)
		return SUKUN;
	if (d==aleft_superscript)
		return ALEF_SUPERSCRIPT;
	else
		return UNDEFINED_DIACRITICS;
}

class Diacritics {
private:
	Diacritic main:4;
	bool shadde:1;
	bool inconsistent:1;
public:
	void clear() {
		main=UNDEFINED_DIACRITICS;
		shadde=false;
		inconsistent=false;
	}
	Diacritics() {
		clear();
	}
	Diacritics(QChar c) {
		clear();
		append(c);
	}
	Diacritics(Diacritic c) {
		clear();
		append(c);
	}
	void append(Diacritic d) {
		if (d==SHADDA)
			shadde=true;
		else if (main==UNDEFINED_DIACRITICS)
			main=d;
		else if (d!=main)
			inconsistent=true;
	}

	void append(QChar c) {
		Diacritic d=interpret_diacritic(c);
		append(d);
	}
	bool operator ==(Diacritics & l) { //strict equality
		return (main==l.main && shadde==l.shadde && inconsistent==l.inconsistent);
	}
	bool isSelfConsistent() const {return !inconsistent;}
	bool isConsistent(Diacritics & l,bool forceShadde=false) {
		if (inconsistent || l.inconsistent) //i.e. diacritics not dependable
			return true;
		if (forceShadde) {
			if (shadde && !l.shadde)
				return false;
			if (main==FATHATAYN) {
				if (l.main!=FATHATAYN)
					return false;
			} else if (main==DAMMATAYN) {
				if (l.main!=DAMMATAYN)
					return false;
			} else if (main==KASRATAYN) {
				if (l.main!=KASRATAYN)
					return false;
			}
		#ifdef ALEF_SUPER_FORCE //we can force it if we are super sure lexicon has all such placees, which is not the case
			if (main==ALEF_SUPERSCRIPT && l.main!=ALEF_SUPERSCRIPT)
				return false;
		#endif
		}
		#ifndef ALEF_SUPER_FORCE
			if ((main==ALEF_SUPERSCRIPT && l.main==FATHA) ||(l.main==ALEF_SUPERSCRIPT && main==FATHA))
				return true;
		#endif
		if (main!=UNDEFINED_DIACRITICS && l.main!=UNDEFINED_DIACRITICS)
			return main==l.main;
		return true;
	}

	bool hasShadde() const { assert(!inconsistent);return shadde;}
	Diacritic getMainDiacritic() const { assert(!inconsistent);return main;}
	QString getEquivalent() const {
		if (inconsistent)
			return "X";
		QString s;
		if (shadde)
			s+=::shadde;
		if (main!=UNDEFINED_DIACRITICS)
			s+=interpret_diacritic(main);
		return s;
	}
	bool isEmpty() const { return !hasShadde() && main==UNDEFINED_DIACRITICS && isSelfConsistent(); }
};

inline bool isDiacritic(const QChar & letter) {
	if (letter==shadde || letter==shadde || letter==fatha || letter==damma || letter==kasra ||
			letter==sukun || letter==kasratayn || letter==dammatayn || letter==fathatayn ||
			letter==aleft_superscript|| letter==shadde_alef_above||letter==madda)
		return true;
	else
		return false;
}

inline int countDiacritics(const QString & s) {
	int count=0;
	for (int i=0;i<s.size();i++)
		if (isDiacritic(s[i]))
			count++;
	return count;
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
	if (position<-1 /*|| position ==diacritic_word->length()*/)
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
	if (pos<1 || pos>text->size())
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
inline QString getDiacritics(QString & s, int & pos) { //get diacritics at position and move the 'pos'
	if (pos>=s.size())
		return "";
	return addlastDiacritics(pos,pos,&s,pos).toString();
}

inline int getPositionOneLetterBackward(int pos,QString * text) {
	int length=text->length();
	if (pos>=length)
		return length-1;
	return getLastLetter_index(*text,pos)-1;
}


#endif	/* _DIACRITICS_H */


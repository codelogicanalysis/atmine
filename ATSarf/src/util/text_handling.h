#ifndef _TEXT_HANDLING_H
#define _TEXT_HANDLING_H

#include <QString>
#include <QChar>
#include <QVector>
#include <QPair>
#include "letters.h"
#include "diacritics.h"
#include "common.h"
#include "logger.h"
#include "textParsing.h"

inline QString get_Possessive_form(QString word)
{
	if (word.length()>=2)
	{
		int last_index=getLastLetter_index(word,word.length()-1);
		QChar last=_getLastLetter(word,last_index);
		QChar before=getLastLetter(word.left(last_index));
		if (last==alef && isConsonant(before))
			return removeLastDiacritics(word).append(waw).append(ya2);
		else if (last==alef && before==waw )
			return removeLastDiacritics(removeLastLetter(word)).append(ya2);
		else if (last==alef && before==ya2 )
			return removeLastDiacritics(removeLastLetter(word)).append(shadde);
		else if (last==ta2_marbouta && isConsonant(before))
			return removeLastDiacritics(removeLastLetter(word)).append(ya2);
		else if (last==ya2)
			return removeLastDiacritics(word).append(shadde);
		else if (isConsonant(last) || last==waw)
			return removeLastDiacritics(word).append(ya2);
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
		if (i<length1 && j<length2 && !equal(word1[i],word2[j])) {
		#ifdef ENABLE_RUNON_WORD_INSIDE_COMPOUND_WORD
			if (word1[i]==' ') {
				i++;
				continue;
			}
			if (word2[j]==' ') {
				j++;
				continue;
			}
		#endif
			return false;
		}
		i++;
		j++;
	}
	if (length1-(i)<=0)
	{
		for (int f=j;f<length2;f++)
			if (!isDiacritic(word2[f]))
				return false;
	}
	else if (length2-(j)<=0)
	{
		for (int f=i;f<length1;f++)
			if (!isDiacritic(word1[f]))
				return false;
	}
	else
		return false;
	return true;
}
bool checkIfSmallestIsPrefixOfLargest(const QStringRef &word1,const QStringRef &word2, int & i1, int & i2); //modifies value of i1 and i2
inline bool equal(const QStringRef &word1,const QStringRef &word2) // is diacritics tolerant and ignores punctuation
{
	int i1,i2;
	int length1=word1.length(),
		length2=word2.length();
	if (!checkIfSmallestIsPrefixOfLargest(word1,word2,i1,i2))
		return false;
	if (length1-(i1+1)<=0) {
		for (int i=i2+1;i<length2;i++)
			if (!isDiacritic(word2.at(i)) && !isPunctuationMark(word2.at(i)))
				return false;
	}
	else if (length2-(i2+1)<=0) {
		for (int i=i1+1;i<length1;i++)
			if (!isDiacritic(word1.at(i)) && !isPunctuationMark(word1.at(i)))
				return false;
	} else
		return false;
	return true;
}
inline bool equal(const QString &word1,const QString &word2) {// is diacritics tolerant
	return equal(word1.rightRef(-1),word2.rightRef(-1));//rightRef of <0 returns whole string
}
inline bool equal(const QStringRef &word1,const QString &word2) { // is diacritics tolerant
	return equal(word1,word2.rightRef(-1));//rightRef of <0 returns whole string
}
inline bool equal_withoutLastDiacritics(const QString &word1,const QString &word2) { // is diacritics tolerant
	return equal(removeLastDiacritics(word1),removeLastDiacritics(word2));
}
inline bool startsWith(const QStringRef &text,const QString &substring, int & finish_pos){ // is diacritics tolerant checks if a word ends with a delimeter terminated 'subset'
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
		if (isDelimiter(text.at(finish_pos))) {//subset but not part of a word
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


inline void skipOneLetter(const QString * text,long & currentPos) {
	if (currentPos<text->size()) {
		currentPos++;
		while (currentPos<text->size() && isDiacritic(text->at(currentPos)))
			currentPos++;
	}
}

inline bool skipAL(const QString * text, long & currPos) { //just moves currPos
	if (currPos<text->size()) {
		QChar l=text->at(currPos);
		if (l==alef) {
			long newPos=currPos;
			skipOneLetter(text,newPos);
			if (newPos<text->size()) {
				l=text->at(newPos);
				if (l==lam) {
					skipOneLetter(text,newPos);
					currPos=newPos;
					return true;
				}
			}
		}
	}
	return false;
}

inline bool startsWithAL(const QString & word)//does not take in account cases were Diacritics may be present on the alef and lam of "al"
{
	long i=0;
	return (skipAL(&word,i)); //will not affect word
}
inline bool removeAL( QString &word)
{
	long i=0;
	if (!skipAL(&word,i))
		return false;
	word=word.mid(i,word.length()); //word.length() is more than needed but no problem will be ignored supposedly
	return true;
}
inline QString withoutAL( QString word)
{
	removeAL(word);
	return word;
}

inline bool overLaps(int start1,int end1,int start2,int end2) {
	assert(start1<=end1 && start2<=end2);
	if (start1>=start2 && start1<=end2)
		return true;
	if (start2>=start1 && start2<=end1)
		return true;
	return false;
}
inline bool after(int start1,int end1,int start2,int end2) {
	assert(start1<=end1 && start2<=end2);
	if (start1>=end2)
		return true;
	return false;
}
inline bool before(int start1,int end1,int start2,int end2) {
	return after(start2,end2,start1,end1);
}

inline int countWords(QString * text, int start,int end, bool * hasParagraphPunc=NULL) {
	if (hasParagraphPunc!=NULL)
		*hasParagraphPunc=false;
	if (start>=end)
		return 0;
	int count=1;
	PunctuationInfo punc;
	while ((start=next_positon(text,getLastLetter_IN_currentWord(text,start),punc))<end) {
		count++;
		if (hasParagraphPunc!=NULL) {
			if (punc.hasParagraphPunctuation()) {
				*hasParagraphPunc=true;
			}
		}
	}
	return count;
}

inline int countWords(QString * text, const QPair<int,int> & st, bool * hasParagraphPunc=NULL) {
	int start=st.first,
		end=st.second;
	return countWords(text,start,end,hasParagraphPunc);
}

inline int commonWords(QString * text, const QPair<int,int> & st1,const QPair<int,int> & st2) {
	int start=max(st1.first,st2.first),
		end=min(st1.second,st2.second);
	return countWords(text,start,end);
}

inline int countWords(QString * text, const QList<QPair<int,int> > & st, bool * hasParagraphPunc=NULL) {
	int count=0;
	for (int i=0;i<st.size();i++) {
		count+=countWords(text,st[i],hasParagraphPunc);
	}
	return count;
}

inline int commonWords(QString * text, const QList< QPair<int,int> > & st1,const QList<QPair<int,int> > & st2) {
	//assumes sanity of lists (i.e. no such case does not exist:
	/*
	  -------- --------- --------
	  [          ]   [          ]
	*/
	//otherwise would return 4 words instead of 3
	int count=0;
	for (int i=0;i<st1.size();i++) {
		for (int j=0;j<st2.size();j++) {
			count+=commonWords(text,st1[i],st2[j]); //if has no common characters will return zero, so although not efficient is expected to return sane results
		}
	}
	return count;
}




#endif // _TEXT_HANDLING_H

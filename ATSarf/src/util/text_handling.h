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

enum RelativePos{Beggining, Middle, End};

/**
  * This method returns an enumerator of the relative position given the current position and size of an item.
  * @param  pos          position in item
  * @param  size         size of item
  * @return enumerator representing relative position
  */
inline RelativePos getRelativePos(int pos, int size) {
	assert((pos>=0 && pos<size) || (pos==size && size==0));
	if (pos==0)
		return Beggining;
	else if (pos==size-1)
		return End;
	else
		return Middle;
}

/**
  * This method returns the possessive form of an input Arabic word.
  * @param  word          Arabic word
  * @return QString containing the possessive form of word
  */
inline QString get_Possessive_form(QString word) {
	if (word.length()>=2) {
		int last_index=getLastLetter_index(word,word.length()-1);
		QChar last=_getLastLetter(word,last_index);
		QChar before=getLastLetter(word.left(last_index));
		if (last==alef && !isLongVowel(before))
			return removeLastDiacritics(word).append(waw).append(ya2);
		else if (last==alef && before==waw )
			return removeLastDiacritics(removeLastLetter(word)).append(ya2);
		else if (last==alef && before==ya2 )
			return removeLastDiacritics(removeLastLetter(word)).append(shadde);
		else if (last==ta2_marbouta && !isLongVowel(before))
			return removeLastDiacritics(removeLastLetter(word)).append(ya2);
		else if (last==ya2)
			return removeLastDiacritics(word).append(shadde);
		else if (!isLongVowel(last) || last==waw)
			return removeLastDiacritics(word).append(ya2);
		else {
                        theSarf->out << "Unknown Rule for Possessive form\n";
			return QString::null;
		}
	} else
		return word.append(ya2);
}

/**
  * This method checks if two characters are equal taking into consideration some differences in the Arabic letters.
  * @param  c1  first character
  * @param  c2  second character
  * @return Boolean indicating if the two characters are equal.
  */
inline bool equal(const QChar & c1, const QChar & c2) {
	if (c1==c2)
		return true;
	if (alefs.contains(c1) && alefs.contains(c2))
		return true;
	if ((c1==veh && c2==feh) || (c2==veh && c1==feh))
		return true;
	return false;
}

/**
  * This method checks if two Arabic words are equal while ignoring the diacritics.
  * @param  word1  first word
  * @param  word2  second word
  * @return Boolean indicating if the two words are equal or not.
  */
inline bool equal_ignore_diacritics(const QString &word1,const QString &word2) {
	int length1=word1.length(),length2=word2.length();
	int i=0,j=0;
	while(i<length1 && j<length2) {
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
	if (length1-(i)<=0) {
		for (int f=j;f<length2;f++)
			if (!isDiacritic(word2[f]))
				return false;
	} else if (length2-(j)<=0) {
		for (int f=i;f<length1;f++)
			if (!isDiacritic(word1[f]))
				return false;
	} else
		return false;
	return true;
}

/**
  * This method checks if word1 is contained in word2 with consistent diacritics.
  * This means that the letters of word1 are contained in word2 and the diacritics of the
  * corresponding letters are consistent.
  * @param  word1          first word
  * @param  word2          second word
  * @param  i1             returns the index of the letter at which the comparison stopped in word1
  * @param  i2             returns the index of the letter at which the comparison stopped in word2
  * @param  force shadde   forces the existence of a shadda in one word if it exists in the other
  * @return Boolean to indicate if first is contained in the other or not.
  */
bool checkIfFirstIsContainedInSecond(const QStringRef &word1,const QStringRef &word2, int & i1, int & i2,bool force_shadde=false); //modifies value of i1 and i2

/**
  * This method checks if one of the words word1 and word2 is a prefix of the other with consistent diacritics.
  * This means that the letters of one of the words are the prefix of the other and the diacritics of the
  * corresponding letters are consistent.
  * @param  word1          first word
  * @param  word2          second word
  * @param  i1             returns the index of the letter at which the comparison stopped in word1
  * @param  i2             returns the index of the letter at which the comparison stopped in word2
  * @param  force shadde   forces the existence of a shadda in one word if it exists in the other
  * @return Boolean to indicate if the smallest word is a prefix of the other or not.
  */
bool checkIfSmallestIsPrefixOfLargest(const QStringRef &word1,const QStringRef &word2, int & i1, int & i2,bool force_shadde=false); //modifies value of i1 and i2

/**
  * This method checks if word1 and word2 are equal with consistent diacritics.
  * This means that the letters of word1 and word2 are the same and the diacritics of the
  * corresponding letters are consistent.
  * @param  word1          first word
  * @param  word2          second word
  * @param  force shadde   forces the existence of a shadda in one word if it exists in the other
  * @param  force last     forces any additional dacritics at the end of one word to be consistent
  *                        with the other word, ex. shadda if forced.
  * @return Boolean to indicate if the words are equal or not.
  */
inline bool equal(const QStringRef &word1,const QStringRef &word2,bool force_shadde=false, bool force_last=true) { // is diacritics tolerant and ignores punctuation
	int i1,i2;
	int length1=word1.length(),
		length2=word2.length();
	if (!checkIfSmallestIsPrefixOfLargest(word1,word2,i1,i2,force_shadde))
		return false;
	if (length1-(i1+1)<=0) {
		Diacritics d2; //maybe not necessary since forcing some diacritics will not affect here
		for (int i=i2+1;i<length2;i++) {
			QChar letter=word2.at(i);
			bool isDia=isDiacritic(letter),
				 isPunc=isPunctuationMark(letter);
			if (!isDia && !isPunc)
				return false;
			else if (isDia){
				d2.append(letter);
			}
		}
		Diacritics d1;
		if (force_last && !d1.isConsistent(d2,force_shadde))
			return false;
	}
	else if (length2-(i2+1)<=0) {
		Diacritics d1;
		for (int i=i1+1;i<length1;i++) {
			QChar letter=word1.at(i);
			bool isDia=isDiacritic(letter),
				 isPunc=isPunctuationMark(letter);
			if (!isDia && !isPunc)
				return false;
			else if (isDia) {
				d1.append(letter);
			}
		}
		Diacritics d2;
		if (force_last && !d1.isConsistent(d2,force_shadde))
			return false;
	} else
		return false;
	return true;
}

inline bool equal(const QString &word1,const QString &word2,bool force_shadde=false, bool force_last=true) {// is diacritics tolerant
	return equal(word1.rightRef(-1),word2.rightRef(-1),force_shadde, force_last);//rightRef of <0 returns whole string
}

inline bool equal(const QStringRef &word1,const QString &word2,bool force_shadde=false, bool force_last=true) { // is diacritics tolerant
	return equal(word1,word2.rightRef(-1),force_shadde, force_last);//rightRef of <0 returns whole string
}

/**
  * This method checks if word1 and word2 are equal with consistent diacritics after removing the diacritics of the last letter in both words.
  * This means that the letters of word1 and word2 are the same and the diacritics of the
  * corresponding letters are consistent excluding the diacritics of the last letter from each word.
  * @param  word1          first word
  * @param  word2          second word
  * @return Boolean to indicate if the words are equal or not.
  */
inline bool equal_withoutLastDiacritics(const QString &word1,const QString &word2) { // is diacritics tolerant
	return equal(removeLastDiacritics(word1),removeLastDiacritics(word2));
}

/**
  * This method checks if text starts with substring and that the first character after the diacritics of the last matching letter is a delimiter.
  * @param  text        input text
  * @param  substring   input substring
  * @param  finish_pos  contains the end position at which the method stops in text. It includes the diacritics of the last letter.
  * @return Boolean to indicate if the text starts with the substring.
  */
inline bool startsWith(const QStringRef &text,const QString &substring, int & finish_pos){ // is diacritics tolerant checks if a word ends with a delimeter terminated 'subset'
	int text_length=text.length(), substring_length=substring.length();
	if(text_length<substring_length)
		return false;
	int i2;
	if (checkIfSmallestIsPrefixOfLargest(text,substring.rightRef(-1),finish_pos,i2)) {
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

/**
  * This method skips the first letter after the current position in the input text
  * @param  text          pointer to input text
  * @param  currentPos    reference to current position in text
  */
inline void skipOneLetter(const QString * text,long & currentPos) {
	if (currentPos<text->size()) {
		currentPos++;
		while (currentPos<text->size() && isDiacritic(text->at(currentPos)))
			currentPos++;
	}
}

/**
  * This method skips "Al" from the beginning of the input Arabic text starting from the current position.
  * @param  text       pointer to input text
  * @param  currPos    reference to current position in text
  * @return Boolean indicating if the method skipped "Al" or not
  */
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

/**
  * This method checks if the input word starts with "Al"
  * @param  word    reference to an input Arabic word
  * @return Boolean indicating if the input word starts with "Al"
  */
inline bool startsWithAL(const QString & word)//does not take in account cases were Diacritics may be present on the alef and lam of "al"
{
	long i=0;
	return (skipAL(&word,i)); //will not affect word
}

/**
  * This method removes "Al" from the input word
  * @param  word    reference to an input Arabic word
  * @return Boolean indicating if "Al" was removed or not.
  */
inline bool removeAL( QString &word)
{
	long i=0;
	if (!skipAL(&word,i))
		return false;
	word=word.mid(i,word.length()); //word.length() is more than needed but no problem will be ignored supposedly
	return true;
}

/**
  * This method removes "Al" from the input word
  * @param  word    input Arabic word
  * @return QString without "Al".
  */
inline QString withoutAL( QString word)
{
	removeAL(word);
	return word;
}

/**
  * This method removes the first word from the input word. It identifies the first word from the first space in s.
  * @param  s    reference to an input Arabic text
  */
inline void removeFirstWord(QString & s) {
	int l=s.indexOf(' ');
	if (l>=0)
		s.remove(0,l+1);
}


/**
  * This method checks of the two intervals have an overlap.
  * @param  start1  start of first interval
  * @param  end1    end of first interval
  * @param  start2  start of second interval
  * @param  end2    end of second interval
  * @return Boolean indicating if the two intervals overlap
  */
inline bool overLaps(int start1,int end1,int start2,int end2) {
	assert(start1<=end1 && start2<=end2);
	if (start1>=start2 && start1<=end2)
		return true;
	if (start2>=start1 && start2<=end1)
		return true;
	return false;
}

/**
  * This method checks of interval [start1, end1] is after [start2, end2].
  * @param  start1  start of first interval
  * @param  end1    end of first interval
  * @param  start2  start of second interval
  * @param  end2    end of second interval
  * @return Boolean indicating if the first interval comes after the second
  */
inline bool after(int start1,int end1,int start2,int end2) {
	assert(start1<=end1 && start2<=end2);
	if (start1>=end2)
		return true;
	return false;
}

/**
  * This method checks of interval [start1, end1] is before [start2, end2].
  * @param  start1  start of first interval
  * @param  end1    end of first interval
  * @param  start2  start of second interval
  * @param  end2    end of second interval
  * @return Boolean indicating if the first interval comes before the second
  */
inline bool before(int start1,int end1,int start2,int end2) {
	return after(start2,end2,start1,end1);
}

/**
  * This method counts the number of words in text between start and end.
  * @param  text                pointer to the input text
  * @param  start               start position
  * @param  end                 end position
  * @param  hasParagraphPunc    pointer to a Boolean which is set by the method to indicate if the input text has full stop and newline in the specified interval.
  * @return integer containing the number of words
  */
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

/**
  * This method counts the number of words in text present in the common interval between st1 and st2.
  * @param  text                pointer to the input text
  * @param  st1                 QPair defining the first interval
  * @param  st2                 QPair defining the second interval
  * @return integer containing the number of words
  */
inline int commonWords(QString * text, const QPair<int,int> & st1,const QPair<int,int> & st2) {
	int start=max(st1.first,st2.first),
		end=min(st1.second,st2.second);
	return countWords(text,start,end);
}

/**
  * This method counts the number of words in text between multiple intervals.
  * @param  text                pointer to the input text
  * @param  st                  list which contains a set of pairs indicating start and end of each interval.
  * @param  hasParagraphPunc    pointer to a Boolean which is set by the method to indicate if the input text has full stop and newline in the specified interval.
  * @return integer containing the number of words
  */
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

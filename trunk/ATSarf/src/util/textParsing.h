#ifndef TEXTPARSING_H
#define TEXTPARSING_H
#include "hadith.h"
#include "letters.h"

class PunctuationInfo {
public:
	bool has_punctuation:1;
	bool comma:1;
	bool semicolon:1;
	bool fullstop:1;
	bool newLine:1;
	PunctuationInfo() {
		reset();
	}
	void reset() {
		has_punctuation=false;
		comma=false;
		semicolon=false;
		fullstop=false;
		newLine=false;
	}
	bool hasEndingPunctuation() { return fullstop || newLine;}
	bool update(const QChar & letter) { //returns true if this letter is a delimiter
		if (non_punctuation_delimiters.contains(letter))
			return true;
		else if (punctuation.contains(letter)) {
			has_punctuation=true;
			if (letter==',' || letter ==fasila)
				comma=true;
			else if (letter=='\n' || letter =='\r' || letter==paragraph_seperator)
				newLine=true;
			else if (letter==semicolon_ar || letter ==';')
				semicolon=true;
			else if (letter==full_stop1 || letter==full_stop2 || letter==full_stop3 || letter==question_mark || letter=='.' || letter=='?')
				fullstop=true;
			return true;
		} else
			return false;
	}
};

inline long next_positon(QString * text,long finish,PunctuationInfo & punctuationInfo) {
	punctuationInfo.reset();
	int size=text->length();
	if (finish>=size)
		return finish+1;//check this
	QChar letter=text->at(finish);
#ifdef PUNCTUATION
	punctuationInfo.update(letter);
#endif
	finish++;
	while(finish<size) {
		letter=text->at(finish);
	#ifdef PUNCTUATION
		if (!punctuationInfo.update(letter)) // update returns true if letter is a delimiter
			break;
	#else
		if (!delimiters.contains(letter))
			break;
	#endif
		finish++;
	}
	return finish;
}

inline long getLastLetter_IN_previousWord(QString * text,long start_letter_current_word) {
	start_letter_current_word--;
	while(start_letter_current_word>=0 && isDelimiter(text->at(start_letter_current_word)))
		start_letter_current_word--;
	return start_letter_current_word;
}

inline long getLastLetter_IN_currentWord(QString * text,long start_letter_current_word) {
	int size=text->length();
#if 0
	if (!isDelimiter(text->at(start_letter_current_word)))
		start_letter_current_word++;
#endif
	while(start_letter_current_word<size)
	{
		if(!isDelimiter(text->at(start_letter_current_word)))
			start_letter_current_word++;
		else
		{
			start_letter_current_word--;
			break;
		}
	}
	return start_letter_current_word;
}

inline bool isNumber(QString * text,long & currentPos,long & finish) {
	bool ret_val=false;
	long size=text->size();
	long i;
	for (i=currentPos;i<size;i++) {
		if (isNumber(text->at(i)))
			ret_val=true;
		else {
			finish=i-1;
			return ret_val;
		}
	}
	finish=i; //means (still number and text finished) or did not enter the loop at all
	return ret_val;
}

#endif // TEXTPARSING_H

#ifndef TEXTPARSING_H
#define TEXTPARSING_H
#include "hadith.h"

inline long next_positon(QString * text,long finish,bool & has_punctuation) {
	has_punctuation=false;
#ifdef PUNCTUATION
	if (finish<text->length() && punctuation.contains(text->at(finish)))
		has_punctuation=true;
#endif
	finish++;
	while(finish<text->length())
	{
	#ifdef PUNCTUATION
		if (punctuation.contains(text->at(finish)))
			has_punctuation=true;
		else if (!non_punctuation_delimiters.contains(text->at(finish)))
			break;
	#else
		if (!delimiters.contains(text->at(finish)))
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

#include "text_handling.h"
#include "diacritics.h"
#include <QString>
#include <QChar>
#include <QList>
//#include <QVector>
#include <QDebug>

/* (rules for possessive)
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
//#define COUNT_RUNON
#ifdef COUNT_RUNON
	extern bool runon;
#endif

inline bool equal(Diacritics & list1,Diacritics & list2, bool forceShadde=false) {
	bool l=list1.isConsistent(list2,forceShadde);
#if 0
	if (forceShadde)
		out<<list1.getEquivalent()<<"\t"<<list2.getEquivalent()<<"\t"<<l<<"\n";
#endif
	return l;
}

bool checkIfSmallestIsPrefixOfLargest(const QStringRef &word1,const QStringRef &word2, int & i1, int & i2, bool force_shadde) {//modifies value of i1 and i2
	//qDebug() << word1<<"-"<<word2;
	int length1=word1.count();
	int length2=word2.count();
	i1=-1;i2=-1;
	Diacritics diacritics1,diacritics2;
	QChar letter1,letter2;
	while (i1+1<length1 && i2+1<length2) {
		i1++;
		i2++;
		diacritics1.clear();
		diacritics2.clear();
		while (i1<length1 && isDiacritic(word1.at(i1))) {
			if (/*word1.at(i1)!=shadde && */word1.at(i1)!=aleft_superscript)
				diacritics1.append(word1.at(i1));
			i1++;
		}
		while (i1<length1 && isPunctuationMark(word1.at(i1)))
			i1++;
		if (i1<length1)
			letter1=word1.at(i1);
		else
			letter1='\0';
		while (i2<length2 && isDiacritic(word2.at(i2))) {
			if (/*word2.at(i2)!=shadde && */word2.at(i2)!=aleft_superscript)
				diacritics2.append(word2.at(i2));
			i2++;
		}
		while (i2<length2 && isPunctuationMark(word2.at(i2)))
			i2++;
		if (i2<length2)
			letter2=word2.at(i2);
		else
			letter2='\0';
		//now comparison first diacritics then next_letter
		if (equal(diacritics1,diacritics2,force_shadde)) {
			if (equal(letter1,letter2))
				continue;
			else {
			#ifdef ENABLE_RUNON_WORD_INSIDE_COMPOUND_WORD
				if (letter1==' ') {
					i2--;//bc will be incremented later so this in effect only moves i1 and keeps i2 in its place
				#ifdef COUNT_RUNON
					runon=true;
				#endif
					continue;
				}
				if (letter2==' ') {
					i1--;//bc will be incremented later so this in effect only moves i2 and keeps i1 in its place
				#ifdef COUNT_RUNON
					runon=true;
				#endif
					continue;
				}
			#endif
				return false;
			}
		}
		return false;
	}
	if (length1-(i1+1)<=0 || length2-(i2+1)<=0)
		return true;
	return false;
}

bool checkIfFirstIsContainedInSecond(const QStringRef &word1,const QStringRef &word2, int & i1, int & i2, bool force_shadde) {//modifies value of i1 and i2
        //qDebug() << word1<<"-"<<word2;
        int length1=word1.count();
        int length2=word2.count();
        i1=-1;i2=-1;
        Diacritics diacritics1,diacritics2;
        QChar letter1,letter2;
        while (i1+1<length1 && i2+1<length2) {
                i1++;
                i2++;
                diacritics1.clear();
                diacritics2.clear();
                while (i1<length1 && isDiacritic(word1.at(i1))) {
                        if (/*word1.at(i1)!=shadde && */word1.at(i1)!=aleft_superscript)
                                diacritics1.append(word1.at(i1));
                        i1++;
                }
                while (i1<length1 && isPunctuationMark(word1.at(i1)))
                        i1++;
                if (i1<length1)
                        letter1=word1.at(i1);
                else
                        letter1='\0';
                while (i2<length2 && isDiacritic(word2.at(i2))) {
                        if (/*word2.at(i2)!=shadde && */word2.at(i2)!=aleft_superscript)
                                diacritics2.append(word2.at(i2));
                        i2++;
                }
                while (i2<length2 && isPunctuationMark(word2.at(i2)))
                        i2++;
                if (i2<length2)
                        letter2=word2.at(i2);
                else
                        letter2='\0';
                //now comparison first diacritics then next_letter
                if (equal(diacritics1,diacritics2,force_shadde)) {
                    if(i1+1 == length1) {
                        return true;
                    }
                    if (equal(letter1,letter2)) {
                        continue;
                    }else {
                        #ifdef ENABLE_RUNON_WORD_INSIDE_COMPOUND_WORD
                                if (letter1==' ') {
                                        i2--;//bc will be incremented later so this in effect only moves i1 and keeps i2 in its place
                                #ifdef COUNT_RUNON
                                        runon=true;
                                #endif
                                        continue;
                                }
                                if (letter2==' ') {
                                        i1--;//bc will be incremented later so this in effect only moves i2 and keeps i1 in its place
                                #ifdef COUNT_RUNON
                                        runon=true;
                                #endif
                                        continue;
                                }
                        #endif
                                i1--;
                    }
                }
                else {
                    i1--;
                }
        }
        return false;
}

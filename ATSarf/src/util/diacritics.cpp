#include "diacritics.h"

#include <QString>
#include <QChar>

#include <QDebug>

//utility functions
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

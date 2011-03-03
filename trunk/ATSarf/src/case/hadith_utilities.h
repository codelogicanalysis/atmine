#ifndef HADITH_UTILITIES_H
#define HADITH_UTILITIES_H

#include "hadith.h"

#ifdef REFINEMENTS
#include <QString>
#include "text_handling.h"
inline bool isRasoul(const QString & word)
{
	QString c;
	foreach (c, rasoul_words)
		if (startsWith(word,c))
			return true;
	return false;
}

inline bool areRasoul(const QString & word1,const QString & word2)
{
	QString c;
	bool eq1=false, eq2=false;
	foreach (c, rasoul_words)
	{
		if (startsWith(word1,c))
			eq1=true;
		if (startsWith(word2,c))
			eq2=true;
		if (eq1 && eq2) //both are rasoul words
			return true;
	}
	return false;
}
#endif

#endif // HADITH_UTILITIES_H

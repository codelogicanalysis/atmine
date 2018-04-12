#ifndef EDITDISTANCE_H
#define EDITDISTANCE_H

#include <QChar>

#include "text_handling.h"

/// Class to compute the Levenshtein edit distance between two strings.
class EditDistance
{
private:
	static bool CharComparer(const QChar & a,const QChar & b) {
		return equal(a,b);
	}
public:
	/// Computes the Levenshtein distance between two strings.
	static int Compute(QString a, QString b){
		return Compute(a, b, false);
	}
	/// Computes the Levenshtein distance between two strings.
	static int Compute(QString a, QString b, bool ignoreDiacritics);

};

#endif // EDITDISTANCE_H

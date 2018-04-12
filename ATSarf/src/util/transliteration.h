#ifndef TRANSLITERATION_H
#define TRANSLITERATION_H

#include <QString>

class Buckwalter {
private:
	#define Buckwalter_LETTER_COUNT 51
	static const QChar buckwalter [Buckwalter_LETTER_COUNT];
	static const int unicode[Buckwalter_LETTER_COUNT];
public:
	static QString convertTo (QString s) {
		for (int i=0;i<Buckwalter_LETTER_COUNT;i++){
			s.replace(QChar(unicode[i]),buckwalter[i]);
		}
		return s;
	}

	static QString convertFrom (QString s) {
		for (int i=0;i<Buckwalter_LETTER_COUNT;i++){
			s.replace(buckwalter[i],QChar(unicode[i]));
		}
		return s;
	}
#undef count
};

#endif // TRANSLITERATION_H

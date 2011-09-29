#ifndef _LETTERS_H
#define	_LETTERS_H

#include <QChar>
#include <QVector>
#include "common.h"

//constantletters
static const QChar hamza=QChar(0x0621);
static const QChar ya2=QChar(0x064A);
static const QChar alef=QChar(0x0627);
static const QChar alef_madda_above= QChar(0x0622);
static const QChar alef_hamza_above= QChar(0x0623);
static const QChar alef_hamza_below= QChar(0x0625);
static const QChar alef_wasla=QChar(0x0671);
static const QChar ta2_marbouta=QChar(0x0629);
static const QChar waw=QChar(0x0648);
static const QChar waw_hamza_above=QChar(0x0624);
static const QChar shadde=QChar(0x0651);
static const QChar shadde_alef_above=QChar(0xFC63);
static const QChar fatha=QChar(0x064E);
static const QChar damma=QChar(0x064F);
static const QChar kasra=QChar(0x0650);
static const QChar sukun=QChar(0x0652);
static const QChar lam=QChar(0x0644);
static const QChar kasratayn=QChar(0x064D);
static const QChar dammatayn=QChar(0x064C);
static const QChar fathatayn=QChar(0x064B);
static const QChar kha2=QChar(0x062E);
static const QChar ba2=QChar(0x0628);
static const QChar ra2=QChar(0x0631);
static const QChar noon=QChar(0x0646);
static const QChar seen=QChar(0x0633);
static const QChar meem=QChar(0x0645);
static const QChar _3yn=QChar(0x0639);
static const QChar ta2=QChar(0x062A);
static const QChar qaf=QChar(0x0642);
static const QChar _7a2=QChar(0x062D);
static const QChar dal=QChar(0x062F);
static const QChar tha2=QChar(0x062B);
static const QChar aleft_superscript=QChar(0x0670);
static const QChar ha2 =QChar(0x0647);
static const QChar veh =QChar(0x06A4);
static const QChar feh =QChar(0x0641);
static const QChar madda =QChar(0x0653);
static const QChar zain =QChar(0x0632);
static const QChar thal =QChar(0x0630);
static const QChar alef_maksoura= QChar(0x0649);

static const QChar question_mark=QChar(0x061F);
static const QChar semicolon_ar=QChar(0x061B);
static const QChar colon_raised=QChar(0x02F8);
static const QChar colon_modifier=QChar(0xA789);
static const QChar fasila=QChar(0x060C);
static const QChar full_stop1=QChar(0x06D4);
static const QChar full_stop2=QChar(0x200F);
static const QChar full_stop3=QChar(0x002E);
static const QChar double_quotation1=QChar(0x00BB);//RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
static const QChar double_quotation2=QChar(0x00AB);//LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
static const QChar double_quotation3=QChar(0x02DD);//DOUBLE ACUTE ACCENT
static const QChar double_quotation4=QChar(0x02EE);//MODIFIER LETTER DOUBLE APOSTROPHE
static const QChar double_quotation5=QChar(0x02F5);//MODIFIER LETTER MIDDLE DOUBLE GRAVE ACCENT
static const QChar double_quotation6=QChar(0x02F6);//MODIFIER LETTER MIDDLE DOUBLE ACUTE ACCENT
static const QChar single_quotation1=QChar(0x02BB);//MODIFIER LETTER TURNED COMMA
static const QChar single_quotation2=QChar(0x02BC);//MODIFIER LETTER APOSTROPHE
static const QChar single_quotation3=QChar(0x02BD);//MODIFIER LETTER REVERSED COMMA
static const QChar single_quotation4=QChar(0x02CA);//MODIFIER LETTER ACUTE ACCENT
static const QChar single_quotation5=QChar(0x02CB);//MODIFIER LETTER GRAVE ACCENT

static const QChar paragraph_seperator=QChar(0x2029);//PARAGRAPH SEPARATOR
static const QChar zero_width_space=QChar(0xFEFF);//ZERO WIDTH NO-BREAK SPACE

static const QString delimiters=QString("[!_ :\\.,()-><{}\\/|'\"\n\t\r")+fasila+question_mark+semicolon_ar+full_stop1+full_stop2+full_stop3+
								double_quotation1+double_quotation2+double_quotation3+double_quotation4+double_quotation5+double_quotation6+
								single_quotation1+single_quotation2+single_quotation3+single_quotation4+single_quotation5+paragraph_seperator+
								zero_width_space+"\\[\\]]";
static const QString punctuation=QString(":!_\\.,()-'\n\r\"")+fasila+question_mark+semicolon_ar+full_stop1+full_stop2+full_stop3+paragraph_seperator;
static const QString alefs=QString("")+alef+alef_hamza_above+alef_hamza_below+alef_madda_above+alef_wasla;
static const QString nonconnectingLetters=QString("")+alef+alef_hamza_above+alef_hamza_below+alef_madda_above+alef_wasla+aleft_superscript+
		waw_hamza_above+waw+hamza+zain+ra2+thal+dal+ta2_marbouta+alef_maksoura;
extern QString non_punctuation_delimiters;

inline bool isDelimiter(const QChar & letter) {
	int uni=letter.unicode();
	if (uni==0)
		return false;
	if (uni<0x0020)
		return true;
	return delimiters.contains(letter);
}

inline bool isPunctuationMark(const QChar & letter) { return punctuation.contains(letter); }

inline bool isNonConnectingLetter(const QChar & letter) { return nonconnectingLetters.contains(letter); }

inline bool isNumber(const QChar & letter) { return letter<='9' && letter >='0';}

inline bool isDelimiterOrNumber(const QChar & letter) { return isNumber(letter) || isDelimiter(letter);}
#endif	/* _LETTERS_H */


#include <QFile>
#include <QRegExp>
#include <QStringList>
#include "test.h"
#include "../logger/logger.h"
#include "../test-cases/hadith.h"
#include "../builders/functions.h"
#include "../sarf/stemmer.h"
#include "../caching_structures/database_info_block.h"

int word_sarf_test()
{

///hhh

		QString word;
		in >>word;
		//	out<<string_to_bitset(word).to_string().data()<<"	 "<<bitset_to_string(string_to_bitset(word))<<"\n";*/
		Stemmer stemmer(word);
		stemmer();
		return 0;



}
int augment()
{
	if (insert_buckwalter()<0)
		return -1;
	if (insert_rules_for_Nprop_Al())
		return -1;
	if (insert_propernames()<0)
		return -1;
	if (insert_placenames()<0)
		return -1;
	return 0;
}

bool first_time=true;
int start(QString input_str, QString &output_str, QString &error_str)
{


	out.setString(&output_str);
	out.setCodec("utf-8");
	in.setString(&input_str);
	in.setCodec("utf-8");
	displayed_error.setString(&error_str);
	displayed_error.setCodec("utf-8");

#if defined(HADITH) || defined(WORD_SARF)
	if (first_time)
	{
		database_info.fill();
		first_time=false;
#ifdef HADITH
		hadith_initialize();
#endif
	}
#endif
#if defined(HADITH)
	if (hadith(input_str))
		return -1;
#elif defined(WORD_SARF)
	if (word_sarf_test())
		return -1;
#elif defined(AUGMENT_DICTIONARY)
	if (augment()<0)
		return -1;
#endif
	return 0;
}



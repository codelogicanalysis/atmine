#include <QFile>
#include <QRegExp>
#include <QStringList>
#include <QString>
#include <QDateTime>
#include "test.h"
#include "logger.h"
#include "hadith.h"
#include "functions.h"
#include "stemmer.h"
#include "database_info_block.h"

#include "diacritics.h"

int word_sarf_test(QString input_str)
{
	QString line=input_str.split('\n')[0];
	Stemmer stemmer(&line,0);
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
int start(QString input_str, QString &output_str, QString &error_str, QString &hadith_str, bool had,ATMProgressIFC *prg)
{
	out.setString(&output_str);
	out.setCodec("utf-8");
	in.setString(&input_str);
	in.setCodec("utf-8");
	displayed_error.setString(&error_str);
	displayed_error.setCodec("utf-8");
	hadith_out.setString(&hadith_str);
	hadith_out.setCodec("utf-8");

	initializa_variables();
#ifndef AUGMENT_DICTIONARY
	if (first_time)
	{
		database_info.fill(prg);
		first_time=false;
		hadith_initialize();
	}
	out<<QDateTime::currentDateTime().time().toString()<<"\n";
#if 1
	if (had && hadith(input_str,prg))
		return -1;
	if (!had && word_sarf_test(input_str))
		return -1;
#else //testing
	int i,j;
	QString s;
	in >>s>>i>>j;
	out <<addlastDiacritics(i,j,&s);
#endif
#else
	if (augment()<0)
		return -1;
#endif

	out<<QDateTime::currentDateTime().time().toString()<<"\n";

	return 0;
}


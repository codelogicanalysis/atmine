#include <QFile>
#include <QStringList>
#include <QString>
#include <QDateTime>
#include "test.h"
#include "logger.h"
#include "hadith.h"
#include "functions.h"
#include "stemmer.h"
#include "database_info_block.h"
#include "text_handling.h"
#include "diacritics.h"
#include <sys/time.h>

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
#ifndef JUST_BUCKWALTER
	if (insert_rules_for_Nprop_Al())
		return -1;
	if (insert_propernames()<0)
		return -1;
	if (insert_placenames()<0)
		return -1;
#endif
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

#ifndef AUGMENT_DICTIONARY
#if 0
	if (first_time)
	{
		database_info.fill(prg);
		first_time=false;
		hadith_initialize();
	}
#endif
	for (int i=0;i<REPETITIONS;i++)
	{
		timeval tim;
		gettimeofday(&tim,NULL);
		double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
	#if 1
		if (had && hadith(input_str,prg))
			return -1;
		if (!had && word_sarf_test(input_str))
			return -1;
	#else //testing
		QString s1,s2;
		in >>s1>>s2;
		out <<equal_ignore_diacritics(s1,s2);
	#endif
#else
		initialize_variables();
		if (augment()<0)
			return -1;
#endif
#ifndef AUGMENT_DICTIONARY
		gettimeofday(&tim, NULL);
		double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
#if REPETITIONS<2
		out	<<"elapsed time="<<t2-t1<<"s\n";
#else
		out	<<t2-t1<<"\n";
#endif
	}
#endif
	return 0;
}



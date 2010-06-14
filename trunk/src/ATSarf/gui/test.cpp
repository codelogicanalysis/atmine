#include <QFile>
#include <QRegExp>
#include <QStringList>
#include <QString>
#include "test.h"
#include "../logger/logger.h"
#include "../test-cases/hadith.h"
#include "../builders/functions.h"
#include "../sarf/stemmer.h"
#include "../caching_structures/database_info_block.h"

#include "../utilities/diacritics.h"

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
#ifdef GUI_SPECIFIC
int start(QString input_str, QString &output_str, QString &error_str, QString &hadith_str, bool had,Ui::MainWindow *m_ui)
#else
int start(QString input_str, QString &output_str, QString &error_str, QString &hadith_str, bool had)
#endif
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
#ifdef GUI_SPECIFIC
		database_info.fill(m_ui);
#else
		database_info.fill();
#endif
		first_time=false;
		hadith_initialize();
	}
#if 1
#ifdef GUI_SPECIFIC
	if (had && hadith(input_str,m_ui))
#else
	if (had && hadith(input_str))
#endif
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
	return 0;
}


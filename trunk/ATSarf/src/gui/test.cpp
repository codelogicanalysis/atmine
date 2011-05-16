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

extern void splitRecursiveAffixes();
extern void drawAffixGraph(item_types type);
extern void listAllAffixes(item_types type);
extern int timeRecognizeHelper(QString input_str,ATMProgressIFC *prg);

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

int morphology(QString input_str,ATMProgressIFC *) {
#ifndef AUGMENT_DICTIONARY
	#if 1
		if (word_sarf_test(input_str))
			return -1;
	#else //testing
		QString s1,s2;
		in >>s1>>s2;
		out <<equal_ignore_diacritics(s1,s2);
	#endif
#else
		if (augment()<0)
			return -1;
#endif
	return 0;
}
int hadith(QString input_str,ATMProgressIFC * prg) {
	for (int i=0;i<REPETITIONS;i++)
	{
		timeval tim;
		gettimeofday(&tim,NULL);
		double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
		if (hadithHelper(input_str,prg))
			return -1;
		gettimeofday(&tim, NULL);
		double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
	#if REPETITIONS<2
		out	<<"elapsed time="<<t2-t1<<"s\n";
	#else
		out	<<t2-t1<<"\n";
	#endif
	}
	return 0;
}
int test(QString ,ATMProgressIFC *)
{
	int source_id=insert_source("Lama and Kawthar Time Categories","Manual Work","Kawthar Ali & Lama Ghusn");
	long abstract_Time=insert_category("TIME",STEM,dbitvec(max_sources),true); //returns id if already present
	QString file_name="../../dic/T/all.txt";
	QFile input(file_name);
	if (!input.open(QIODevice::ReadWrite))
	{
		out << "Unexpected Error: File not found\n";
		return 1;
	}
	QTextStream file(&input);
	file.setCodec("utf-8");
	int line_num=0;
	while (!file.atEnd())
	{
		line_num++;
		QString line=file.readLine(0);
		if (line.isNull())
		{
			line_num--; //finished
			break;
		}
		if (line.isEmpty()) //ignore empty lines if they exist
			continue;
		QList<long> * abstract_categories=new QList<long>();
		abstract_categories->append(abstract_Time);
		QStringList entries=line.split("\t",QString::KeepEmptyParts);
		if (entries.size()<5)
		{
			out<<"Error at line "<<line_num<<": '"<<line<<"'\n";
			return -1;
		}
		QString item=entries[0];
		QString raw_data=entries[1];
		QString category=entries[2];
		QString description=entries[3];
		QString POS=entries[4];
		if (!addAbstractCategory(item,raw_data,category,source_id,abstract_categories,description,POS))
		{
			out<<"Error at line "<<line_num<<": '"<<line<<"'\n";
			return -1;
		}
	}
	out <<QString("\nSuccessfully processed all %1 %2 entries\n").arg(line_num).arg(file_name);
	input.close();
	return 0;

}
int verify(QString ,ATMProgressIFC *)
{
	//drawAffixGraph(PREFIX);
	drawAffixGraph(SUFFIX);
	listAllAffixes(SUFFIX);
	listAllAffixes(PREFIX);
	return 0;
}
int breakAffix(QString, ATMProgressIFC *) {
	splitRecursiveAffixes();
	return 0;
}
int timeRecognize(QString input_str, ATMProgressIFC * prg) {
	return timeRecognizeHelper(input_str,prg);
}

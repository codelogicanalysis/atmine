#include <QFile>
#include <QStringList>
#include <QString>
#include <QDateTime>
#include <QMessageBox>
#include "test.h"
#include "logger.h"
#include "hadith.h"
#include "functions.h"
#include "stemmer.h"
#include "database_info_block.h"
#include "text_handling.h"
#include "diacritics.h"
#include "timeRecognizer.h"
#include "bibleGeneology.h"
#include <sys/time.h>
#include "vocalizedCombinations.h"
#include "transliteration.h"


extern void splitRecursiveAffixes();
extern void drawAffixGraph(item_types type);
extern void listAllAffixes(item_types type);
extern int timeTagger(QString input_str);
extern int deserializeGraph(QString fileName,ATMProgressIFC * prg);
extern int mergeGraphs(QString file1,QString file2,ATMProgressIFC * prg);
extern int bibleTagger(QString input_str);
extern int hadithTagger(QString input_str);
extern int atb(QString inputString,ATMProgressIFC * prg);
extern int atb2(QString inputString,ATMProgressIFC * prg);
extern int atbDiacritic(QString inputString,ATMProgressIFC * prg);
extern void diacriticDisambiguationCount(item_types t, int numDiacritics=1);
extern void diacriticDisambiguationCount(QString fileName, int numDiacritics, ATMProgressIFC * prg, QString reducedFile="reducedOutput", QString allFile="fullOutput");
extern void diacriticDisambiguationCount(QStringList & list, int numDiacritics, ATMProgressIFC * prg);
extern int mada(QString folderName, ATMProgressIFC * prg);
extern int diacriticStatistics(QString inputString, ATMProgressIFC * prg);
extern int regressionTest(QString inputString, ATMProgressIFC * prg);

int word_sarf_test(QString input_str){
	QString line=input_str.split('\n')[0];
	Stemmer stemmer(&line,0);
	stemmer();
	return 0;
}
int augment(){
#if !defined(INSERT_ONLY_TIME) && !defined(INSERT_ONLY_NAMES)
	if (insert_buckwalter()<0)
		return -1;
#endif
#ifndef JUST_BUCKWALTER
#ifndef INSERT_ONLY_TIME
#ifndef INSERT_ONLY_NAMES
	if (insert_rules_for_Nprop_Al())
		return -1;
	if (insert_placenames()<0)
		return -1;
#endif
	if (insert_propernames()<0)
		return -1;
#endif
#ifndef INSERT_ONLY_NAMES
	if (insert_time_categorizations()<0)
		return -1;
#endif
#endif
	return 0;
}

int morphology(QString input_str,ATMProgressIFC *) {
	if (word_sarf_test(input_str))
		return -1;
	return 0;
}
int hadith(QString input_str,ATMProgressIFC * prg) {
	for (int i=0;i<REPETITIONS;i++) {
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
int genealogy(QString inputString,ATMProgressIFC *prg) {
	if (genealogyHelper(inputString,prg))
		return -1;
	return 0;
}

int biography(QString inputString,ATMProgressIFC *prg) {
	if (deserializeGraph(inputString,prg))
		return -1;
	return 0;
}
int simple_annotation(QString inputString,ATMProgressIFC *) {
	if (timeTagger(inputString))
		return -1;
	return 0;
}

int bible_annotation(QString inputString,ATMProgressIFC *) {
	if (bibleTagger(inputString))
		return -1;
	return 0;
}

int hadith_annotation(QString inputString,ATMProgressIFC *) {
	if (hadithTagger(inputString))
		return -1;
	return 0;
}

int test(QString inputString,ATMProgressIFC * prg) {
#if 0
	if (regressionTest(inputString,prg))
		return -1;
#elif 0
	if (mada(inputString,prg))
		return -1;
#elif 0
	out<<Buckwalter::convertTo(inputString);

#elif 0
	if (diacriticStatistics(inputString, prg))
		return -1;
#elif 0
	QStringList entries=inputString.split("\t");
	diacriticDisambiguationCount(entries,-1,prg);
	//out<<equal(first, second);
#elif 1
	for (int i=0;i<10;i++) {
		out<<i<<":\n";
		VocalizedCombinationsGenerator c(inputString,i);
		if (c.isUnderVocalized())
			out<<"\t---\n";
		for (c.begin();!c.isFinished();++c) {
			VocalizedCombination comb=c.getCombination();
			out<<"\t"<<comb.getString();
			QList<Diacritics> list=comb.getDiacritics();
			out<<"\t| ";
			for (int j=0;j<list.size();j++)
				out<<list[j].getEquivalent()<<" | ";
			out<<"\n";
		}
	}
#elif defined(SUBMISSION)
	QMessageBox msg;
	msg.setWindowTitle("Sarf");
	msg.setText("Test Options Unavailable for submission version of software.");
	msg.exec();
#elif defined(DIACRITIC_DISAMBIGUATION)
#if 0
	displayed_error<<"Prefix:\n";
	out<<"\n\nPrefix:\n";
	diacriticDisambiguationCount(PREFIX);
	displayed_error<<"Stem:\n";
	out<<"\n\nStem:\n";
	diacriticDisambiguationCount(STEM);
	displayed_error<<"Suffix:\n";
	out<<"\n\nSuffix:\n";
	diacriticDisambiguationCount(SUFFIX);
#else
	displayed_error<<"\nFull:\n";
	int count=1;
	displayed_error<<"\nFull ("<<count<<"): \n";
	//out<<"\n\nFull:\n";
	QString name=QString("%1dia").arg(count>=0?QString("%1").arg(count):"*");
	diacriticDisambiguationCount(inputString,count,prg, name+".reduced.txt",name+".all.txt");

#endif
#elif  defined(ATB)
	if (atb(inputString,prg)<0)
		return -1;
#elif  defined(ATB2)
	if (atb2(inputString,prg)<0)
		return -1;
#elif  defined(ATB_DIACRITIC)
	if (atbDiacritic(inputString,prg)<0)
		return -1;
#elif defined(AUGMENT_DICTIONARY)
	if (augment()<0)
		return -1;
#elif 0
	if (biographyHelper(inputString,prg))
		return -1;
#else
	QString fileName2=prg->getFileName();
	if (fileName2=="")
		return -1;
	if (mergeGraphs(inputString,fileName2,prg))
		return -1;
#endif
	return 0;
}
int verify(QString ,ATMProgressIFC *) {
	//drawAffixGraph(PREFIX);
	//drawAffixGraph(SUFFIX);
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

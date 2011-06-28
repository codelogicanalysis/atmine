#include "hadithCommon.h"
#include <QStringList>

HadithParameters hadithParameters;

#ifdef REFINEMENTS
	QStringList rasoul_words;
	QStringList compound_words,suffixNames;
#endif

	QString chainDataStreamFileName= ".chainOutput";
#ifdef PREPROCESS_DESCRIPTIONS
	QString preProcessedDescriptionsFileName= ".HadithPreProcessedDescriptions";
#endif
#ifndef SUBMISSION
	QString PhrasesFileName="../src/case/phrases";
	QString StopwordsFileName="../src/case/stop_words";
#else
	QString PhrasesFileName=".phrases";
	QString StopwordsFileName=".stop_words";
#endif

#ifdef COUNT_RUNON
	bool runon;
#endif

	QString hadath,abid,alrasoul,abyi,_3an;
	int bit_POSSESSIVE, bit_PLACE,bit_CITY,bit_COUNTRY,bit_NOUN_PROP;
	QList<int> bits_NAME;

#ifdef PREPROCESS_DESCRIPTIONS
	QHash<long,bool> familyNMC_descriptions;
	QHash<long,bool> NRC_descriptions;
	QHash<long,bool> IBN_descriptions;
#endif
#ifdef COMPARE_TO_BUCKWALTER
	QTextStream * myoutPtr;
#endif
#ifdef COUNT_AVERAGE_SOLUTIONS
	long total_solutions=0;
	long stemmings=0;
#endif


#ifdef PREPROCESS_DESCRIPTIONS
void readFromDatabasePreProcessedDescriptions() {
	Retrieve_Template nrc_s("description","id","name='said' OR name='say' OR name='notify/communicate' OR name LIKE '%/listen' OR name LIKE 'listen/%' OR name LIKE 'listen %' OR name LIKE '% listen' OR name = 'listen' OR name LIKE '%/inform' OR name LIKE 'inform/%' OR name LIKE 'inform %' OR name LIKE '% inform' OR name = 'inform' OR name LIKE '%from/about%' OR name LIKE '%narrate%'");
	while (nrc_s.retrieve())
		NRC_descriptions.insert(nrc_s.get(0).toULongLong(),true);
	Retrieve_Template nmc_s("description","id","name='son' or name LIKE '% father' or name LIKE 'father' or name LIKE 'father %' or name LIKE '% mother' or name LIKE 'mother' or name LIKE 'mother/%' or name LIKE 'grandfather' or name LIKE 'grandmother' or name LIKE 'father-in-law' or name LIKE 'mother-in-law' or name LIKE '% uncle' or name LIKE '% uncles'");
	while (nmc_s.retrieve())
		familyNMC_descriptions.insert(nmc_s.get(0).toULongLong(),true);//TODO: this used to mean definite NMC along with POSSESSIVE, but called ibn_possesive should be termed in code definite NMC
	Retrieve_Template ibn_s("description","id","name='son'");
	while (ibn_s.retrieve())
		IBN_descriptions.insert(ibn_s.get(0).toULongLong(),true);

	QFile file(preProcessedDescriptionsFileName.toStdString().data());
	if (file.open(QIODevice::WriteOnly))
	{
		QDataStream out(&file);   // we will serialize the data into the file
		out	<< NRC_descriptions
			<< familyNMC_descriptions
			<< IBN_descriptions;
		file.close();
	}
	else
		error <<"Unexpected Error: Unable to write PreProcessed Descriptions to file\n";
}
void readFromFilePreprocessedDescriptions() {
#ifndef LOAD_FROM_FILE
	readFromDatabasePreProcessedDescriptions();
#else
	QFile file(preProcessedDescriptionsFileName.toStdString().data());
	if (file.open(QIODevice::ReadOnly))
	{
		QDataStream in(&file);    // read the data serialized from the file
		in	>> NRC_descriptions
			>> familyNMC_descriptions
			>> IBN_descriptions;
		file.close();
	}
	else
		readFromDatabasePreProcessedDescriptions();
#endif
}
#endif

void hadith_initialize() {
	hadath.append(_7a2).append(dal).append(tha2);
	abid.append(_3yn).append(ba2).append(dal);
	//abihi.append(alef).append(ba2).append(ya2).append(ha2);
	abyi.append(alef).append(ba2).append(ya2);
	suffixNames.append(QString("")+ha2);
	suffixNames.append(QString("")+ha2+meem);
	suffixNames.append(QString("")+ha2+meem+alef);
	alrasoul.append(alef).append(lam).append(ra2).append(seen).append(waw).append(lam);
	_3an.append(_3yn).append(noon);
#if defined(REFINEMENTS) && !defined(JUST_BUCKWALTER)
	long abstract_NAME=database_info.comp_rules->getAbstractCategoryID("Male Names");
	long abstract_COMPOUND_NAMES=database_info.comp_rules->getAbstractCategoryID("Compound Names");
	int bit_COMPOUND_NAMES=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_COMPOUND_NAMES);
	bits_NAME.append(bit_COMPOUND_NAMES);
	long abstract_NOUN_PROP=database_info.comp_rules->getAbstractCategoryID("Female Names");//"NOUN_PROP"
	bit_NOUN_PROP=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_NOUN_PROP);
#ifdef ADD_ENARRATOR_NAMES
	long abstract_ENARRATOR_NAMES=database_info.comp_rules->getAbstractCategoryID("eNarrator Names");
	int bit_ENARRATOR_NAMES=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_ENARRATOR_NAMES);
	bits_NAME.append(bit_ENARRATOR_NAMES);
#endif
#elif defined(JUST_BUCKWALTER)
	long abstract_NAME=database_info.comp_rules->getAbstractCategoryID("NOUN_PROP");
#else
	long abstract_NAME=database_info.comp_rules->getAbstractCategoryID("Name of Person");
#endif
#ifndef JUST_BUCKWALTER
	long abstract_POSSESSIVE=database_info.comp_rules->getAbstractCategoryID("POSSESSIVE");
	long abstract_PLACE=database_info.comp_rules->getAbstractCategoryID("Name of Place");
	long abstract_CITY=database_info.comp_rules->getAbstractCategoryID("City/Town");
	long abstract_COUNTRY=database_info.comp_rules->getAbstractCategoryID("Country");
	bit_POSSESSIVE=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_POSSESSIVE);
	bit_PLACE=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_PLACE);
	bit_CITY=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_CITY);
	bit_COUNTRY=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_COUNTRY);
#else
	abstract_POSSESSIVE=-1;
	abstract_PLACE=-1;
	abstract_CITY=-1;
	abstract_COUNTRY=-1;
#endif
	int bit_NAME=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_NAME);
	bits_NAME.append(bit_NAME);
#ifdef REFINEMENTS
	QFile input(PhrasesFileName);	 //contains compound words or phrases
									 //maybe if later number of words becomes larger we save it into a trie and thus make their finding in a text faster
	if (!input.open(QIODevice::ReadOnly))
		return;
	QTextStream file(&input);
	file.setCodec("utf-8");
	QString phrases=file.readAll();
	if (phrases.isNull() || phrases.isEmpty())
		return;
	compound_words=phrases.split("\n",QString::SkipEmptyParts);

	QFile input2(StopwordsFileName);	//words at which sanad is assumed to have finished
										//maybe if later number of words becomes larger we save it into a trie and thus make their finding in a text faster
	if (!input2.open(QIODevice::ReadOnly))
		return;
	QTextStream file2(&input2);
	file2.setCodec("utf-8");
	QString stopwords=file2.readAll();
	if (stopwords.isNull() || stopwords.isEmpty())
		return;
	rasoul_words=stopwords.split("\n",QString::SkipEmptyParts);
#endif
#ifdef STATS
	stat.chains=0;
	stat.names_in=0;
	stat.names_out=0;
	stat.narrators=0;
	stat.name_per_narrator.clear();
	stat.narrator_per_chain.clear();
#endif
#ifdef PREPROCESS_DESCRIPTIONS
	readFromFilePreprocessedDescriptions();
#endif
}

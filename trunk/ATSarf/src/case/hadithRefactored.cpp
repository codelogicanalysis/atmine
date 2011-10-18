#if 1
#include "hadith.h"

#ifdef GENERAL_HADITH
#include <QTextBrowser>
#include <assert.h>

#include "ATMProgressIFC.h"
#include "Math_functions.h"
#include "graph.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "hadithCommon.h"


#ifdef HADITHDEBUG
inline QString type_to_text(WordType t) {
	switch(t)
	{
		case NAME:
			return "NAME";
		case NRC:
			return "NRC";
		case NMC:
			return "NMC";
	#ifdef REFINEMENTS
		case STOP_WORD:
			return "STOP_WORD";
	#endif
		default:
			return "UNDEFINED-TYPE";
	}
}
inline QString type_to_text(StateType t) {
	switch(t)
	{
		case TEXT_S:
			return "TEXT_S";
		case NAME_S:
			return "NAME_S";
		case NMC_S:
			return "NMC_S";
		case NRC_S:
			return "NRC_S";
	#ifdef REFINEMENTS
		case STOP_WORD_S:
			return "STOP_WORD_S";
	#endif
		default:
			return "UNDEFINED-TYPE";
	}
}
inline void display(WordType t) {
	out<<type_to_text(t)<<" ";
	//qDebug() <<type_to_text(t)<<" ";
}
inline void display(StateType t) {
	out<<type_to_text(t)<<" ";
	//qDebug() <<type_to_text(t);
}
inline void display(QString t) {
	out<<t;
	//qDebug() <<t;
}
#else
	#define display(c)
#endif


class HadithSegmentor {
private:

	QString fileName;
	StateData currentData;
	QString * text;
	long current_pos;

#ifdef STATS
	QVector<map_entry*> temp_nrc_s, temp_nmc_s;
	int temp_nrc_count,temp_nmc_count;
	statistics stat;
	int temp_names_per_narrator;
	QString current_exact,current_stem;
#endif
	int segmentHelper(QString * text,int (*functionUsingChains)(ChainsContainer &, ATMProgressIFC *, QString),ATMProgressIFC *prg) {
	#ifdef COMPARE_TO_BUCKWALTER
		QFile myfile("output");

		myfile.remove();
		if (!myfile.open(QIODevice::ReadWrite))
			return 1;
		QByteArray output_to_file;
		myoutPtr= new QTextStream(&output_to_file);
	#else
		QFile chainOutput(chainDataStreamFileName);

		chainOutput.remove();
		if (!chainOutput.open(QIODevice::ReadWrite))
			return 1;
		QDataStream chainOut(&chainOutput);
	#endif
	#ifdef BUCKWALTER_INTERFACE
		timeval tim;
		gettimeofday(&tim,NULL);
		double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
	#endif
		if (text==NULL)
			return -1;
		long text_size=text->size();
	#ifdef BUCKWALTER_INTERFACE
		gettimeofday(&tim,NULL);
		double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
		out	<<"="<<t2-t1<<"-";
	#endif
	#ifdef COUNT_AVERAGE_SOLUTIONS
		total_solutions=0;
		stemmings=0;
	#endif
	#ifndef COMPARE_TO_BUCKWALTER
		long  newHadithStart=-1;
		currentData.initialize();

	#ifdef CHAIN_BUILDING
		HadithData *currentChain=new HadithData(text,true,NULL,fileName);
		currentChain->initialize(text);
		display(QString("\ninit0\n"));
	#else
		chainData *currentChain=NULL;
	#endif
		long  sanadEnd;
		int hadith_Counter=1;
	#endif
		StateInfo stateInfo;
		stateInfo.resetCurrentWordInfo();
		stateInfo.currentState=TEXT_S;
		stateInfo.nextState=TEXT_S;
		stateInfo.lastEndPos=0;
		stateInfo.startPos=0;
		stateInfo.nrcPreviousType=false;
		stateInfo.processedStructure=INITIALIZE;
	#ifdef PUNCTUATION
		stateInfo.previousPunctuationInfo.fullstop=true;
	#endif
	#ifndef BUCKWALTER_INTERFACE
		while(stateInfo.startPos<text->length() && isDelimiter(text->at(stateInfo.startPos)))
			stateInfo.startPos++;
	#else
		QString line =getnext();//just for first line
		assert(line=="");
	#endif
	#ifdef PROGRESSBAR
		prg->setCurrentAction("Parsing Hadith");
	#endif
		for (;stateInfo.startPos<text_size;) {
	#ifndef COMPARE_TO_BUCKWALTER
			if((proceedInStateMachine(stateInfo,currentChain,currentData)==false)) {
				assert (currentChain->narrator==NULL);
				if (currentData.narratorCount>=hadithParameters.narr_min) {
					sanadEnd=currentData.narratorEndIndex;
				#ifdef DISPLAY_HADITH_OVERVIEW
					newHadithStart=currentData.mainStructureStartIndex;
					//long end=text->indexOf(QRegExp(delimiters),sanadEnd);//sanadEnd is first letter of last word in sanad
					//long end=stateInfo.endPos;
					out<<"\n"<<hadith_Counter<<" new hadith start: "<<text->mid(newHadithStart,display_letters)<<endl;
					out<<"sanad end: "<<text->mid(sanadEnd-display_letters+1,display_letters)<<endl<<endl;
				#ifdef CHAIN_BUILDING
					currentChain->chain->serialize(chainOut);
					//currentChain->chain->serialize(displayed_error);
				#endif
				#endif
					hadith_Counter++;
				#ifdef STATS
					int additional_names=temp_names_per_narrator;
					temp_names_per_narrator=0;
					for (int i=1;i<=currentData.narratorCount;i++)
						additional_names+=stat.name_per_narrator[stat.name_per_narrator.size()-i];//we are sure names found are in narrators
					stat.names_in+=additional_names;
					stat.chains++;
					stat.narrator_per_chain.append(currentData.narratorCount);
					stat.narrators+=currentData.narratorCount;
					display(QString("narrator_per_chain=")+QString::number(currentData.narratorCount)+", names="+QString::number(additional_names)+"\n");

					for (int i=0;i<temp_nmc_s.count();i++)
					{
						if (!stat.nmc_stem.contains(temp_nmc_s[i]->stem))
							stat.nmc_stem[temp_nmc_s[i]->stem]=1;
						else
							stat.nmc_stem[temp_nmc_s[i]->stem]++;
						if (!stat.nmc_exact.contains(temp_nmc_s[i]->exact))
							stat.nmc_exact.insert(temp_nmc_s[i]->exact,temp_nmc_s[i]);
						else
						{
							map_entry * entry=stat.nmc_exact.value(temp_nmc_s[i]->exact);
							assert (entry->stem==temp_nmc_s[i]->stem);
							entry->frequency++;
							delete  temp_nmc_s[i];
						}
					}
					temp_nmc_s.clear();
					temp_nmc_count=0;
					for (int i=0;i<temp_nrc_s.count();i++)
					{
						if (!stat.nrc_stem.contains(temp_nrc_s[i]->stem))
							stat.nrc_stem[temp_nrc_s[i]->stem]=1;
						else
							stat.nrc_stem[temp_nrc_s[i]->stem]++;
						if (!stat.nrc_exact.contains(temp_nrc_s[i]->exact))
							stat.nrc_exact.insert(temp_nrc_s[i]->exact,temp_nrc_s[i]);
						else
						{
							map_entry * entry=stat.nrc_exact.value(temp_nrc_s[i]->exact);
							assert (entry->stem==temp_nrc_s[i]->stem);
							entry->frequency++;
							delete  temp_nrc_s[i];
						}
					}
					temp_nrc_s.clear();
					temp_nrc_count=0;
				#endif
				}
				else {
				#ifdef STATS
					int additional_names=temp_names_per_narrator;
					temp_names_per_narrator=0;
					for (int i=1;i<=currentData.narratorCount;i++)
						additional_names+=stat.name_per_narrator[stat.name_per_narrator.size()-i];//we are sure names found are in not inside valid narrators
					stat.names_out+=additional_names;
					stat.name_per_narrator.remove(stat.name_per_narrator.size()-currentData.narratorCount,currentData.narratorCount);
					display(QString("additional names out names =")+QString::number(additional_names)+"\n");

					for (int i=0;i<temp_nmc_s.count();i++)
						delete temp_nmc_s[i];
					temp_nmc_s.clear();
					for (int i=0;i<temp_nrc_s.count();i++)
						delete temp_nrc_s[i];
					temp_nrc_s.clear();
					temp_nmc_count=0;
					temp_nrc_count=0;
				#endif
				}
			}
			stateInfo.currentState=stateInfo.nextState;
			stateInfo.startPos=stateInfo.nextPos;
			stateInfo.lastEndPos=stateInfo.endPos;
		#ifdef PUNCTUATION
			stateInfo.previousPunctuationInfo=stateInfo.currentPunctuationInfo;
			if (stateInfo.number) {
				stateInfo.previousPunctuationInfo.fullstop=true;
				stateInfo.previousPunctuationInfo.has_punctuation=true;
			}
			if (stateInfo.previousPunctuationInfo.has_punctuation)
				stateInfo.previousPunctuationInfo.fullstop=true;
		#endif
	#else
			hadith_stemmer s(text,stateInfo.startPos);
			s();
			long finish=max(s.info.finish,s.finish_pos);
			if (finish==current_pos)
				finish=getLastLetter_IN_currentWord(stateInfo.startPos);
			stateInfo.startPos=next_positon(finish);
	#endif
	#ifdef PROGRESSBAR
			prg->report((double)stateInfo.startPos/text_size*100+0.5);
			if (stateInfo.startPos==text_size-1)
				break;
	#endif
		}
	#ifdef NONCONTEXT_LEARNING
		currentChain->learningEvaluator.displayNameLearningStatistics();
		currentChain->learningEvaluator.resetLearnedNames();
	#endif
		prg->report(100);
	#if !defined(COMPARE_TO_BUCKWALTER) && defined(DISPLAY_HADITH_OVERVIEW)
		if (newHadithStart<0)
		{
			out<<"no hadith found\n";
			chainOutput.close();
			return 2;
		}
		chainOutput.close();
	#endif
	#ifdef CHAIN_BUILDING //just for testing deserialize
		QFile f("hadith_chains.txt");
		if (!f.open(QIODevice::WriteOnly))
			return 1;
		QTextStream file_hadith(&f);
			file_hadith.setCodec("utf-8");

		if (!chainOutput.open(QIODevice::ReadWrite))
			return 1;
		QDataStream tester(&chainOutput);
		int tester_Counter=1;
	#ifdef TEST_NARRATOR_GRAPH
		ChainsContainer chains;
		chains.clear();
	#endif
	#if defined(TAG_HADITH)
		prg->startTaggingText(*text);
	#endif
		while (!tester.atEnd())
		{
			Chain * s=new Chain(text);
			s->deserialize(tester);
		#ifdef TEST_NARRATOR_GRAPH
			chains.append(s);
		#endif
		#if defined(TAG_HADITH)
			for (int j=0;j<s->m_chain.size();j++)
			{
				ChainPrim * curr_struct=s->m_chain[j];
				if (curr_struct->isNarrator())
				{
					Narrator * n=(Narrator *)curr_struct;
					if (n->m_narrator.size()==0) {
						out<<"found a problem an empty narrator in ("<<tester_Counter<<","<<j<<")\n";
						continue;
					}
					prg->tag(curr_struct->getStart(),curr_struct->getLength(),Qt::darkYellow,false);
					for (int i=0;i<n->m_narrator.size();i++)
					{
						NarratorPrim * nar_struct=n->m_narrator[i];
						if (nar_struct->isNamePrim()) {
							if (((NamePrim*)nar_struct)->learnedName) {
								prg->tag(nar_struct->getStart(),nar_struct->getLength(),Qt::blue,true);
								//error<<nar_struct->getString()<<"\n";
							}
							else
								prg->tag(nar_struct->getStart(),nar_struct->getLength(),Qt::white,true);
						}
						else if (((NameConnectorPrim *)nar_struct)->isFamilyConnector())
							prg->tag(nar_struct->getStart(),nar_struct->getLength(),Qt::darkRed,true);
						else if (((NameConnectorPrim *)nar_struct)->isPossessive())
							prg->tag(nar_struct->getStart(),nar_struct->getLength(),Qt::darkMagenta,true);
					}
				}
				else
					prg->tag(curr_struct->getStart(),curr_struct->getLength(),Qt::gray,false);
			}
		#else
			hadith_out<<tester_Counter<<" ";
			s->serialize(hadith_out);
		#endif
			tester_Counter++;
			s->serialize(file_hadith);
		}
		chainOutput.close();
		f.close();
	#ifdef TEST_NARRATOR_GRAPH
		(*functionUsingChains)(chains, prg,fileName);
	#endif
	#endif
	#ifndef TAG_HADITH
	#if 0
		prg->startTaggingText(*hadith_out.string()); //we will not tag but this will force a text to be written there
	#endif
	#else
		prg->finishTaggingText();
	#endif


	#endif
	#ifdef STATS
		double avg_narrators_per_chain=average(stat.narrator_per_chain);
		double avg_names_per_narrator=average(stat.name_per_narrator);

		displayed_error	<<"Chains=\t\t"<<stat.chains<<"\n"
						<<"Narrators=\t\t"<<stat.narrators<<"\n"
						<<"Names IN=\t\t"<<stat.names_in<<"\n"
						<<"Names OUT=\t\t"<<stat.names_out<<"\n"
						<<"Avg Names/Narr=\t"<<avg_names_per_narrator<<"\n"
						<<"Avg Narr/Chain=\t"<<avg_narrators_per_chain<<"\n"
						<<"Median Names/Narr=\t"<<median(stat.name_per_narrator)<<"\n"
						<<"Median Narr/Chain=\t"<<median(stat.narrator_per_chain)<<"\n"
						<<"St Dev Names/Narr=\t"<<standard_deviation(stat.name_per_narrator,avg_names_per_narrator)<<"\n"
						<<"St Dev Narr/Chain=\t"<<standard_deviation(stat.narrator_per_chain,avg_narrators_per_chain)<<"\n";
		displayed_error <<"\nNMC:\n";
		show_according_to_frequency(stat.nmc_stem.values(),stat.nmc_stem.keys());
		displayed_error <<"\nNRC:\n";
		show_according_to_frequency(stat.nrc_stem.values(),stat.nrc_stem.keys());
		displayed_error <<"\n\nNMC-exact:\n";
		QList<int> freq;
		QList<map_entry *> temp=stat.nmc_exact.values();
		for (int i=0;i<temp.size();i++)
			freq.append(temp[i]->frequency);
		show_according_to_frequency(freq,stat.nmc_exact.keys());
		displayed_error <<"\nNRC-exact:\n";
		freq.clear();
		temp=stat.nrc_exact.values();
		for (int i=0;i<temp.size();i++)
			freq.append(temp[i]->frequency);
		show_according_to_frequency(freq,stat.nrc_exact.keys());
		displayed_error <<"\n";
		temp=stat.nmc_exact.values();
		for (int i=0;i<temp.size();i++)
			delete temp[i];
		temp=stat.nrc_exact.values();
		for (int i=0;i<temp.size();i++)
			delete temp[i];
	#endif
	#ifdef COMPARE_TO_BUCKWALTER
		myfile.write(output_to_file);
	#endif
	#ifdef COUNT_AVERAGE_SOLUTIONS
		displayed_error	<< (double)(total_solutions/(long double)stemmings)<<"\n"
						<<stemmings<<"\n";
	#endif
		//delete text;
		if (currentChain!=NULL)
			delete currentChain;
		return 0;
	}

public:
	int segment(QString input_str,int (*functionUsingChains)(ChainsContainer &, ATMProgressIFC *, QString),ATMProgressIFC *prg)  {
		fileName=input_str;
		QFile input(input_str);
		if (!input.open(QIODevice::ReadOnly)) {
			out << "File not found\n";
			return 1;
		}
		QTextStream file(&input);
		file.setCodec("utf-8");
		text=new QString(file.readAll());
		if (text->isNull())	{
			out<<"file error:"<<input.errorString()<<"\n";
			return 1;
		}
		if (text->isEmpty()) {//ignore empty files
			out<<"empty file\n";
			return 0;
		}
		return segmentHelper(text,functionUsingChains,prg);
	}
	int segment(QString * text,int (*functionUsingChains)(ChainsContainer &, ATMProgressIFC *, QString),ATMProgressIFC *prg)  {
		fileName="";
		return segmentHelper(text,functionUsingChains,prg);
	}
};

#ifdef IMAN_CODE
class adjective_stemmer: public Stemmer
{
public:
	bool adj ;
	long finish_pos;

	adjective_stemmer(QString * word, int start):Stemmer(word,start/*,false*/)
	{
		adj=false;
		finish_pos=start;
	}
	bool on_match()
	{
		for (unsigned int i=0;i<stem_info->abstract_categories.length();i++)
			if (stem_info->abstract_categories[i] && get_abstractCategory_id(i)>=0)
			{
				if (getColumn("category","name",get_abstractCategory_id(i))=="ADJ")
				{
					adj=true;
					finish_pos=info.finish;
					return false;
				}
			}
		return true;
	}
};

int adjective_detector(QString input_str)
{
	QFile input(input_str);
	if (!input.open(QIODevice::ReadWrite))
	{
		out << "File not found\n";
		return 1;
	}
	QTextStream file(&input);
	file.setCodec("utf-8");
	text=new QString(file.readAll());
	if (text->isNull())
	{
		out<<"file error:"<<input.errorString()<<"\n";
		return 1;
	}
	if (text->isEmpty()) //ignore empty files
	{
		out<<"empty file\n";
		return 0;
	}
	long text_size=text->size();

	while(current_pos<text->length() && delimiters.contains(text->at(current_pos)))
		current_pos++;
	for (;current_pos<text_size;)
	{
		adjective_stemmer s(text,current_pos);
		s();
		long finish=max(s.info.finish,s.finish_pos);
		if (s.adj )
			out <<text->mid(current_pos,finish-current_pos+1)+":ADJECTIVE\n";
		current_pos=next_positon(finish);;//here current_pos is changed
	}
	return 0;
}
#endif

#ifdef BUCKWALTER_INTERFACE //i dont think this code functional anymore
class machine_info
{
public:
	bool name:1;
	bool nrc:1;
	bool nmc:1;

	machine_info()
	{
		name=false;
		nrc=false;
		nmc=false;
	}
};

inline QString getnext()
{
	long next_pos=text->indexOf('\n',current_pos);
	QString line=text->mid(current_pos,next_pos-current_pos);
	current_pos=next_pos+1;
	//qDebug()<<line;
	return line;
}
#ifdef OPTIMIZED_BUCKWALTER_TEST_CASE
#define check(n) ;
#else
#define check(n) assert(n)
#endif
machine_info readNextWord()
{
	machine_info info;
	QString line;
	line=getnext();
	if (line=="")
	{
		current_pos=text->size();
		return info;
	}
	check(line.contains("INPUT STRING"));
	line=getnext();
	if (line.contains("Non-Alphabetic Data"))
	{
		line=getnext();
		check (line=="");
		return info;
	}
	check(line.contains("LOOK-UP WORD"));
	while(1)
	{
		line=getnext();
		if (line=="")
			break;
		while (line.contains("NOT FOUND"))
		{
			line=getnext();
			if (line=="")
				return info;
			else if (line.contains("ALTERNATIVE"))
			{
				line=getnext();
				if (line=="")
					return info;
			}
		}
		check(line.contains("SOLUTION"));
		QString raw_data=line.split('(').at(1).split(')').at(0);
		/*line=getnext();
		check(line.startsWith(']'));*/
		if (line.contains("NOUN_PROP"))
			info.name=true;
		line=getnext();
		check(line.contains("(GLOSS)"));
		QString description=line.split(" + ").at(1);
		if (raw_data.contains("Hdv"))
			info.nrc=true;
		else if (description=="son")
			info.nmc=true;
		else if (description=="said" || description=="say" || description=="notify/communicate" || description.split(QRegExp("[ /]")).contains("listen") || description.contains("from/about") || description.contains("narrate"))
			info.nrc=true;
	}
	return info;
}

wordType getWordType(bool & isBinOrPossessive,bool & possessive, long &next_pos)
{
	isBinOrPossessive=false;
	possessive=false;
	machine_info s=readNextWord();
	next_pos=current_pos;
	if (s.nrc )
		return result(NRC);
	else if (s.nmc)
	{
		display("NMC-Bin/Pos ");
		isBinOrPossessive=true;
		return NMC;
	}
	else if (s.name)
		return result(NAME);
	else
		return result(NMC);
}
#ifdef OPTIMIZED_BUCKWALTER_TEST_CASE
#undef check
#endif
#include <sys/time.h>
#endif

int hadithHelper(QString input_str,ATMProgressIFC *prg) {
	input_str=input_str.split("\n")[0];
#ifdef IMAN_CODE
	return adjective_detector(input_str);
#endif
	HadithSegmentor s;
	s.segment(input_str,&test_GraphFunctionalities,prg);
	return 0;
}

int segmentNarrators(QString * text,int (*functionUsingChains)(ChainsContainer &, ATMProgressIFC *, QString), ATMProgressIFC *prg) {
	int narr_min=hadithParameters.narr_min;
	hadithParameters.narr_min=0;
	int size=text->size();
	text->append(QString(" stop").repeated(hadithParameters.nmc_max+2));
	HadithSegmentor s;
	s.segment(text,functionUsingChains,prg);
	hadithParameters.narr_min=narr_min;
	text->remove(size,10);
	return 0;
}

#endif

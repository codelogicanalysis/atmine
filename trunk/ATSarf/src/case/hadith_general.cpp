#include "hadith.h"

#ifdef GENERAL_HADITH
#include <QFile>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QTextBrowser>

#include "stemmer.h"
#include "letters.h"
#include "text_handling.h"
#include "diacritics.h"
#include "narrator_abstraction.h"
#include "common.h"
#include "ATMProgressIFC.h"
#include "Math_functions.h"
#include "sql_queries.h"
#include "Retrieve_Template.h"
#include "Search_by_item_locally.h"
#include "graph.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <assert.h>

HadithParameters parameters;
QString chainDataStreamFileName= "chainOutput";
#ifndef SUBMISSION
QString PhrasesFileName="../src/case/phrases";
QString StopwordsFileName="../src/case/stop_words";
#else
QString PhrasesFileName=".phrases";
QString StopwordsFileName=".stop_words";
#endif


#ifndef REFINEMENTS
	enum wordType { NAME, NRC,NMC};
	enum stateType { TEXT_S , NAME_S, NMC_S , NRC_S};
#else
	enum wordType { NAME, NRC,NMC,STOP_WORD};
	enum stateType { TEXT_S , NAME_S, NMC_S , NRC_S, STOP_WORD_S};
	QStringList rasoul_words;
#endif

	QStringList compound_words;
	QString hadath,abid,alrasoul;
	long abstract_NAME, abstract_POSSESSIVE, abstract_PLACE, abstract_CITY,abstract_COUNTRY;
	int bit_NAME, bit_POSSESSIVE, bit_PLACE,bit_CITY,bit_COUNTRY;
	QString non_punctuation_delimiters;
#ifdef PREPROCESS_DESCRIPTIONS
	QHash<long,bool> NMC_descriptions;
	QHash<long,bool> NRC_descriptions;
	QHash<long,bool> IBN_descriptions;
#endif
#ifdef COMPARE_TO_BUCKWALTER
	QTextStream * myoutPtr;
#endif
#ifdef COUNT_AVERAGE_SOLUTIONS
	long long total_solutions=0;
	long stemmings=0;
#endif
#ifdef HADITHDEBUG
inline QString type_to_text(wordType t)
{
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
inline QString type_to_text(stateType t)
{
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
inline void display(wordType t)
{
	out<<type_to_text(t)<<" ";
	//qDebug() <<type_to_text(t)<<" ";
}
inline void display(stateType t)
{
	out<<type_to_text(t)<<" ";
	//qDebug() <<type_to_text(t);
}
inline void display(QString t)
{
	out<<t;
	//qDebug() <<t;
}
#else
#define display(c)
#endif

typedef QList<NameConnectorPrim *> TempConnectorPrimList;

#define display_letters 30
typedef struct chainData_ {
    NamePrim *namePrim;
    NameConnectorPrim *nameConnectorPrim;
    NarratorConnectorPrim *narratorConnectorPrim;
	TempConnectorPrimList * temp_nameConnectors;
    Narrator *narrator;
    Chain *chain;

} chainData;
typedef struct stateData_ {
	long long  sanadStartIndex, narratorCount,narratorStartIndex,narratorEndIndex,nrcStartIndex,nrcEndIndex;
	long long  nmcCount, nrcCount,nameStartIndex,nmcStartIndex;
	bool nmcValid;
	bool ibn_or_3abid;
	bool nrcPunctuation;
} stateData;

#ifdef STATS
typedef struct map_entry_ {
	int frequency;
	QString exact;
	QString stem;
} map_entry;
typedef struct statistics_{
	int names_in, names_out, chains, narrators;
	QVector<int> narrator_per_chain, name_per_narrator;
	QHash <QString, map_entry*>  nrc_exact, nmc_exact;
	QHash <QString, int> nrc_stem,nmc_stem;
} statistics;
QVector<map_entry*> temp_nrc_s, temp_nmc_s;
int temp_nrc_count,temp_nmc_count;
statistics stat;
int temp_names_per_narrator;
QString current_exact,current_stem;
#endif

stateData currentData;
QString * text;
long current_pos;
#ifdef CHAIN_BUILDING
void initializeChainData(chainData *currentChain)
{
	delete currentChain->namePrim;
	delete currentChain->nameConnectorPrim;
	delete currentChain->narratorConnectorPrim;
	delete currentChain->narrator;
	delete currentChain->chain;
	/*for (int i=0;i<currentChain->temp_nameConnectors->count()-1;i++)
		delete (*currentChain->temp_nameConnectors)[i];*/
	delete currentChain->temp_nameConnectors;
	currentChain-> namePrim=new NamePrim(text);
	currentChain-> nameConnectorPrim=new NameConnectorPrim(text);
	currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text) ;
	currentChain->  narrator=new Narrator (text);
	currentChain->  chain=new Chain(text);
	currentChain->temp_nameConnectors=new TempConnectorPrimList();
	display(QString("\ninit%1\n").arg(currentChain->narrator->m_narrator.size()));
}
#endif

void hadith_initialize()
{
	hadath.append(_7a2).append(dal).append(tha2);
	abid.append(_3yn).append(ba2).append(dal);
	alrasoul.append(alef).append(lam).append(ra2).append(seen).append(waw).append(lam);
#if defined(REFINEMENTS) && !defined(JUST_BUCKWALTER)
	abstract_NAME=get_abstractCategory_id("Male Names");
#elif defined(JUST_BUCKWALTER)
	abstract_NAME=get_abstractCategory_id("NOUN_PROP");
#else
	abstract_NAME=get_abstractCategory_id("Name of Person");
#endif
#ifndef JUST_BUCKWALTER
	abstract_POSSESSIVE=get_abstractCategory_id("POSSESSIVE");
	abstract_PLACE=get_abstractCategory_id("Name of Place");
	abstract_CITY=get_abstractCategory_id("City/Town");
	abstract_COUNTRY=get_abstractCategory_id("Country");
	bit_POSSESSIVE=get_bitindex(abstract_POSSESSIVE,abstract_category_ids);
	bit_PLACE=get_bitindex(abstract_PLACE,abstract_category_ids);
	bit_CITY=get_bitindex(abstract_CITY,abstract_category_ids);
	bit_COUNTRY=get_bitindex(abstract_COUNTRY,abstract_category_ids);
#else
	abstract_POSSESSIVE=-1;
	abstract_PLACE=-1;
	abstract_CITY=-1;
	abstract_COUNTRY=-1;
#endif
	bit_NAME=get_bitindex(abstract_NAME,abstract_category_ids);
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
	//preprocessing descriptions:
	Retrieve_Template nrc_s("description","id","name='said' OR name='say' OR name='notify/communicate' OR name LIKE '%/listen' OR name LIKE 'listen/%' OR name LIKE 'listen %' OR name LIKE '% listen' OR name = 'listen' OR name LIKE '%/inform' OR name LIKE 'inform/%' OR name LIKE 'inform %' OR name LIKE '% inform' OR name = 'inform' OR name LIKE '%from/about%' OR name LIKE '%narrate%'");
	while (nrc_s.retrieve())
		NRC_descriptions.insert(nrc_s.get(0).toULongLong(),true);
	Retrieve_Template nmc_s("description","id","name='son' or name LIKE '% father' or name LIKE 'father' or name LIKE 'father %'");
	while (nmc_s.retrieve())
		NMC_descriptions.insert(nmc_s.get(0).toULongLong(),true);//TODO: this used to mean definite NMC along with POSSESSIVE, but called ibn_possesive should be termed in code definite NMC
	Retrieve_Template ibn_s("description","id","name='son'");
	while (ibn_s.retrieve())
		IBN_descriptions.insert(ibn_s.get(0).toULongLong(),true);
#endif
	non_punctuation_delimiters=delimiters;
	non_punctuation_delimiters.remove(QRegExp(QString("[")+punctuation+"]"));
}

inline void initializeStateData()
{
	currentData.nmcCount=0;
	currentData.narratorCount=0;
	currentData.nrcCount=0;
	currentData.narratorStartIndex=0;
	currentData.narratorEndIndex=0;
	currentData.nrcStartIndex=0;
	currentData.nrcEndIndex=0;
	currentData.nmcValid=false;
	currentData.ibn_or_3abid=false;
	currentData.nameStartIndex=0;
	currentData.nmcStartIndex=0;
#ifdef PUNCTUATION
	currentData.nrcPunctuation=false;
#endif
}

class hadith_stemmer: public Stemmer
{
private:
	bool place;
#ifdef TEST_WITHOUT_SKIPPING
	bool finished;
#endif
public:
	bool name, nrc, nmc,possessive, ibn_or_3abid;
	long long finish_pos;
#ifdef STATS
	QString stem;
	QList<QString> stems;
#endif
	hadith_stemmer(QString * word, int start)
#ifndef COMPARE_TO_BUCKWALTER
		:Stemmer(word,start,false)
#else
		:Stemmer(word,start,true)
#endif
	{
		setSolutionSettings(M_ALL);
		init(start);
	}
	void init(int start)
	{
		this->info.start=start;
		this->info.finish=start;
		name=false;
		nmc=false;
		nrc=false;
		possessive=false;
		place=false;
		ibn_or_3abid=false;
		finish_pos=start;
	#ifdef STATS
		stem="";
		stems.clear();
	#endif
	#ifdef TEST_WITHOUT_SKIPPING
		finished=false;
	#endif
	#ifdef COUNT_AVERAGE_SOLUTIONS
		stemmings++;
	#endif
	}
#ifndef COMPARE_TO_BUCKWALTER
	bool on_match()
	{
	#if 0
		Search_by_item_locally s(STEM,Stem->id_of_currentmatch,Stem->category_of_currentmatch,Stem->raw_data_of_currentmatch);
		if (!called_everything)
			info.finish=Stem->currentMatchPos;
		stem_info=new minimal_item_info;
		while(s.retrieve(*stem_info))
		{
			if (!analyze())
				return false;
		}
		return true;
	#else
		solution_position * S_inf=Stem->computeFirstSolution();
		do
		{
			stem_info=Stem->solution;
		#ifdef GET_AFFIXES_ALSO
			solution_position * p_inf=Prefix->computeFirstSolution();
			do
			{
				prefix_infos=Prefix->affix_info;
				solution_position * s_inf=Suffix->computeFirstSolution();
				do
				{
					suffix_infos=Suffix->affix_info;
		#endif
				#ifdef COUNT_AVERAGE_SOLUTIONS
					total_solutions++;
				#endif
				#ifndef TEST_WITHOUT_SKIPPING
					if (!analyze())
						return false;
				#else
					if (!finished && !analyze())
						finished=true;
				#endif
		#ifdef GET_AFFIXES_ALSO
				}while (Suffix->computeNextSolution(s_inf));
				delete s_inf;
			}while (Prefix->computeNextSolution(p_inf));
			delete p_inf;
		#endif
		}while (Stem->computeNextSolution(S_inf));
		delete S_inf;
		return true;
	#endif
	}

	bool analyze()
	{
	#ifdef STATS
		QString temp_stem=removeDiacritics(info.text->mid(Stem->starting_pos, Suffix->info.start-Stem->starting_pos));//removeDiacritics(stem_info->raw_data);
	#endif
	#ifndef PREPROCESS_DESCRIPTIONS
		QString description=stem_info->description();
	#endif
	#ifdef REFINEMENTS
		ibn_or_3abid= (equal_ignore_diacritics(stem_info->raw_data,abid));
	#endif
		if (equal_ignore_diacritics(stem_info->raw_data,hadath))
		{
		#ifdef STATS
			stem=temp_stem;
		#endif
			nrc=true;
			finish_pos=info.finish;
			return false;
		}
	#ifndef PREPROCESS_DESCRIPTIONS
		else if (description=="son")
	#else
		else if (NMC_descriptions.contains(stem_info->description_id))
	#endif
		{
		#ifdef STATS
			stem=temp_stem;
		#endif
		#ifdef REFINEMENTS
			ibn_or_3abid=true;
		#endif
			nmc=true;
			finish_pos=info.finish;
			return false;
		}
	#ifndef PREPROCESS_DESCRIPTIONS
		else if (description=="said" || description=="say" || description=="notify/communicate" || description.split(QRegExp("[ /]")).contains("listen") || description.contains("from/about") || description.contains("narrate"))
	#else
		else if (NRC_descriptions.contains(stem_info->description_id))
	#endif
		{
		#ifdef STATS
			stem=temp_stem;
		#endif
			nrc=true;
			finish_pos=info.finish;
			return false;
		}
		if (stem_info->abstract_categories.getBit(bit_NAME))
		{
			name=true;
			if (info.finish>finish_pos)
			{
				finish_pos=info.finish;
			#ifdef STATS
				stem=temp_stem;
			#endif
			}
			return true;
		}
#ifndef JUST_BUCKWALTER
		if (stem_info->abstract_categories.getBit(bit_POSSESSIVE))
		{
			possessive=true;
			if (place)
			{
			#ifdef STATS
				stem=temp_stem;
			#endif
				nmc=true;
				finish_pos=info.finish;
				return false;
			}
		}
		if (stem_info->abstract_categories.getBit(bit_PLACE))
		{
		#ifdef STATS
			stem=temp_stem;
		#endif
			place=true;
			if (possessive)
			{
				nmc=true;
				finish_pos=info.finish;
				return false;
			}
		}
#endif
		#ifdef STATS
			stems.append(temp_stem);
		#endif
		return true;
	}
#else
	bool on_match()
	{
		QTextStream & myout=*myoutPtr;
		myout	<<"( "<<prefix_infos->at(0).description()<<" )-"
				<<"-("<<stem_info->description()
				<<" [ ";
#ifndef COMPARE_WITHOUT_ABSCAT
		for (int i=0;i<abstract_category_ids[max_sources]/*stem_info->abstract_categories.length()*/;i++)
			if (stem_info->abstract_categories[i])
				myout<<database_info.comp_rules->getCategoryName(get_abstractCategory_id(i))<<"/";
		myout	<<" ][ ";
#endif
		myout	<<stem_info->POS
				<<"])--("<< suffix_infos->at(0).description()<<")"
				<<" "<<prefix_infos->at(0).raw_data<<stem_info->raw_data<<suffix_infos->at(0).raw_data<<" "
				<<" "<<info.finish+1-info.start<<"\n";
		return true;
	}
#endif
};

inline long long next_positon(long long finish,bool & has_punctuation)
{
	has_punctuation=false;
#ifdef PUNCTUATION
	if (finish<text->length() && punctuation.contains(text->at(finish)))
		has_punctuation=true;
#endif
	finish++;
	while(finish<text->length())
	{
	#ifdef PUNCTUATION
		if (punctuation.contains(text->at(finish)))
			has_punctuation=true;
		else if (!non_punctuation_delimiters.contains(text->at(finish)))
			break;
	#else
		if (!delimiters.contains(text->at(finish)))
			break;
	#endif
		finish++;
	}
	return finish;
}

inline long long getLastLetter_IN_previousWord(long long start_letter_current_word)
{
	start_letter_current_word--;
	while(start_letter_current_word>=0 && isDelimiter(text->at(start_letter_current_word)))
		start_letter_current_word--;
	return start_letter_current_word;
}

inline long long getLastLetter_IN_currentWord(long long start_letter_current_word)
{
	int size=text->length();
	while(start_letter_current_word<size && !isDelimiter(text->at(start_letter_current_word)))
		start_letter_current_word++;
	return start_letter_current_word;
}

inline wordType result(wordType t){display(t); return t;}
inline QString choose_stem(QList<QString> stems) //rule can be modified later
{
	if (stems.size()==0)
		return "";
	QString result=stems[0];
	for (int i=1;i<stems.size();i++)
		if (result.length()>stems[i].length())
			result=stems[i];
	return result;
}
#ifndef BUCKWALTER_INTERFACE
wordType getWordType(bool & isBinOrPossessive,bool & possessive, bool & ibn_or_3abid,bool & has_punctuation, long long &next_pos)
{
	long long  finish;
	isBinOrPossessive=false;
	ibn_or_3abid=false;
#if 0
	static hadith_stemmer * s_p=NULL;
	if (s_p==NULL)
		s_p=new hadith_stemmer(text,current_pos);
	else
		s_p->init(current_pos);
	hadith_stemmer & s=*s_p;
#endif
	hadith_stemmer s(text,current_pos);
#ifdef REFINEMENTS
	QString c;
	bool found,phrase=false,stop_word=false;
	foreach (c, rasoul_words)
	{
	#if 1
		int pos;
		if (startsWith(text->midRef(current_pos),c,pos))
		{
			stop_word=true;
			found=true;
			finish=pos+current_pos;
		#ifdef STATS
			current_stem=c;
			current_exact=c;
		#endif
			break;
		}
	#else
		found=true;
		int pos=current_pos;
		for (int i=0;i<c.length();)
		{
			if (!isDiacritic(text->at(pos)))
			{
				if (!equal(c[i],text->at(pos)))
				{
					found=false;
					break;
				}
				i++;
			}
			pos++;
		}
		if (found)
		{
			stop_word=true;
			finish=pos-1;
		#ifdef STATS
			current_stem=c;
			current_exact=c;
		#endif
			break;
		}
	#endif
	}
	if (!stop_word)//TODO: maybe modified to be set as a utility function, and just called from here
	{
		foreach (c, compound_words)
		{
		#if 1
			int pos;
			if (startsWith(text->midRef(current_pos),c,pos))
			{
				phrase=true;
				found=true;
				finish=pos+current_pos;
			#ifdef STATS
				current_stem=c;
				current_exact=c;
			#endif
				break;
			}
		#else
			found=true;
			int pos=current_pos;
			for (int i=0;i<c.length();)
			{
				if (!isDiacritic(text->at(pos)))
				{
					if (!equal(c[i],text->at(pos)))
					{
						found=false;
						break;
					}
					i++;
				}
				pos++;
			}
			while (isDiacritic(text->at(pos)))
				pos++;
			if (isDiacritic(text->at(pos)))
			if (found)
			{
				phrase=true;
				finish=pos-1;
			#ifdef STATS
				current_stem=c;
				current_exact=c;
			#endif
				break;
			}
		#endif
		}
	}
	if (!stop_word && !phrase)
	{
#endif
		s();
		finish=max(s.info.finish,s.finish_pos);
		if (finish==current_pos)
			finish=getLastLetter_IN_currentWord(current_pos);
		#ifdef STATS
			current_exact=removeDiacritics(s.info.text->mid(current_pos,finish-current_pos+1));
			current_stem=s.stem;
			if (current_stem=="")
				current_stem=choose_stem(s.stems);
			if (current_stem=="")
				current_stem=current_exact;
		#endif
#ifdef REFINEMENTS
	}
#endif
	next_pos=next_positon(finish,has_punctuation);
	display(text->mid(current_pos,finish-current_pos+1)+":");
#ifdef REFINEMENTS
	if (stop_word)
	{
		display("STOP_WORD");
		return STOP_WORD;
	}
	if (phrase)
	{
		display("PHRASE");
		//isBinOrPossessive=true; //same behaviour as Bin
		return NMC;
	}
	ibn_or_3abid=s.ibn_or_3abid;
#endif
	possessive=s.possessive;
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
#endif

bool getNextState(stateType currentState,wordType currentType,stateType & nextState,long long  start_index,bool isBinOrPossessive,bool possessive,bool ibn_or_3abid,bool has_punctuation, long long end_pos,chainData *currentChain)
{
	display(QString(" nmcsize: %1 ").arg(currentData.nmcCount));
	display(QString(" nrcsize: %1 ").arg(currentData.nrcCount));
	display(currentState);
	display("\n");
#ifdef PUNCTUATION
	if (has_punctuation)
	{
		display("<has punctuation>");
	}
#endif
#ifdef REFINEMENTS
	bool should_stop= (currentType== STOP_WORD && !ibn_or_3abid);//stop_word not preceeded by 3abid or ibn
#endif
	bool return_value=true;
	switch(currentState)
	{
	case TEXT_S:
		if(currentType==NAME)
		{
			initializeStateData();
			nextState=NAME_S;
			currentData.sanadStartIndex=start_index;
			currentData.narratorStartIndex=start_index;
		#ifdef CHAIN_BUILDING
			initializeChainData(currentChain);
			currentChain->namePrim=new NamePrim(text,start_index);
		#endif
		#ifdef STATS
			temp_names_per_narrator=1;
		#endif
		#ifdef PUNCTUATION
			if (has_punctuation)
			{
				display("<punc1>");
				currentData.narratorCount++;
				nextState=NRC_S;
				currentData.narratorEndIndex=end_pos;
				currentData.nrcStartIndex=next_positon(end_pos,has_punctuation);
			#ifdef CHAIN_BUILDING
				currentChain->namePrim->m_end=end_pos;
				currentChain->narrator->m_narrator.append(currentChain->namePrim);
				currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,currentData.nrcStartIndex);
				currentChain->chain->m_chain.append(currentChain->narrator);
				currentChain->narrator=new Narrator(text);
			#endif
			}
		#endif
		}
		else if (currentType==NRC)
		{
			initializeStateData();
			currentData.sanadStartIndex=start_index;
			currentData.nrcStartIndex=start_index;
			nextState=NRC_S;
		#ifdef CHAIN_BUILDING
			initializeChainData(currentChain);
			currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,start_index);
		#endif
		#ifdef STATS
			temp_nrc_s.clear();
			map_entry * entry=new map_entry;
			entry->exact=current_exact;
			entry->stem=current_stem;
			entry->frequency=1;
			temp_nrc_s.append(entry);
			temp_nrc_count=1;
		#endif
		}
#ifdef IBN_START//needed in case a hadith starts by ibn such as "ibn yousef qal..."
		else if (currentType==NMC && isBinOrPossessive)
		{
			display("<IBN1>");
			initializeStateData();
			currentData.sanadStartIndex=start_index;
			currentData.nmcStartIndex=start_index;
			currentData.narratorStartIndex=start_index;
			currentData.nmcCount=1;
			nextState=NMC_S;
			currentData.nmcValid=true;
		#ifdef CHAIN_BUILDING
			initializeChainData(currentChain);
			currentChain->nameConnectorPrim=new NameConnectorPrim(text,start_index);
		#endif
		#ifdef STATS
			temp_nmc_s.clear();
			map_entry * entry=new map_entry;
			entry->exact=current_exact;
			entry->stem=current_stem;
			entry->frequency=1;
			temp_nmc_s.append(entry);
			temp_nmc_count=1;
		#endif
		}
#endif
		else
		{
			nextState=TEXT_S;
		}
		break;

	case NAME_S:
	#ifdef REFINEMENTS
		if(should_stop)
		{
			display("<STOP1>");
			nextState=STOP_WORD_S;
			currentData.narratorCount++;
		#ifdef CHAIN_BUILDING
			currentChain->namePrim->m_end=getLastLetter_IN_previousWord(start_index);
			currentChain->narrator->m_narrator.append(currentChain->namePrim);
			delete currentChain->temp_nameConnectors;
			currentChain->temp_nameConnectors= new TempConnectorPrimList();

			currentChain->nameConnectorPrim=new NameConnectorPrim(text,start_index); //we added this to previous name bc assumed this will only happen if it is muhamad and "sal3am"
			currentChain->nameConnectorPrim->m_end=end_pos;
			currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);
			currentChain->chain->m_chain.append(currentChain->narrator);
		#endif
			currentData.narratorEndIndex=end_pos;
		#ifdef STATS
			for (int i=temp_nmc_s.count()-temp_nmc_count;i<temp_nmc_s.count();i++)
			{
				delete temp_nmc_s[i];
				temp_nmc_s.remove(i);
			}
			for (int i=temp_nrc_s.count()-temp_nrc_count;i<temp_nrc_s.count();i++)
			{
				delete temp_nrc_s[i];
				temp_nrc_s.remove(i);
			}
			temp_nmc_count=0;
			temp_nrc_count=0;
		#endif

			//return_value= false;
			break;
		}
	#endif
		if(currentType==NMC)
		{
			nextState=NMC_S;
			currentData.nmcValid=isBinOrPossessive;
			currentData.nmcCount=1;
			currentData.nmcStartIndex=start_index;
		#ifdef CHAIN_BUILDING
			currentChain->namePrim->m_end=getLastLetter_IN_previousWord(start_index);
			currentChain->narrator->m_narrator.append(currentChain->namePrim);
			currentChain->nameConnectorPrim=new NameConnectorPrim(text,start_index);
			/*for (int i=0;i<currentChain->temp_nameConnectors->count()-1;i++)
				delete (*currentChain->temp_nameConnectors)[i];*/
			delete currentChain->temp_nameConnectors;
			currentChain->temp_nameConnectors= new TempConnectorPrimList();
		#endif
		#ifdef STATS
			map_entry * entry=new map_entry;
			entry->exact=current_exact;
			entry->stem=current_stem;
			entry->frequency=1;
			temp_nmc_s.append(entry);
			temp_nmc_count++;
		#endif
		#ifdef PUNCTUATION
			if (has_punctuation)
				currentData.nmcCount=parameters.nmc_max+1;
		#endif
		}
		else if (currentType==NRC)
		{
			nextState=NRC_S;
			currentData.narratorCount++;
		#ifdef STATS
			stat.name_per_narrator.append(temp_names_per_narrator);//found 1 name
			temp_names_per_narrator=0;

			map_entry * entry=new map_entry;
			entry->exact=current_exact;
			entry->stem=current_stem;
			entry->frequency=1;
			temp_nrc_s.append(entry);
			temp_nrc_count=1;
			temp_nmc_count=0;
		#endif
			display(QString("counter%1\n").arg(currentData.narratorCount));
			currentData.nrcCount=1;
			currentData.narratorEndIndex=getLastLetter_IN_previousWord(start_index);
			currentData.nrcStartIndex=start_index;
		#ifdef CHAIN_BUILDING
			currentChain->namePrim->m_end=getLastLetter_IN_previousWord(start_index);
			currentChain->narrator->m_narrator.append(currentChain->namePrim);
			currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,start_index);
			currentChain->chain->m_chain.append(currentChain->narrator);
			currentChain->narrator=new Narrator(text);
		#endif
		#ifdef PUNCTUATION
		#if 0
			if (has_punctuation)
				currentData.nrcCount=parameters.nrc_max;
		#endif
		#endif
		}
		else
		{
		#ifdef STATS
			if (currentType==NAME)
			{
				temp_names_per_narrator++;//found another name name
			}
		#endif
			nextState=NAME_S;
		#ifdef PUNCTUATION
			if (has_punctuation)
			{
				display("<punc2>");
				currentData.narratorCount++;
				nextState=NRC_S;
				currentData.narratorEndIndex=end_pos;
				currentData.nrcStartIndex=next_positon(end_pos,has_punctuation);
			#ifdef CHAIN_BUILDING
				currentChain->namePrim->m_end=end_pos;
				currentChain->narrator->m_narrator.append(currentChain->namePrim);
				currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,currentData.nrcStartIndex);
				currentChain->chain->m_chain.append(currentChain->narrator);
				currentChain->narrator=new Narrator(text);
			#endif
			}
		#endif
		}
		break;

	case NMC_S:
	#ifdef REFINEMENTS
		if(should_stop)
		{
			display("<STOP2>");
			//1-finish old narrator and use last nmc as nrc
			currentData.narratorCount++;
		#ifdef STATS
			stat.name_per_narrator.append(temp_names_per_narrator);//found 1 name
			temp_names_per_narrator=0;

			map_entry * entry=new map_entry;
			entry->exact=current_exact;
			entry->stem=current_stem;
			entry->frequency=1;
			temp_nrc_s.append(entry);
			temp_nrc_count=0;
			temp_nmc_count=0;
		#endif
			display(QString("counter%1\n").arg(currentData.narratorCount));
			currentData.nrcCount=1;
			nextState=NRC_S;

			currentData.narratorEndIndex=getLastLetter_IN_previousWord(currentData.nmcStartIndex);
			currentData.nrcStartIndex=currentData.nmcStartIndex;
			currentData.nrcEndIndex=getLastLetter_IN_previousWord(start_index);

		#ifdef CHAIN_BUILDING
			for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
				currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));
			currentChain->chain->m_chain.append(currentChain->narrator);
			display(currentChain->narrator->getString()+"\n");
			currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,currentData.nrcStartIndex);
			currentChain->narratorConnectorPrim->m_end=currentData.nrcEndIndex;
			currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);
			display(currentChain->narratorConnectorPrim->getString()+"\n");
		#endif
		//2-create a new narrator of just this stop word as name connector
		#ifdef CHAIN_BUILDING
			currentChain->nameConnectorPrim->m_start=start_index;
			currentChain->nameConnectorPrim->m_end=end_pos;
			currentChain->narrator=new Narrator(text);
			currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);
			display(currentChain->narrator->getString()+"\n");
			currentChain->chain->m_chain.append(currentChain->narrator);
		#endif

			currentData.narratorEndIndex=end_pos;
			nextState=STOP_WORD_S;
			currentData.narratorCount++;
		#ifdef STATS
			for (int i=temp_nmc_s.count()-temp_nmc_count;i<temp_nmc_s.count();i++)
			{
				delete temp_nmc_s[i];
				temp_nmc_s.remove(i);
			}
			for (int i=temp_nrc_s.count()-temp_nrc_count;i<temp_nrc_s.count();i++)
			{
				delete temp_nrc_s[i];
				temp_nrc_s.remove(i);
			}
			temp_nmc_count=0;
			temp_nrc_count=0;
		#endif
			//return_value= false;
			break;
		}
	#endif
		if (currentType==NRC)
		{
			currentData.narratorCount++;
		#ifdef STATS
			stat.name_per_narrator.append(temp_names_per_narrator);//found 1 name
			temp_names_per_narrator=0;

			map_entry * entry=new map_entry;
			entry->exact=current_exact;
			entry->stem=current_stem;
			entry->frequency=1;
			temp_nrc_s.append(entry);
			temp_nrc_count=1;
			temp_nmc_count=0;
		#endif
			display(QString("counter%1\n").arg(currentData.narratorCount));
			currentData.nmcCount=0;
			currentData.nrcCount=1;
			nextState=NRC_S;

			currentData.narratorEndIndex=getLastLetter_IN_previousWord(start_index);
			currentData.nrcStartIndex=start_index;
		#ifdef CHAIN_BUILDING
			currentChain->nameConnectorPrim->m_end=getLastLetter_IN_previousWord(start_index);
			currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
			for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
				currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));
			currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,start_index);
			currentChain->chain->m_chain.append(currentChain->narrator);
			currentChain->narrator=new Narrator(text);
		#endif
		}
		else if(currentType==NAME)
		{
			currentData.nmcCount=0;
			nextState=NAME_S;
		#ifdef CHAIN_BUILDING
			currentChain->nameConnectorPrim->m_end=getLastLetter_IN_previousWord(start_index);
			currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
			for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
				currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));
			currentChain->namePrim=new NamePrim(text,start_index);
		#endif
		#ifdef STATS
			temp_names_per_narrator++;//found another name name
		#endif
		#ifdef PUNCTUATION
			if (has_punctuation)
			{
				display("<punc3>");
				currentData.narratorCount++;
				nextState=NRC_S;
				currentData.narratorEndIndex=end_pos;
				currentData.nrcStartIndex=next_positon(end_pos,has_punctuation);
			#ifdef CHAIN_BUILDING
				currentChain->namePrim->m_end=end_pos;
				currentChain->narrator->m_narrator.append(currentChain->namePrim);
				currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,currentData.nrcStartIndex);
				currentChain->chain->m_chain.append(currentChain->narrator);
				currentChain->narrator=new Narrator(text);
			#endif
			}
		#endif
		}
	#ifdef PUNCTUATION
		else if (has_punctuation) //TODO: if punctuation check is all what is required
		{
			nextState=NMC_S;
			currentData.nmcCount=parameters.nmc_max+1;
			currentData.nmcValid=false;
		}
	#endif
		else if (currentData.nmcCount>parameters.nmc_max)
		{
			if (currentData.nmcValid)
			{
				currentData.nmcValid=false;
				nextState=NMC_S;
				currentData.nmcCount=0;
			}
			else
			{
				nextState=TEXT_S;
			#ifdef CHAIN_BUILDING
				/*currentChain->nameConnectorPrim->m_end=getLastLetter_IN_previousWord(start_index);//later check for out of bounds
				currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
				for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
					currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));//check to see if we should also add the narrator to chain
				*/
				currentChain->chain->m_chain.append(currentChain->narrator);
			#endif

				// TODO: added this later to the code, check if really is in correct place, but seemed necessary
				currentData.narratorCount++;
			#ifdef STATS
				for (int i=temp_nmc_s.count()-temp_nmc_count;i<temp_nmc_s.count();i++)
				{
					delete temp_nmc_s[i];
					temp_nmc_s.remove(i);
				}
				for (int i=temp_nrc_s.count()-temp_nrc_count;i<temp_nrc_s.count();i++)
				{
					delete temp_nrc_s[i];
					temp_nrc_s.remove(i);
				}
				temp_nmc_count=0;
				temp_nrc_count=0;
			#endif
				//till here was added later


				currentData.narratorEndIndex=getLastLetter_IN_previousWord(currentData.nmcStartIndex); //check this case
				return_value= false;
				break;
			}
			//currentData.narratorEndIndex=getLastLetter_IN_previousWord(start_index); check this case

		}
		else if (currentType==NMC)
		{
		#ifdef CHAIN_BUILDING
			currentChain->nameConnectorPrim->m_end=getLastLetter_IN_previousWord(start_index);
			currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
			currentChain->nameConnectorPrim=new NameConnectorPrim(text,start_index);
		#endif
			currentData.nmcCount++;
			if (isBinOrPossessive)
				currentData.nmcValid=true;
			nextState=NMC_S;
		#ifdef STATS
			map_entry * entry=new map_entry;
			entry->exact=current_exact;
			entry->stem=current_stem;
			entry->frequency=1;
			temp_nmc_s.append(entry);
			temp_nmc_count++;
		#endif
		}
		else
		{
			nextState=NMC_S; //maybe modify later
			currentChain->nameConnectorPrim->m_end=getLastLetter_IN_previousWord(start_index);
			currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
			currentChain->nameConnectorPrim=new NameConnectorPrim(text,start_index);
		#ifdef STATS
			map_entry * entry=new map_entry;
			entry->exact=current_exact;
			entry->stem=current_stem;
			entry->frequency=1;
			temp_nmc_s.append(entry);
			temp_nmc_count++;
		#endif
		}
		break;

	case NRC_S:
	#ifdef REFINEMENTS
		if(should_stop)
		{
			display("<STOP3>");
		#ifdef CHAIN_BUILDING
			//1-first add thus stop word as NMC
			currentChain->narratorConnectorPrim->m_end=getLastLetter_IN_previousWord(start_index);
			currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);
			if (currentChain->narrator!=NULL)
				delete currentChain->narrator;
			currentChain->narrator=new Narrator(text);
			currentChain->nameConnectorPrim=new NameConnectorPrim(text,start_index);
			currentChain->temp_nameConnectors->clear();
			//currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);
		#endif
			currentData.nrcEndIndex=getLastLetter_IN_previousWord(start_index);
			currentData.nmcCount=1;
		#ifdef STATS
			map_entry * entry=new map_entry;
			entry->exact=current_exact;
			entry->stem=current_stem;
			entry->frequency=1;
			temp_nmc_s.append(entry);
			temp_nmc_count++;
		#endif
			currentData.narratorStartIndex=start_index;
			currentData.nmcStartIndex=start_index;

			//2-add this narrator and end lookup
			nextState=STOP_WORD_S;
			currentData.narratorCount++;
		#ifdef CHAIN_BUILDING
			currentChain->nameConnectorPrim->m_end=end_pos;
			currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);
			currentChain->chain->m_chain.append(currentChain->narrator);
		#endif
			currentData.narratorEndIndex=end_pos;
		#ifdef STATS
			for (int i=temp_nmc_s.count()-temp_nmc_count;i<temp_nmc_s.count();i++)
			{
				delete temp_nmc_s[i];
				temp_nmc_s.remove(i);
			}
			for (int i=temp_nrc_s.count()-temp_nrc_count;i<temp_nrc_s.count();i++)
			{
				delete temp_nrc_s[i];
				temp_nrc_s.remove(i);
			}
			temp_nmc_count=0;
			temp_nrc_count=0;
		#endif
			//return_value= false;
			break;
		}
	#endif
	#ifdef PUNCTUATION
		if(currentType==NAME || currentType ==NRC)
			currentData.nrcPunctuation=false;
	#endif
	#ifdef REFINEMENTS
		if (currentType==NAME || possessive)
	#else
		if (currentType==NAME)
	#endif
		{
			nextState=NAME_S;
			currentData.nrcCount=0;
			//currentData.nameStartIndex=start_index;
		#ifdef CHAIN_BUILDING
			//currentChain->namePrim->m_start=start_index;

			currentChain->narratorConnectorPrim->m_end=getLastLetter_IN_previousWord(start_index);
			currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);
			currentChain->namePrim=new NamePrim(text,start_index);

		#endif
			currentData.nrcEndIndex=getLastLetter_IN_previousWord(start_index);
		#ifdef STATS
			temp_names_per_narrator++;//found another name name
		#endif
		#ifdef PUNCTUATION
			if (has_punctuation)
			{
				display("<punc4>");
				currentData.narratorCount++;
				nextState=NRC_S;
				currentData.narratorEndIndex=end_pos;
				currentData.nrcStartIndex=next_positon(end_pos,has_punctuation);
			#ifdef CHAIN_BUILDING
				currentChain->namePrim->m_end=end_pos;
				currentChain->narrator->m_narrator.append(currentChain->namePrim);
				currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,currentData.nrcStartIndex);
				currentChain->chain->m_chain.append(currentChain->narrator);
				currentChain->narrator=new Narrator(text);
			#endif
				break;
			}
		#endif
		}
	#ifdef PUNCTUATION
		else if (currentData.nrcCount>=parameters.nrc_max || currentData.nrcPunctuation)
	#else
		else if (currentData.nrcCount>=parameters.nrc_max)
	#endif
		{
			nextState=TEXT_S;
		#ifdef STATS
			for (int i=temp_nmc_s.count()-temp_nmc_count;i<temp_nmc_s.count();i++)
			{
				delete temp_nmc_s[i];
				temp_nmc_s.remove(i);
			}
			for (int i=temp_nrc_s.count()-temp_nrc_count;i<temp_nrc_s.count();i++)
			{
				delete temp_nrc_s[i];
				temp_nrc_s.remove(i);
			}
			temp_nmc_count=0;
			temp_nrc_count=0;
		#endif
			//currentData.nrcEndIndex=getLastLetter_IN_previousWord(start_index);
			//currentChain->narratorConnectorPrim->m_end=getLastLetter_IN_previousWord(start_index);
			//currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);

			//currentData.narratorEndIndex=getLastLetter_IN_previousWord(start_index); //check this case
			return_value= false;
			break;
		}
#ifdef TENTATIVE ////needed in case 2 3an's appear after each other intervened by a name which is unknown
		else if (currentType==NRC)
		{
			if(currentData.nrcCount>1) //in order not to have two NRCs without a name suspect between them
			{
			currentData.narratorCount++;
			display(QString("counter%1\n").arg(currentData.narratorCount));
			currentData.nrcCount=1;
			nextState=NRC_S;
			currentData.narratorEndIndex=getLastLetter_IN_previousWord(start_index);
			currentData.nrcStartIndex=start_index;

			currentChain->narratorConnectorPrim->m_end=getLastLetter_IN_previousWord(start_index);
			currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);

			currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,start_index);
			}
		}
#endif
#ifdef IBN_START
		else if (currentType==NMC && isBinOrPossessive)
		{
			display("<IBN3>");
		#ifdef CHAIN_BUILDING
			/*currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
			for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
				currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));*/
			currentChain->narratorConnectorPrim->m_end=getLastLetter_IN_previousWord(start_index);
			currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);
			if (currentChain->narrator!=NULL)
				delete currentChain->narrator;
			currentChain->narrator=new Narrator(text);
			currentChain->nameConnectorPrim=new NameConnectorPrim(text,start_index);
			currentChain->temp_nameConnectors->clear();
			//currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);
		#endif
			currentData.nrcEndIndex=getLastLetter_IN_previousWord(start_index);
			currentData.nmcValid=true;
			currentData.nmcCount=1;
		#ifdef STATS
			map_entry * entry=new map_entry;
			entry->exact=current_exact;
			entry->stem=current_stem;
			entry->frequency=1;
			temp_nmc_s.append(entry);
			temp_nmc_count++;
		#endif
			currentData.narratorStartIndex=start_index;
			currentData.nmcStartIndex=start_index;
			nextState=NMC_S;
		}
#endif
		else
		{
			nextState=NRC_S;
			currentData.nrcCount++;
		#ifdef STATS
			map_entry * entry=new map_entry;
			entry->exact=current_exact;
			entry->stem=current_stem;
			entry->frequency=1;
			temp_nrc_s.append(entry);
			temp_nrc_count++;
		#endif
		}
	#ifdef PUNCTUATION
		if (has_punctuation && nextState==NRC_S && currentType!=NAME && currentType!=NRC)
			currentData.nrcPunctuation=true;
	#endif
		break;
#ifdef REFINEMENTS
	case STOP_WORD_S:
		if (currentType==STOP_WORD)
		{
			nextState=STOP_WORD_S;
			((Narrator*)currentChain->chain->m_chain[currentChain->chain->m_chain.length()-1])->m_narrator[0]->m_end=end_pos;
			currentData.narratorEndIndex=end_pos;
		}
		else
		{
			nextState=TEXT_S;
			return_value=false;
		}
		break;
#endif
	default:
		break;
	}

#ifdef REFINEMENTS
	currentData.ibn_or_3abid=ibn_or_3abid; //for it to be saved for next time use
#endif
	return return_value;
}
#ifdef STATS
void show_according_to_frequency(QList<int> freq,QList<QString> words)
{
	QList<QPair<int, QString> > l;
	for (int i=0;i<freq.size() && i<words.size();i++)
		l.append(QPair<int,QString>(freq[i],words[i]));
	qSort(l.begin(),l.end());
	for (int i=l.size()-1;i>=0;i--)
		displayed_error <<"("<<l[i].first<<") "<<l[i].second<<"\n";
}
#endif

#ifdef IMAN_CODE
class adjective_stemmer: public Stemmer
{
public:
	bool adj ;
	long long finish_pos;

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
	QFile input(input_str.split("\n")[0]);
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
	long long text_size=text->size();

	while(current_pos<text->length() && delimiters.contains(text->at(current_pos)))
		current_pos++;
	for (;current_pos<text_size;)
	{
		adjective_stemmer s(text,current_pos);
		s();
		long long finish=max(s.info.finish,s.finish_pos);
		if (s.adj )
			out <<text->mid(current_pos,finish-current_pos+1)+":ADJECTIVE\n";
		current_pos=next_positon(finish);;//here current_pos is changed
	}
	return 0;
}
#endif

#ifdef BUCKWALTER_INTERFACE
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

wordType getWordType(bool & isBinOrPossessive,bool & possessive, long long &next_pos)
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

int hadith(QString input_str,ATMProgressIFC *prg)
{
#ifdef IMAN_CODE
	return adjective_detector(input_str);
#endif
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
	QFile input(input_str.split("\n")[0]);
	if (!input.open(QIODevice::ReadOnly))
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
	long long text_size=text->size();
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
	stateType currentState=TEXT_S;
	stateType nextState=TEXT_S;
	wordType currentType;
	long long  newHadithStart=-1;

	initializeStateData();

#ifdef CHAIN_BUILDING
	chainData *currentChain=new chainData();
	initializeChainData(currentChain);
#else
	chainData *currentChain=NULL;
#endif
	long long  sanadEnd;
	int hadith_Counter=1;
#endif
	current_pos=0;
#ifndef BUCKWALTER_INTERFACE
	while(current_pos<text->length() && isDelimiter(text->at(current_pos)))
		current_pos++;
#else
	QString line =getnext();//just for first line
	assert(line=="");
#endif
	for (;current_pos<text_size;)
	{
#ifndef COMPARE_TO_BUCKWALTER
		long long start=current_pos;
		long long next;
		bool isBinOrPossessive=false,possessive=false,ibn_or_3abid=false,has_punctuation=false;
		currentType=getWordType(isBinOrPossessive,possessive,ibn_or_3abid,has_punctuation,next);
		current_pos=next;//here current_pos is changed
		long long last_letter=next>=text_size?text_size-1:getLastLetter_IN_previousWord(next);
		if((getNextState(currentState,currentType,nextState,start,isBinOrPossessive,possessive,ibn_or_3abid,has_punctuation,last_letter,currentChain)==false))
		{
			if (currentData.narratorCount>=parameters.narr_min)
			{
				sanadEnd=currentData.narratorEndIndex;
			#ifdef DISPLAY_HADITH_OVERVIEW
				newHadithStart=currentData.sanadStartIndex;
				long long end=text->indexOf(QRegExp(delimiters),sanadEnd);//sanadEnd is first letter of last word in sanad
				out<<"\n"<<hadith_Counter<<" new hadith start: "<<text->mid(newHadithStart,display_letters)<<endl;
				out<<"sanad end: "<<text->mid(end-display_letters,display_letters)<<endl<<endl;
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
			else
			{
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
		currentState=nextState;
#else
		hadith_stemmer s(text,current_pos);
		s();
		long long finish=max(s.info.finish,s.finish_pos);
		if (finish==current_pos)
			finish=getLastLetter_IN_currentWord(current_pos);
		current_pos=next_positon(finish);
#endif
#ifdef PROGRESSBAR
		prg->report((double)current_pos/text_size*100+0.5);
		if (current_pos==text_size-1)
			break;
#endif
	}
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
#endif
#if defined(TAG_HADITH)
	prg->startTaggingText(*text);
#endif
	chains.clear();
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
				prg->tag(curr_struct->getStart(),curr_struct->getLength(),Qt::darkYellow,false);
				Narrator * n=(Narrator *)curr_struct;
				for (int i=0;i<n->m_narrator.size();i++)
				{
					NarratorPrim * nar_struct=n->m_narrator[i];
					if (nar_struct->isNamePrim())
						prg->tag(nar_struct->getStart(),nar_struct->getLength(),Qt::white,true);
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
	test_GraphFunctionalities(chains, prg);
#endif
#endif
#ifndef TAG_HADITH
	prg->startTaggingText(*hadith_out.string()); //we will not tag but this will force a text to be written there
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
	return 0;
}
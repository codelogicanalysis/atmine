#include "hadith.h"

#ifdef GENERAL_HADITH
#include <QFile>
#include <QRegExp>
#include <QString>
#include <QStringList>

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
#include <assert.h>

	enum wordType { NAME, NRC,NMC};
	enum stateType { TEXT_S , NAME_S, NMC_S , NRC_S};
	QStringList compound_words;
	QString hadath;
	long abstract_NAME, abstract_POSSESSIVE, abstract_PLACE;
	int bit_NAME, bit_POSSESSIVE, bit_PLACE;
#ifdef PREPROCESS_DESCRIPTIONS
	QHash<long,bool> NMC_descriptions;
	QHash<long,bool> NRC_descriptions;
#endif
#ifdef COMPARE_TO_BUCKWALTER
	QTextStream * myoutPtr;
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
	out<<type_to_text(t)<<endl;
	//qDebug() <<type_to_text(t);
}
inline void display(QString t)
{
	out<<t;
	//qDebug() <<t;
}
#else
#define display(c) ;
#endif



#define display_letters 30
typedef struct chainData_ {
    NamePrim *namePrim;
    NameConnectorPrim *nameConnectorPrim;
    NarratorConnectorPrim *narratorConnectorPrim;
    Narrator *narrator;
    Chain *chain;
} chainData;
typedef struct stateData_ {
	long long  sanadStartIndex,nmcThreshold, narratorCount,nrcThreshold,narratorStartIndex,narratorEndIndex,nrcStartIndex,nrcEndIndex,narratorThreshold,;
	long long  nmcCount, nrcCount,nameStartIndex,nmcStartIndex;
	bool nmcValid;
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
int current_pos;
#ifdef CHAIN_BUILDING
void initializeChainData(chainData *currentChain)
{
	delete currentChain->namePrim;
	delete currentChain->nameConnectorPrim;
	delete currentChain->narratorConnectorPrim;
	delete currentChain->narrator;
	delete currentChain->chain;
	currentChain-> namePrim=new NamePrim(text);
	currentChain-> nameConnectorPrim=new NameConnectorPrim(text);
	currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text) ;
	currentChain->  narrator=new Narrator (text);
	currentChain->  chain=new Chain(text);
	display(QString("\ninit%1\n").arg(currentChain->narrator->m_narrator.size()));
}
#endif

void hadith_initialize()
{
	hadath.append(_7a2).append(dal).append(tha2);
#ifdef REFINEMENTS
	abstract_NAME=get_abstractCategory_id("Male Names");
#else
	abstract_NAME=get_abstractCategory_id("Name of Person");
#endif
	abstract_POSSESSIVE=get_abstractCategory_id("POSSESSIVE");
	abstract_PLACE=get_abstractCategory_id("Name of Place");
	bit_NAME=get_bitindex(abstract_NAME,abstract_category_ids);
	bit_POSSESSIVE=get_bitindex(abstract_POSSESSIVE,abstract_category_ids);
	bit_PLACE=get_bitindex(abstract_PLACE,abstract_category_ids);
#ifdef REFINEMENTS
	QFile input("../src/case/phrases"); //contains compound words or phrases
									   //maybe if later number of words becomes larger we save it into a trie and thus make their finding in a text faster
	if (!input.open(QIODevice::ReadOnly))
		return;
	QTextStream file(&input);
	file.setCodec("utf-8");
	QString phrases=file.readAll();
	if (phrases.isNull() || phrases.isEmpty())
		return;
	compound_words=phrases.split("\n",QString::SkipEmptyParts);
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
	Retrieve_Template nrc_s("description","id","name='said' OR name='say' OR name='notify/communicate' OR name LIKE '%/listen' OR name LIKE 'listen/%' OR name LIKE 'listen %' OR name LIKE '% listen' OR name = 'listen' OR name LIKE '%from/about%' OR name LIKE '%narrate%'");
	while (nrc_s.retrieve())
		NRC_descriptions.insert(nrc_s.get(0).toULongLong(),true);
	Retrieve_Template nmc_s("description","id","name='son'");
	while (nmc_s.retrieve())
		NMC_descriptions.insert(nmc_s.get(0).toULongLong(),true);
#endif
}

void initializeStateData()
{
	currentData.nmcThreshold=3;
	currentData.nmcCount=0;
	currentData.narratorCount=0;
	currentData.nrcThreshold=5;
	currentData.nrcCount=0;
	currentData.narratorStartIndex=0;
	currentData.narratorEndIndex=0;
	currentData.nrcStartIndex=0;
	currentData.nrcEndIndex=0;
	currentData.narratorThreshold=3;
	currentData.nmcValid=false;
	currentData.nameStartIndex=0;
	currentData.nmcStartIndex=0;
}

class hadith_stemmer: public Stemmer
{
private:
	bool place;
public:
	bool name, nrc, nmc,possessive;
	long long finish_pos;
#ifdef STATS
	QString stem;
	QList<QString> stems;
#endif

	hadith_stemmer(QString * word, int start):Stemmer(word,start,false)
	{
		name=false;
		nmc=false;
		nrc=false;
		possessive=false;
		place=false;
		finish_pos=start;

	#ifdef STATS
		stem="";
		stems.clear();
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
			if (!analyze())
				return false;
		}while (Stem->computeNextSolution(S_inf));
		delete S_inf;
		return true;
	#endif
	}

	bool analyze()
	{
	#ifdef STATS
		QString temp_stem=removeDiacritics(diacritic_text->mid(Stem->starting_pos, Suffix->startingPos-Stem->starting_pos));//removeDiacritics(stem_info->raw_data);
	#endif
	#ifndef PREPROCESS_DESCRIPTIONS
		QString description=stem_info->description();
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
		#ifdef STATS
			stems.append(temp_stem);
		#endif
		return true;
	}
#else
	bool on_match()
	{
		QTextStream & myout=*myoutPtr;
		int count=0;
		int number=0;
		if (called_everything || type==PREFIX)
		{
			myout<<"(";
			for (int i=0;i<prefix_infos->count();i++) //TODO: results with incorrect behaviour assuming more than 1 category works for any item
			{
				if (number>0)
					myout<<" + ";
				number++;
				if (count>0)
					myout<<" OR ";
				count++;
				const minimal_item_info & rmii = prefix_infos->at(i);
				myout<<rmii.description();
			}
			myout<<")-";
		}
		if (called_everything || type==STEM)
		{
			myout<<"-(";
			count=0;
			if (count>0)
				myout<<" OR ";
			count++;
			myout<<stem_info->description();
			myout<<" [ ";
			for (unsigned int i=0;i<stem_info->abstract_categories.length();i++)
				if (stem_info->abstract_categories[i])
					if (get_abstractCategory_id(i)>=0)
						myout<<getColumn("category","name",get_abstractCategory_id(i))<<" ";
			myout<<"])-";
		}
		if (called_everything || type==SUFFIX)
		{
			myout<<"-(";
			number=0;
			count=0;
			for (int i=0;i<suffix_infos->count();i++)
			{
				if (number>0)
					myout<<" + ";
				number++;
				if (count>0)
					myout<<" OR ";
				count++;
				myout<< suffix_infos->at(i).description();
			}
			myout<<")";
		}
		if (called_everything)
		{
			QString word;
			for (int i=0;i<prefix_infos->count();i++)
				word.append(prefix_infos->at(i).raw_data);
			word.append(stem_info->raw_data);
			for (int i=0;i<suffix_infos->count();i++)
				word.append(suffix_infos->at(i).raw_data);
			myout<<" "+word+" ";
		}
		myout<<" "<<info.finish+1<<"\n";
		return true;
	}
#endif
};

long long next_positon(long long finish)
{
	finish++;
	while(finish<text->length() && delimiters.contains(text->at(finish)))
		finish++;
	return finish;
}

long long getLastLetter_IN_previousWord(long long start_letter_current_word)
{
	start_letter_current_word--;
	while(start_letter_current_word>=0 && delimiters.contains(text->at(start_letter_current_word)))
		start_letter_current_word--;
	return start_letter_current_word;
}

long long getLastLetter_IN_currentWord(long long start_letter_current_word)
{
	int size=text->length();
	while(start_letter_current_word<size && !delimiters.contains(text->at(start_letter_current_word)))
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
wordType getWordType(bool & isBinOrPossessive,bool & possessive, long long &next_pos)
{
	long long  finish;
	isBinOrPossessive=false;
	hadith_stemmer s(text,current_pos);
	s.setSolutionSettings(M_ALL);
#ifdef REFINEMENTS
	QString c;
	bool found,phrase=false;
	foreach (c, compound_words)
	{
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
			phrase=true;
			finish=pos-1;
		#ifdef STATS
			current_stem=c;
			current_exact=c;
		#endif
			break;
		}
	}
	if (!phrase)
	{
#endif
		s();
		finish=max(s.info.finish,s.finish_pos);
		if (finish==current_pos)
			finish=getLastLetter_IN_currentWord(current_pos);
		#ifdef STATS
			current_exact=removeDiacritics(s.diacritic_text->mid(current_pos,finish-current_pos+1));
			current_stem=s.stem;
			if (current_stem=="")
				current_stem=choose_stem(s.stems);
			if (current_stem=="")
				current_stem=current_exact;
		#endif
#ifdef REFINEMENTS
	}
#endif
	next_pos=next_positon(finish);
	display(text->mid(current_pos,finish-current_pos+1)+":");
#ifdef REFINEMENTS
	if (phrase)
	{
		display("PHRASE");
		//isBinOrPossessive=true; //same behaviour as Bin
		return NMC;
	}
#endif
	possessive=s.possessive;
	if (s.nrc )
		return result(NRC);
	else if (s.name)
		return result(NAME);
	else if (s.nmc)
	{
		display("NMC-Bin/Pos ");
		isBinOrPossessive=true;
		return NMC;
	}
	else
		return result(NMC);
}

bool getNextState(stateType currentState,wordType currentType,stateType & nextState,long long  start_index,bool isBinOrPossessive,bool possessive,chainData *currentChain)
{
	display(QString(" nmcsize: %1 ").arg(currentData.nmcCount));
	display(QString(" nrcsize: %1 ").arg(currentData.nrcCount));
	display(currentState);

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
#ifdef TENTATIVE//needed in case a hadith starts by ibn such as "ibn yousef qal..."
		else if (currentType==NMC && isBinOrPossessive)
		{
			initializeStateData(currentData);
			initializeChainData(currentChain);
			currentData.narratorStartIndex=start_index;
			currentData.sanadStartIndex=start_index;
			currentData.nmcStartIndex=start_index;
			currentChain->nameConnectorPrim=new NameConnectorPrim(text,start_index);
			nextState=NAME_S;
		}
#endif
		else
		{
			nextState=TEXT_S;
		}
		return true;

	case NAME_S:
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
		#endif
		#ifdef STATS
			map_entry * entry=new map_entry;
			entry->exact=current_exact;
			entry->stem=current_stem;
			entry->frequency=1;
			temp_nmc_s.append(entry);
			temp_nmc_count++;
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
		}
		else
		{
			if (currentType==NAME)
			{
			#ifdef STATS
				temp_names_per_narrator++;//found another name name
			#endif
			}
			nextState=NAME_S;
		}
		return true;

	case NMC_S:
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
			currentData.nrcCount=1;
			nextState=NRC_S;

			currentData.narratorEndIndex=getLastLetter_IN_previousWord(start_index);
			currentData.nrcStartIndex=start_index;
		#ifdef CHAIN_BUILDING
			currentChain->nameConnectorPrim->m_end=getLastLetter_IN_previousWord(start_index);
			currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);

			currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,start_index);
			currentChain->chain->m_chain.append(currentChain->narrator);
			currentChain->narrator=new Narrator(text);
		#endif
		}
		else if(currentType==NAME)
		{
			nextState=NAME_S;
		#ifdef CHAIN_BUILDING
			currentChain->nameConnectorPrim->m_end=getLastLetter_IN_previousWord(start_index);
			currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);
			currentChain->namePrim=new NamePrim(text,start_index);
		#endif
		#ifdef STATS
			temp_names_per_narrator++;//found another name name
		#endif
		}
		else if (currentData.nmcCount>currentData.nmcThreshold)
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
				currentChain->nameConnectorPrim->m_end=getLastLetter_IN_previousWord(start_index);//later check for out of bounds
				currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);//check to see if we should also add the narrator to chain
				currentChain->chain->m_chain.append(currentChain->narrator);
			#endif
				currentData.narratorEndIndex=getLastLetter_IN_previousWord(start_index); //check this case
				return false;
			}
			//currentData.narratorEndIndex=getLastLetter_IN_previousWord(start_index); check this case

		}
		else if (currentType==NMC)
		{
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
		#ifdef STATS
			map_entry * entry=new map_entry;
			entry->exact=current_exact;
			entry->stem=current_stem;
			entry->frequency=1;
			temp_nmc_s.append(entry);
			temp_nmc_count++;
		#endif
		}
		return true;

	case NRC_S:
#ifdef REFINEMENTS
		if (currentType==NAME || possessive)
#else
		if (currentType==NAME)
#endif
		{
			nextState=NAME_S;
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
		}
		else if (currentData.nrcCount>=currentData.nrcThreshold)
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
			return false;
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
		return true;
	default:
		return true;
	}
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
					finish_pos=finish;
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
		long long start=current_pos;
		long long next;
		adjective_stemmer s(text,current_pos);
		s();
		long long finish=max(s.finish,s.finish_pos);
		if (s.adj )
			out <<text->mid(current_pos,finish-current_pos+1)+":ADJECTIVE\n";
		current_pos=next_positon(finish);;//here current_pos is changed
	}
	return 0;
}
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
	myoutPtr= new QTextStream(&myfile);
#else
	QFile chainOutput("chainOutput");

	chainOutput.remove();
	if (!chainOutput.open(QIODevice::ReadWrite))
		return 1;
	QDataStream chainOut(&chainOutput);
#endif
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
	}
	if (text->isEmpty()) //ignore empty files
	{
		out<<"empty file\n";
		return 0;
	}
	long long text_size=text->size();
	stateType currentState=TEXT_S;
	stateType nextState=TEXT_S;
	wordType currentType;
	long long  newHadithStart=-1;

	initializeStateData();
	current_pos=0;
	while(current_pos<text->length() && delimiters.contains(text->at(current_pos)))
		current_pos++;
#ifdef CHAIN_BUILDING
	chainData *currentChain=new chainData();
	initializeChainData(currentChain);
#else
	chainData *currentChain=NULL;
#endif
	long long  sanadEnd;
	int hadith_Counter=1;

	for (;current_pos<text_size;)
	{
		long long start=current_pos;
		long long next;
		bool isBinOrPossessive=false,possessive=false;
		currentType=getWordType(isBinOrPossessive,possessive,next);
		current_pos=next;//here current_pos is changed
#ifndef COMPARE_TO_BUCKWALTER
		if((getNextState(currentState,currentType,nextState,start,isBinOrPossessive,possessive,currentChain)==false))
		{
			if (currentData.narratorCount>=currentData.narratorThreshold)
			{
				sanadEnd=currentData.narratorEndIndex;
#if 1
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
#endif
		currentState=nextState;
#if 1
		prg->report((double)current_pos/text_size*100+0.5);
		if (current_pos==text_size-1)
			break;
#endif
	}
#ifndef COMPARE_TO_BUCKWALTER
	if (newHadithStart<0)
	{
		out<<"no hadith found\n";
		chainOutput.close();
		return 0;
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
	while (!tester.atEnd())
	{
		Chain * s=new Chain(text);
		s->deserialize(tester);
		hadith_out<<tester_Counter<<" ";
		s->serialize(hadith_out);
		tester_Counter++;
		s->serialize(file_hadith);
		delete s;
	}
	chainOutput.close();
	f.close();
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
	return 0;
}

#endif

#include "hadith.h"

#ifdef GENERAL_HADITH
#include <QFile>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include "../logger/logger.h"
#include "../sql-interface/Search_by_item.h"
#include "../sarf/stemmer.h"
#include "../utilities/letters.h"
#include "../utilities/text_handling.h"
#include "../utilities/diacritics.h"
#include "narrator_abstraction.h"
#include "../common_structures/common.h"

#include "ATMProgressIFC.h"

enum wordType { NAME, NRC,NMC};
enum stateType { TEXT_S , NAME_S, NMC_S , NRC_S};
QStringList compound_words;
QString hadath,alayhi_alsalam;
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
inline void display(wordType) {}
inline void display(stateType){}
inline void display(QString)  {}
#endif

#define display_letters 30

typedef struct chainData_{
    NamePrim *namePrim;
    NameConnectorPrim *nameConnectorPrim;
    NarratorConnectorPrim *narratorConnectorPrim;
    Narrator *narrator;
    Chain *chain;
} chainData;
typedef struct stateData_{
	long long  sanadStartIndex,nmcThreshold, narratorCount,nrcThreshold,narratorStartIndex,narratorEndIndex,nrcStartIndex,nrcEndIndex,narratorThreshold,;
	long long  nmcCount, nrcCount,nameStartIndex,nmcStartIndex;
	bool nmcValid;
} stateData;

#ifdef STATS
typedef struct statistics_{
	int names_in, names_out, chains, narrators;
	QVector<int> narrator_per_chain, name_per_narrator;
} statistics;
statistics stat;
int temp_names_per_narrator;
#endif

stateData currentData;
QString * text;
int current_pos;

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

void hadith_initialize()
{
	QString alayhi,alsalam;
	hadath.append(_7a2).append(dal).append(tha2);
	alayhi.append(ayn).append(lam).append(ya2).append(ha2);
	alsalam.append(alef).append(lam).append(seen).append(lam).append(alef).append(meem);
	alayhi_alsalam=alayhi.append(' ').append(alsalam);

	QFile input("test-cases/phrases"); //contains compound words or phrases
									   //maybe if later number of words becomes larger we save it into a trie and thus make their finding in a text faster
	if (!input.open(QIODevice::ReadOnly))
		return;
	QTextStream file(&input);
	file.setCodec("utf-8");
	QString phrases=file.readAll();
	if (phrases.isNull() || phrases.isEmpty())
		return;
	compound_words=phrases.split("\n",QString::SkipEmptyParts);
#ifdef STATS
	stat.chains=0;
	stat.names_in=0;
	stat.names_out=0;
	stat.narrators=0;
	stat.name_per_narrator.clear();
	stat.narrator_per_chain.clear();
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
	bool possessive,place;
public:
	bool name, nrc, nmc ;
	long long finish_pos;

	hadith_stemmer(QString * word, int start):Stemmer(word,start/*,false*/)
	{
		name=false;
		nmc=false;
		nrc=false;
		possessive=false;
		place=false;
		finish_pos=start;
	}
	bool on_match()
	{
		if (removeDiacritics(stem_info->raw_data)==hadath)
		{
			nrc=true;
			finish_pos=finish;
			return false;
		}
		else if (stem_info->description=="son")
		{
			nmc=true;
			finish_pos=finish;
			return false;
		}
		else if (stem_info->description=="said" || stem_info->description=="say" || stem_info->description=="notify/communicate" || stem_info->description.split(QRegExp("[ /]")).contains("listen") || stem_info->description.contains("from/about")||stem_info->description.contains("narrate"))
		{
			nrc=true;
			finish_pos=finish;
			return false;
		}
		for (unsigned int i=0;i<stem_info->abstract_categories.size();i++)
			if (stem_info->abstract_categories[i] && get_abstractCategory_id(i)>=0)
			{
				QString abstract_category=getColumn("category","name",get_abstractCategory_id(i));
				if (abstract_category=="Male Names") //Name of Person
				{
					name=true;
					if (finish>finish_pos)
						finish_pos=finish;
					return true;
				}
				else if (abstract_category=="POSSESSIVE")
				{
					possessive=true;
					if (place)
					{
						nmc=true;
						finish_pos=finish;
						return false;
					}
				}
				else if (abstract_category=="Name of Place")
				{
					place=true;
					if (possessive)
					{
						nmc=true;
						finish_pos=finish;
						return false;
					}
				}
			}
		return true;
	}
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
	//start_letter_current_word++;
	int size=text->length();
	while(start_letter_current_word<size && !delimiters.contains(text->at(start_letter_current_word)))
		start_letter_current_word++;
	return start_letter_current_word;
}

inline wordType result(wordType t){display(t); return t;}
wordType getWordType(bool & isBinOrPossessive, long long &next_pos)
{
	long long  finish;
	isBinOrPossessive=false;
	QString c;
	bool found,phrase=false;
	foreach (c, compound_words)
	{
		found=true;
		int pos=current_pos;
		/*qDebug()<< c<< " - "<<text->mid(current_pos,c.length());
		if (equal( c,text->mid(current_pos,c.length())))
			qDebug()<<"should be";*/
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
			//qDebug()<<"found";
			phrase=true;
			finish=pos-1;
			//qDebug()<<text->mid(current_pos,finish-current_pos+1);
			break;
		}
	}
	hadith_stemmer s(text,current_pos);
	if (!phrase)
	{
		s();
		finish=max(s.finish,s.finish_pos);
		if (finish==current_pos)
			finish=getLastLetter_IN_currentWord(current_pos);
		//qDebug()<<text->mid(current_pos,finish-current_pos+1);
	}
	next_pos=next_positon(finish);
	display(text->mid(current_pos,finish-current_pos+1)+":");
#ifdef TENTATIVE //needed in order not consider 'alayhi_alsalam' as an additional NMC which results in it not being counted in case it is the last allowable count of NMC...
	if (word==alayhi_alsalam)
		return result(NRC);
#endif
	if (phrase)
	{
		display("PHRASE");
		isBinOrPossessive=true; //same behaviour as Bin
		return NMC;
	}
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

bool getNextState(stateType currentState,wordType currentType,stateType & nextState,long long  start_index,bool isBinOrPossessive,chainData *currentChain)
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
				initializeChainData(currentChain);
				nextState=NAME_S;
				currentData.sanadStartIndex=start_index;
				currentData.narratorStartIndex=start_index;
				currentChain->namePrim=new NamePrim(text,start_index);
			#ifdef STATS
				temp_names_per_narrator=1;
			#endif
			}
			else if (currentType==NRC)
			{
				initializeStateData();
				initializeChainData(currentChain);
				currentData.sanadStartIndex=start_index;
				currentData.nrcStartIndex=start_index;
				currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,start_index);
				nextState=NRC_S;
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

				currentChain->namePrim->m_end=getLastLetter_IN_previousWord(start_index);
				currentChain->narrator->m_narrator.append(currentChain->namePrim);

				currentChain->nameConnectorPrim=new NameConnectorPrim(text,start_index);

			}
			else if (currentType==NRC)
			{
				nextState=NRC_S;
				currentData.narratorCount++;
			#ifdef STATS
				stat.name_per_narrator.append(temp_names_per_narrator);//found 1 name
				temp_names_per_narrator=0;
			#endif
				display(QString("counter%1\n").arg(currentData.narratorCount));
				currentData.nrcCount=1;
				currentData.narratorEndIndex=getLastLetter_IN_previousWord(start_index);
				currentData.nrcStartIndex=start_index;

				currentChain->namePrim->m_end=getLastLetter_IN_previousWord(start_index);
				currentChain->narrator->m_narrator.append(currentChain->namePrim);

				currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,start_index);
				currentChain->chain->m_chain.append(currentChain->narrator);
				currentChain->narrator=new Narrator(text);
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
			#endif
				display(QString("counter%1\n").arg(currentData.narratorCount));
				currentData.nrcCount=1;
				nextState=NRC_S;

				currentData.narratorEndIndex=getLastLetter_IN_previousWord(start_index);
				currentData.nrcStartIndex=start_index;

				currentChain->nameConnectorPrim->m_end=getLastLetter_IN_previousWord(start_index);
				currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);

				currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,start_index);
				currentChain->chain->m_chain.append(currentChain->narrator);
				currentChain->narrator=new Narrator(text);
			}
			else if(currentType==NAME)
			{
				nextState=NAME_S;
				currentChain->nameConnectorPrim->m_end=getLastLetter_IN_previousWord(start_index);
				currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);
				currentChain->namePrim=new NamePrim(text,start_index);
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
					currentChain->nameConnectorPrim->m_end=getLastLetter_IN_previousWord(start_index);//later check for out of bounds
					currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);//check to see if we should also add the narrator to chain
					currentChain->chain->m_chain.append(currentChain->narrator);
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
			}
			else
			{
				nextState=NMC_S; //maybe modify later
			}
			return true;

		case NRC_S:

			if (currentType==NAME)
			{
				nextState=NAME_S;
				//currentData.nameStartIndex=start_index;
				//currentChain->namePrim->m_start=start_index;
				currentChain->narratorConnectorPrim->m_end=getLastLetter_IN_previousWord(start_index);
				currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);
				currentChain->namePrim=new NamePrim(text,start_index);
				currentData.nrcEndIndex=getLastLetter_IN_previousWord(start_index);
			#ifdef STATS
				temp_names_per_narrator++;//found another name name
			#endif
			}
			else if (currentData.nrcCount>=currentData.nrcThreshold)
			{
				nextState=TEXT_S;
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
			}
			return true;
		default:
			return true;
		}
	}

#if 0
int parse(QString & text,QStringList & list)//returns number of times compound words were found
{
	int count=0;
	list=text.split((QRegExp(delimiters)),QString::SkipEmptyParts);
	QString c;
	for (long long  i=0;i<list.size();i++)
	{
		foreach (c,compound_words)
		{
			int s=list.size();
			QStringList w=c.split(QString(' '),QString::SkipEmptyParts);
			int j;
			for(j=0;j<w.size() && j+i<s;j++)
			{
				if (!equal(w[j],list[j+i]))
					break;
			}
			if (j==w.size())
			{
				count++;
				for (int k=1;k<j;k++)
				{
					list[i].append(' ').append(list[i+1]);
					list.removeAt(i+1);
				}

			}
		}
	}
	return count;
}
#endif
int hadith(QString input_str,ATMProgressIFC *prg)
{

	QFile chainOutput("test-cases/chainOutput");

	chainOutput.remove();
	if (!chainOutput.open(QIODevice::ReadWrite))
		return 1;

	//chainOutput.flush();
	QDataStream chainOut(&chainOutput);
	//chainOut.setCodec("utf-8");
	//chainOut.setFieldAlignment(QTextStream::AlignCenter);

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
#if 0 //just to test parsing
	QStringList wordList;
	parse(*text,wordList);
	for (long long  i=0;i<wordList.size();i++)
		out<<wordList[i]<<"_";
	return 0;
#endif
	long long text_size=text->size();
	stateType currentState=TEXT_S;
	stateType nextState=TEXT_S;
	wordType currentType;
	long long  newHadithStart=-1;

	initializeStateData();
	current_pos=0;
	while(current_pos<text->length() && delimiters.contains(text->at(current_pos)))
		current_pos++;

	chainData *currentChain=new chainData();
	initializeChainData(currentChain);

	long long  sanadEnd;

	bool isBinOrPossessive=false;
        int hadith_Counter=1;

	for (;current_pos<text_size;)
	{
		long long start=current_pos;
		long long next;
		currentType=getWordType(isBinOrPossessive,next);
		current_pos=next;//here current_pos is changed
		if((getNextState(currentState,currentType,nextState,start,isBinOrPossessive,currentChain)==false))
		{
			if (currentData.narratorCount>=currentData.narratorThreshold)
			{
				sanadEnd=currentData.narratorEndIndex;
				newHadithStart=currentData.sanadStartIndex;
				out<<"\n"<<hadith_Counter<<" new hadith start: "<<text->mid(newHadithStart,display_letters)<<endl;
				long long end=text->indexOf(QRegExp(delimiters),sanadEnd);//sanadEnd is first letter of last word in sanad
				out<<"sanad end: "<<text->mid(end-display_letters,display_letters)<<endl<<endl;
				currentChain->chain->serialize(chainOut);
				//currentChain->chain->serialize(displayed_error);
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
			#endif
			}
		}
		currentState=nextState;
		prg->report((double)current_pos/text_size*100+0.5);
		if (current_pos==text_size-1)
			break;
	}
	if (newHadithStart<0)
	{
		out<<"no hadith found\n";
		chainOutput.close();
		return 0;
	}
	chainOutput.close();
#if 1 //just for testing deserialize
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
	double avg_narrators_per_chain=0;
	for (int i=0;i<stat.narrator_per_chain.size();i++)
		avg_narrators_per_chain+=stat.narrator_per_chain[i];
	avg_narrators_per_chain/=stat.narrator_per_chain.size();
	double avg_names_per_narrator=0;
	for (int i=0;i<stat.name_per_narrator.size();i++)
		avg_names_per_narrator+=stat.name_per_narrator[i];
	avg_names_per_narrator/=stat.name_per_narrator.size();

	displayed_error	<<"Chains=\t\t"<<stat.chains<<"\n"
					<<"Narrators=\t\t"<<stat.narrators<<"\n"
					<<"Names IN=\t\t"<<stat.names_in<<"\n"
					<<"Names OUT=\t\t"<<stat.names_out<<"\n"
					<<"Avg Names/Narr=\t"<<avg_names_per_narrator<<"\n"
					<<"Avg Narr/Chain=\t"<<avg_narrators_per_chain<<"\n";
#endif
	return 0;
}

#endif

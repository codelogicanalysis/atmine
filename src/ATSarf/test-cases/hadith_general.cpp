#include "hadith.h"

#ifdef GENERAL_HADITH
#include <QFile>
#include <QRegExp>
#include <QStringList>
#include "logger/logger.h"
#include "../sql-interface/Search_by_item.h"
#include "../sarf/stemmer.h"
#include "../utilities/letters.h"
#include "../utilities/text_handling.h"
#include "narrator_abstraction.h"

enum wordType { NAME, NRC,NMC};
enum stateType { TEXT_S , NAME_S, NMC_S , NRC_S};
QString delimiters(" :.,()");
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
}
inline void display(stateType t)
{
	out<<type_to_text(t)<<endl;
}
inline void display(QString t)
{
	out<<t;
}
#else
inline void display(wordType) {}
inline void display(stateType){}
inline void display(QString)  {}
#endif

typedef struct chainData_{
    NamePrim *namePrim;
    NameConnectorPrim *nameConnectorPrim;
    NarratorConnectorPrim *narratorConnectorPrim;
    Narrator *narrator;
    Chain *chain;
} chainData;

void initializeChainData(chainData *currentChain){

   currentChain-> namePrim=new NamePrim();
   currentChain-> nameConnectorPrim=new NameConnectorPrim ();
   currentChain->narratorConnectorPrim=new NarratorConnectorPrim ;
   currentChain->  narrator=new Narrator ();
   currentChain->  chain=new Chain();
}


typedef struct stateData_{
//	bool started;
        long long  sanadStartIndex,nmcThreshold, narratorCount,nrcThreshold,narratorStartIndex,narratorEndIndex,nrcStartIndex,nrcEndIndex,narratorThreshold,;
        long long  nmcCount, nrcCount,nameStartIndex,nmcStartIndex;
	bool nmcValid;

	// QList <QString>nmcList;
   // QList <QString>nrcList;
} stateData;

void hadith_initialize()
{
	QString alayhi,alsalam;
	hadath.append(_7a2).append(dal).append(tha2);
	alayhi.append(ayn).append(lam).append(ya2).append(ha2);
	alsalam.append(alef).append(lam).append(seen).append(lam).append(alef).append(meem);
	delimiters="["+delimiters+fasila+"]";
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

}

void initializeStateData(stateData & currentData)
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

class mystemmer: public Stemmer
{
private:
	bool possessive,place;
public:
	bool name, nrc, nmc ;

	mystemmer(QString word):Stemmer(word)
	{
		name=false;
		nmc=false;
		nrc=false;
		possessive=false;
		place=false;
	}
	bool on_match()
	{
#if !defined (REDUCE_THRU_DIACRITICS)
				Stem->fill_details();
#endif
				minimal_item_info stem_info;
				//out<<Suffix->startingPos-1<<" "<<getColumn("category","name",Stem->category_of_currentmatch)<<" --- ";
				Search_by_item s(STEM,Stem->id_of_currentmatch);
				while(s.retrieve(stem_info))
				{
						if (s.Name()==hadath)
						{
							nrc=true;
							return false;
						}

#ifdef REDUCE_THRU_DIACRITICS
						if (stem_info.category_id==Stem->category_of_currentmatch && stem_info.raw_data==Stem->raw_data_of_currentmatch)
#else
						if (stem_info.category_id==Stem->category_of_currentmatch)
#endif
						{
							if (stem_info.description=="son")
							{
								nmc=true;
								return false;
							}
							else if (stem_info.description=="said" || stem_info.description=="say" || stem_info.description=="notify/communicate" || stem_info.description.split(QRegExp("[ /]")).contains("listen") || stem_info.description.contains("from/about")||stem_info.description.contains("narrate"))
							{
								nrc=true;
								return false;
							}
							for (unsigned int i=0;i<stem_info.abstract_categories.size();i++)
									if (stem_info.abstract_categories[i] && get_abstractCategory_id(i)>=0)
									{
										if (getColumn("category","name",get_abstractCategory_id(i))=="Male Names") //Name of Person
										{
											//out<<"abcat:"<<	getColumn("category","name",get_abstractCategory_id(i))<<"\n";
											name=true;
											return false;
										}
										else if (getColumn("category","name",get_abstractCategory_id(i))=="POSSESSIVE")
										{
											possessive=true;
											// out<<"\nI am possesive\n";
											if (place)
											{
												nmc=true;
												return false;
											}
										}
										else if (getColumn("category","name",get_abstractCategory_id(i))=="Name of Place")
										{
											place=true;
											if (possessive)
											{
												nmc=true;
												return false;
											}
										}
									}
						}
				}
				return true;
	}

};

inline wordType result(wordType t){display(t); return t;}
wordType getWordType(QString word,bool & isBinOrPossessive)
{
	display(word+":");
	isBinOrPossessive=false;
	mystemmer s(word);
	s();
#ifdef TENTATIVE //needed in order not consider 'alayhi_alsalam' as an additional NMC which results in it not being counted in case it is the last allowable count of NMC...
	if (word==alayhi_alsalam)
		return result(NRC);
#endif
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

bool getNextState(stateType currentState,wordType currentType,stateType & nextState,stateData & currentData,long long  index,bool isBinOrPossessive,chainData *currentChain)
{
		display(QString(" nmcsize: %1 ").arg(currentData.nmcCount));
		display(QString(" nrcsize: %1 ").arg(currentData.nrcCount));
		display(currentState);

		switch(currentState)
		{
		case TEXT_S:
			initializeStateData(currentData);
			if(currentType==NAME)
			{
				nextState=NAME_S;
				currentData.sanadStartIndex=index;
				//currentData.started=true;
				currentData.narratorStartIndex=index;
                                //currentData.nameStartIndex=index;
                                currentChain->namePrim=new NamePrim(index);

			}
			else if (currentType==NRC)
			{
				currentData.sanadStartIndex=index;
                                currentData.nrcStartIndex=index;
                                currentChain->narratorConnectorPrim=new NarratorConnectorPrim(index);
				nextState=NRC_S;
			}
#ifdef TENTATIVE//needed in case a hadith starts by ibn such as "ibn yousef qal..."
			else if (currentType==NMC && isBinOrPossessive)
			{
				currentData.narratorStartIndex=index;
				currentData.sanadStartIndex=index;
                                currentData.nmcStartIndex=index;
                                currentChain->nameConnectorPrim=new NameConnectorPrim(index);
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
                                currentData.nmcStartIndex=index;

                                currentChain->namePrim->m_end=index-1;
                                currentChain->narrator->m_narrator.append(currentChain->namePrim);

                                currentChain->nameConnectorPrim=new NameConnectorPrim(index);

			}
			else if (currentType==NRC)
			{
				nextState=NRC_S;
				currentData.narratorCount++;
				display(QString("counter%1\n").arg(currentData.narratorCount));
				currentData.nrcCount=1;
				currentData.narratorEndIndex=index-1;
				currentData.nrcStartIndex=index;

                                currentChain->namePrim->m_end=index-1;
                                currentChain->narrator->m_narrator.append(currentChain->namePrim);

                                currentChain->narratorConnectorPrim=new NarratorConnectorPrim(index);
                                currentChain->chain->m_chain.append(currentChain->narrator);
			}
			else
			{
				nextState=NAME_S;
			}
			return true;

		case NMC_S:
			if (currentData.nmcCount>currentData.nmcThreshold)
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
                                        currentChain->nameConnectorPrim->m_end=index-1;//later check for out of bounds
                                        currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);//check to see if we should also add the narrator to chain
					return FALSE;
				}
				//currentData.narratorEndIndex=index-1; check this case

			}
			else if (currentType==NRC)
			{
				currentData.narratorCount++;
				display(QString("counter%1\n").arg(currentData.narratorCount));
				currentData.nrcCount=1;
				nextState=NRC_S;

				currentData.narratorEndIndex=index-1;
				currentData.nrcStartIndex=index;

                                currentChain->nameConnectorPrim->m_end=index-1;
                                currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);

                                currentChain->narratorConnectorPrim=new NarratorConnectorPrim(index);
                                currentChain->chain->m_chain.append(currentChain->narrator);
			}
			else if(currentType==NAME)
			{
				nextState=NAME_S;

                                currentChain->nameConnectorPrim->m_end=index-1;
                                currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);

                                currentChain->namePrim=new NamePrim(index);

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
			if (currentData.nrcCount>=currentData.nrcThreshold)
			{
				nextState=TEXT_S;
				currentData.nrcEndIndex=index-1;

                                currentChain->narratorConnectorPrim->m_end=index-1;

                                currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);                                
				return false;
			}
			else if (currentType==NAME)
			{
				nextState=NAME_S;
                                //currentData.nameStartIndex=index;
                                currentChain->namePrim->m_start=index;

                                currentChain->narratorConnectorPrim->m_end=index-1;
                                currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);

                                currentChain->namePrim=new NamePrim(index);


				currentData.nrcEndIndex=index-1;
			}
#ifdef TENTATIVE ////needed in case 2 3an's appear after each other intervened by a name which is unknown
			else if (currentType==NRC)
			{
                                if(currentData.nrcCount>=1) //in order not to have two NRCs without a name suspect between them
                            {
				currentData.narratorCount++;
				display(QString("counter%1\n").arg(currentData.narratorCount));
				currentData.nrcCount=1;
				nextState=NRC_S;
				currentData.narratorEndIndex=index-1;
				currentData.nrcStartIndex=index;                               

                                currentChain->narratorConnectorPrim->m_end=index-1;
                                currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);

                                currentChain->narratorConnectorPrim=new NarratorConnectorPrim(index);
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

int hadith(QString input_str)
{

         QFile chainOutput("test-cases/chainOutput");
        if (!chainOutput.open(QIODevice::ReadWrite))
                return 1;

        QTextStream chainOut(&chainOutput);
        chainOut.setCodec("utf-8");



	QFile input(input_str.split("\n")[0]);

	if (!input.open(QIODevice::ReadWrite))
	{
		out << "File not found\n";
		return 1;
	}
	QTextStream file(&input);
	file.setCodec("utf-8");

	QString wholeFile=file.readAll();
	if (wholeFile.isNull())
	{
		out<<"file error:"<<input.errorString()<<"\n";
	}
	if (wholeFile.isEmpty()) //ignore empty files
	{
		out<<"empty file\n";
		return 0;
	}
	QStringList wordList;
	parse(wholeFile,wordList);

#if 0 //just to test parsing
        for (long long  i=0;i<wordList.size();i++)
		out<<wordList[i]<<"_";
	return 0;
#endif
        long long listSize=wordList.size();
	stateType currentState=TEXT_S;
	stateType nextState=TEXT_S;
	wordType currentType;
        long long  newHadithStart=-1;

	stateData currentData;
	initializeStateData(currentData);

        chainData *currentChain=new chainData();
        initializeChainData(currentChain);

        long long i=0;
        long long  sanadEnd;

	bool isBinOrPossessive=false;
	while (i<listSize)
	{
		currentType=getWordType(wordList[i],isBinOrPossessive);

                if((getNextState(currentState,currentType,nextState,currentData,i,isBinOrPossessive,currentChain)==FALSE)&& currentData.narratorCount>=currentData.narratorThreshold)
		{
				 sanadEnd=currentData.narratorEndIndex;
                                 newHadithStart=currentData.sanadStartIndex;
				 out<<"new hadith start: "<<wordList[newHadithStart]<<" "<<(newHadithStart+1<wordList.size()?wordList[newHadithStart+1]:"")<<" "<<(newHadithStart+2<wordList.size()?wordList[newHadithStart+2]:"")<<" "<<(newHadithStart+3<wordList.size()?wordList[newHadithStart+3]:"")<<endl;
				 out<<"sanad end: "<<wordList[sanadEnd-2]<<" "<<wordList[sanadEnd-1]<<" "<<wordList[sanadEnd]<<endl<<endl; //maybe i+-1
                                 currentChain->chain->serialize();
                               // chainOut<<"Hamza";

		}
		currentState=nextState;
		i++;
	}

	if (newHadithStart<0)
	{
		out<<"no hadith found\n";
		return 0;
	}

	return 0;
}

#endif

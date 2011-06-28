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
inline QString type_to_text(wordType t) {
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
inline QString type_to_text(stateType t) {
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
inline void display(wordType t) {
	out<<type_to_text(t)<<" ";
	//qDebug() <<type_to_text(t)<<" ";
}
inline void display(stateType t) {
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

typedef struct chainDataBiography_ {
	NamePrim *namePrim;
	NameConnectorPrim *nameConnectorPrim;
	NarratorConnectorPrim *narratorConnectorPrim;
	TempConnectorPrimList * temp_nameConnectors;
	Narrator *narrator;
	Chain *chain;

	void initialize(QString * text) {
		delete namePrim;
		delete nameConnectorPrim;
		delete narratorConnectorPrim;
		delete narrator;
		delete chain;
		/*for (int i=0;i<temp_nameConnectors->count()-1;i++)
			delete (*temp_nameConnectors)[i];*/
		delete temp_nameConnectors;
		namePrim=new NamePrim(text);
		nameConnectorPrim=new NameConnectorPrim(text);
		narratorConnectorPrim=new NarratorConnectorPrim(text) ;
		narrator=new Narrator (text);
		chain=new Chain(text);
		temp_nameConnectors=new TempConnectorPrimList();
	}
} chainDataBiography;
typedef struct stateDataBiography_ {
	long  sanadStartIndex, narratorCount,narratorStartIndex,narratorEndIndex,nrcStartIndex,nrcEndIndex;
	long  nmcCount, nrcCount,nameStartIndex,nmcStartIndex;
	bool nmcValid;
	bool ibn_or_3abid;
	bool nrcPunctuation;

	void initialize() {
		nmcCount=0;
		narratorCount=0;
		nrcCount=0;
		narratorStartIndex=0;
		narratorEndIndex=0;
		nrcStartIndex=0;
		nrcEndIndex=0;
		nmcValid=false;
		ibn_or_3abid=false;
		nameStartIndex=0;
		nmcStartIndex=0;
	#ifdef PUNCTUATION
		nrcPunctuation=false;
	#endif
	}

} stateDataBiography;

class HadithSegmentor {
private:
	stateDataBiography currentData;
	QString * text;
	long current_pos;

#ifdef STATS
	QVector<map_entry*> temp_nrc_s, temp_nmc_s;
	int temp_nrc_count,temp_nmc_count;
	statistics stat;
	int temp_names_per_narrator;
	QString current_exact,current_stem;
#endif

	bool getNextState(StateInfo &  stateInfo,chainDataBiography *currentChain) {
		display(QString(" nmcsize: %1 ").arg(currentData.nmcCount));
		display(QString(" nrcsize: %1 ").arg(currentData.nrcCount));
		display(stateInfo.currentState);
	#ifdef PUNCTUATION
		if (stateInfo.currentPunctuationInfo.has_punctuation) {
			display("<has punctuation>");
			if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
				display("<ending Punctuation>");
			}
		}
		if (stateInfo.number) {
			stateInfo.currentPunctuationInfo.has_punctuation=true;
			stateInfo.currentPunctuationInfo.fullstop=true;
			stateInfo.currentPunctuationInfo.newLine=true;
		}
	#endif
	#ifdef TRYTOLEARN
		stateInfo.nrcIsPunctuation=false;
	#endif
	#ifdef REFINEMENTS
		bool should_stop= (stateInfo.currentType== STOP_WORD && !stateInfo.ibnOr3abid());//stop_word not preceeded by 3abid or ibn
	#endif
		bool return_value=true;
		switch(stateInfo.currentState)
		{
		case TEXT_S:
			if(stateInfo.currentType==NAME) {
			#ifdef PUNCTUATION
				if (!stateInfo.previousPunctuationInfo.fullstop) {
					stateInfo.nextState=TEXT_S;
					break;
				}
			#endif

				currentData.initialize();
				stateInfo.nextState=NAME_S;
				currentData.sanadStartIndex=stateInfo.startPos;
				currentData.narratorStartIndex=stateInfo.startPos;
			#ifdef CHAIN_BUILDING
				currentChain->initialize(text);
				display(QString("\ninit%1\n").arg(currentChain->narrator->m_narrator.size()));
				currentChain->namePrim->m_start=stateInfo.startPos; //changed this now
				currentChain->namePrim->m_end=stateInfo.endPos;
			#endif
			#ifdef STATS
				temp_names_per_narrator=1;
			#endif
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.has_punctuation) {
					display("<punc1>");
					currentData.narratorCount++;
					stateInfo.nextState=NRC_S;
					currentData.nrcCount=0;//punctuation is zero
					currentData.narratorEndIndex=stateInfo.endPos;
					currentData.nrcStartIndex=stateInfo.nextPos;//next_positon(stateInfo.endPos,stateInfo.followedByPunctuation);
				#ifdef TRYTOLEARN
					stateInfo.nrcIsPunctuation=true;
				#endif
				#ifdef CHAIN_BUILDING
					currentChain->namePrim->m_end=stateInfo.endPos;
					currentChain->narrator->m_narrator.append(currentChain->namePrim);
					currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,currentData.nrcStartIndex);
					assert(currentChain->narrator->m_narrator.size()>0);
					currentChain->chain->m_chain.append(currentChain->narrator);
					currentChain->narrator=new Narrator(text);
				#endif
					if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
						stateInfo.nextState=TEXT_S;
						currentData.narratorEndIndex=stateInfo.endPos;
						currentData.narratorCount++;
						return_value=false;
					}
				}
			#endif
			}
			else if (stateInfo.currentType==NRC) {
			/*#ifdef PUNCTUATION
				if (!stateInfo.previousPunctuationInfo.fullstop && stateInfo._3an) {
					stateInfo.nextState=TEXT_S;
					break;
				}
			#endif*/
				currentData.initialize();
				currentData.sanadStartIndex=stateInfo.startPos;
				currentData.nrcStartIndex=stateInfo.startPos;
				stateInfo.nextState=NRC_S;
				currentData.nrcCount=1;
			#ifdef CHAIN_BUILDING
				currentChain->initialize(text);
				display(QString("\ninit%1\n").arg(currentChain->narrator->m_narrator.size()));
				currentChain->narratorConnectorPrim->m_start=stateInfo.startPos;
				currentChain->narratorConnectorPrim->m_end=stateInfo.endPos;
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
			#ifdef REFINEMENTS
				if (stateInfo._3an) {
					currentData.nrcCount=1;
					currentData.nrcEndIndex=stateInfo.endPos;
				#ifdef CHAIN_BUILDING
					currentChain->narratorConnectorPrim->m_end=stateInfo.endPos;
					currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);
					currentChain->namePrim=new NamePrim(text,stateInfo.nextPos);
					currentChain->narrator=new Narrator(text);
				#endif
					stateInfo.nextState=NAME_S;
				}
			#endif
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
					currentData.narratorEndIndex=stateInfo.endPos;
					stateInfo.nextState=TEXT_S;
					return_value=false;
				}
			#endif
			}
	#ifdef IBN_START//needed in case a hadith starts by ibn such as "ibn yousef qal..."
			else if (stateInfo.currentType==NMC && stateInfo.familyNMC) {
			#ifdef PUNCTUATION
				if (!stateInfo.previousPunctuationInfo.fullstop) {
					stateInfo.nextState=TEXT_S;
					break;
				}
			#endif
				display("<IBN1>");
				currentData.initialize();
				currentData.sanadStartIndex=stateInfo.startPos;
				currentData.nmcStartIndex=stateInfo.startPos;
				currentData.narratorStartIndex=stateInfo.startPos;
				currentData.nmcCount=1;
				stateInfo.nextState=NMC_S;
				currentData.nmcValid=true;
			#ifdef CHAIN_BUILDING
				currentChain->initialize(text);
				display(QString("\ninit%1\n").arg(currentChain->narrator->m_narrator.size()));
				currentChain->nameConnectorPrim->m_start=stateInfo.startPos;
				currentChain->nameConnectorPrim->m_end=stateInfo.endPos;
				currentChain->nameConnectorPrim->setFamilyConnector();
				if (stateInfo.ibn)
					currentChain->nameConnectorPrim->setIbn();
				currentChain->namePrim->m_start=stateInfo.nextPos;//added this now
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
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
					stateInfo.nextState=TEXT_S;
				#ifdef CHAIN_BUILDING
					currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);
					assert(currentChain->narrator->m_narrator.size()>0);
					currentChain->chain->m_chain.append(currentChain->narrator);
				#endif
					currentData.narratorCount++;
					currentData.narratorEndIndex=stateInfo.endPos;
					return_value=false;
				}
			#endif
			}
	#endif
			else {
				stateInfo.nextState=TEXT_S;
			}
			break;

		case NAME_S:
		#ifdef REFINEMENTS
			if(should_stop)
			{
				display("<STOP1>");
				stateInfo.nextState=STOP_WORD_S;
				currentData.narratorCount++;
			#ifdef CHAIN_BUILDING
				currentChain->namePrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
				currentChain->narrator->m_narrator.append(currentChain->namePrim);
				delete currentChain->temp_nameConnectors;
				currentChain->temp_nameConnectors= new TempConnectorPrimList();

				currentChain->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos); //we added this to previous name bc assumed this will only happen if it is muhamad and "sal3am"
				currentChain->nameConnectorPrim->m_end=stateInfo.endPos;
				currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);
				assert(currentChain->narrator->m_narrator.size()>0);
				currentChain->narrator->isRasoul=true;
				currentChain->chain->m_chain.append(currentChain->narrator);
			#endif
				currentData.narratorEndIndex=stateInfo.endPos;
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
			if(stateInfo.currentType==NMC) {
				stateInfo.nextState=NMC_S;
				currentData.nmcValid=stateInfo.isFamilyConnectorOrPossessivePlace();
				currentData.nmcCount=1;
				currentData.nmcStartIndex=stateInfo.startPos;
			#ifdef CHAIN_BUILDING
				currentChain->namePrim->m_end=stateInfo.lastEndPos;
				currentChain->narrator->m_narrator.append(currentChain->namePrim);
				currentChain->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
				currentChain->nameConnectorPrim->m_end=stateInfo.endPos;
			#ifdef REFINEMENTS
				if (stateInfo.familyNMC) {
					currentChain->nameConnectorPrim->setFamilyConnector();
					if (stateInfo.ibn)
						currentChain->nameConnectorPrim->setIbn();
				} else if (stateInfo.possessivePlace)
					currentChain->nameConnectorPrim->setPossessive();
			#endif
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
				if (stateInfo.currentPunctuationInfo.has_punctuation) {
					currentData.nmcCount=hadithParameters.nmc_max+1;
					if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
						stateInfo.nextState=TEXT_S;
					#ifdef CHAIN_BUILDING
						if (currentChain->chain->m_chain.size()>0) //not needed check
							currentChain->chain->m_chain.append(currentChain->narrator);
					#endif
						currentData.narratorEndIndex=stateInfo.endPos;
						currentData.narratorCount++;
						return_value=false;
					}
				}
			#endif
			}
			else if (stateInfo.currentType==NRC) {
				stateInfo.nextState=NRC_S;
			#ifdef GET_WAW
				if (!stateInfo.isWaw) //so as not to affect the count for tolerance and lead to false positives, just used for accuracy
			#endif
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
				currentData.narratorEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
				currentData.nrcStartIndex=stateInfo.startPos;
			#ifdef CHAIN_BUILDING
				currentChain->namePrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
				currentChain->narrator->m_narrator.append(currentChain->namePrim);
				currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,stateInfo.startPos);
				currentChain->narratorConnectorPrim->m_end=stateInfo.endPos;
				assert(currentChain->narrator->m_narrator.size()>0);
				currentChain->chain->m_chain.append(currentChain->narrator);
				currentChain->narrator=new Narrator(text);
			#endif
			#ifdef REFINEMENTS
				if (stateInfo._3an) {
					currentData.nrcCount=1;
					currentData.nrcEndIndex=stateInfo.endPos;
				#ifdef CHAIN_BUILDING
					currentChain->narratorConnectorPrim->m_end=stateInfo.endPos;
					currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);
					currentChain->namePrim=new NamePrim(text,stateInfo.nextPos);
				#endif
					stateInfo.nextState=NAME_S;
				}
			#endif
			#ifdef PUNCTUATION
			#if 0
				if (stateInfo.punctuationInfo.has_punctuation)
					currentData.nrcCount=parameters.nrc_max;
			#endif
				if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
					currentData.narratorEndIndex=stateInfo.endPos;
					stateInfo.nextState=TEXT_S;
					return_value=false;
				}
			#endif
			}
			else {
			#ifdef STATS
				if (currentType==NAME) {
					temp_names_per_narrator++;//found another name name
				}
			#endif
				stateInfo.nextState=NAME_S;
				currentChain->namePrim->m_end=stateInfo.endPos;
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.has_punctuation) {
					display("<punc2>");
					currentData.narratorCount++;
					stateInfo.nextState=NRC_S;
					currentData.nrcCount=0; //punctuation not counted
					currentData.narratorEndIndex=stateInfo.endPos;
					currentData.nrcStartIndex=stateInfo.nextPos;//next_positon(stateInfo.endPos,stateInfo.followedByPunctuation);
				#ifdef CHAIN_BUILDING
					currentChain->namePrim->m_end=stateInfo.endPos;
					currentChain->narrator->m_narrator.append(currentChain->namePrim);
					currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,currentData.nrcStartIndex);
					assert(currentChain->narrator->m_narrator.size()>0);
					currentChain->chain->m_chain.append(currentChain->narrator);
					currentChain->narrator=new Narrator(text);
				#endif
					if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
						currentData.narratorEndIndex=stateInfo.endPos;
						stateInfo.nextState=TEXT_S;
						return_value=false;
					}
				}
			#endif
			}
			break;

		case NMC_S:
		#ifdef REFINEMENTS
			if(should_stop) {
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
				currentData.nrcCount=0;
				stateInfo.nextState=NRC_S;

				currentData.narratorEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(currentData.nmcStartIndex);
				currentData.nrcStartIndex=currentData.nmcStartIndex;
				currentData.nrcEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);

			#ifdef CHAIN_BUILDING
				for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
					currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));
				if (currentChain->chain->m_chain.size()>0)
					currentChain->chain->m_chain.append(currentChain->narrator);
				display(currentChain->narrator->getString()+"\n");
				currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,currentData.nrcStartIndex);
				currentChain->narratorConnectorPrim->m_end=currentData.nrcEndIndex;
				currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);
				display(currentChain->narratorConnectorPrim->getString()+"\n");
			#endif
			//2-create a new narrator of just this stop word as name connector
			#ifdef CHAIN_BUILDING
				currentChain->nameConnectorPrim->m_start=stateInfo.startPos;
				currentChain->nameConnectorPrim->m_end=stateInfo.endPos;
				currentChain->narrator=new Narrator(text);
				currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);
				currentChain->narrator->isRasoul=true;
				assert(currentChain->narrator->m_narrator.size()>0);
				currentChain->chain->m_chain.append(currentChain->narrator);
			#endif

				currentData.narratorEndIndex=stateInfo.endPos;
				stateInfo.nextState=STOP_WORD_S;
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
			if (stateInfo.currentType==NRC) {
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
				stateInfo.nextState=NRC_S;

				currentData.narratorEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
				currentData.nrcStartIndex=stateInfo.startPos;
			#ifdef CHAIN_BUILDING
				currentChain->nameConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
				currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
				for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
					currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));
				currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,stateInfo.startPos);
				assert(currentChain->narrator->m_narrator.size()>0);
				currentChain->chain->m_chain.append(currentChain->narrator);
				currentChain->narrator=new Narrator(text);
			#endif
			#ifdef REFINEMENTS
				if (stateInfo._3an) {
					currentData.nrcCount=1;
					currentData.nrcEndIndex=stateInfo.endPos;
				#ifdef CHAIN_BUILDING
					currentChain->narratorConnectorPrim->m_end=stateInfo.endPos;
					currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);
					currentChain->namePrim=new NamePrim(text,stateInfo.nextPos);
				#endif
					stateInfo.nextState=NAME_S;
				}
			#endif
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
					currentData.narratorEndIndex=stateInfo.endPos;
					stateInfo.nextState=TEXT_S;
					return_value=false;
				}
			#endif
			}
			else if(stateInfo.currentType==NAME) {
				currentData.nmcCount=0;
				stateInfo.nextState=NAME_S;
			#ifdef CHAIN_BUILDING
				currentChain->nameConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
				currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
				for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
					currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));
				currentChain->namePrim=new NamePrim(text,stateInfo.startPos);
				currentChain->namePrim->m_end=stateInfo.endPos;
			#endif
			#ifdef STATS
				temp_names_per_narrator++;//found another name name
			#endif
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.has_punctuation) {
					display("<punc3>");
					currentData.narratorCount++;
					stateInfo.nextState=NRC_S;
					currentData.nrcCount=0;
					currentData.narratorEndIndex=stateInfo.endPos;
					currentData.nrcStartIndex=stateInfo.nextPos;//next_positon(stateInfo.endPos,stateInfo.followedByPunctuation);
				/*#ifdef TRYTOLEARN
					stateInfo.nrcIsPunctuation=true;
				#endif*/
				#ifdef CHAIN_BUILDING
					currentChain->namePrim->m_end=stateInfo.endPos;
					currentChain->narrator->m_narrator.append(currentChain->namePrim);
					currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,currentData.nrcStartIndex);
					assert(currentChain->narrator->m_narrator.size()>0);
					currentChain->chain->m_chain.append(currentChain->narrator);
					currentChain->narrator=new Narrator(text);
				#endif
					if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
						currentData.narratorEndIndex=stateInfo.endPos;
						stateInfo.nextState=TEXT_S;
						return_value=false;
						break;
					}
				}
			#endif
			}
		#ifdef PUNCTUATION
			else if (stateInfo.currentPunctuationInfo.has_punctuation) { //TODO: if punctuation check is all what is required
				stateInfo.nextState=NMC_S;
				currentData.nmcCount=hadithParameters.nmc_max+1;
				currentData.nmcValid=false;
				if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
					stateInfo.nextState=TEXT_S;
				#ifdef CHAIN_BUILDING
					/*currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
					for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
						currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));*/
					if (currentChain->narrator->m_narrator.size()>0)
						currentChain->chain->m_chain.append(currentChain->narrator);
					display("{this}");
				#endif
					currentData.narratorEndIndex=(currentChain->narrator->m_narrator.size()!=0?
												  currentChain->narrator->getEnd():
												  (currentChain->chain->m_chain.size()==0?
												   stateInfo.lastEndPos:
												   currentChain->chain->m_chain.last()->getEnd()));
					currentData.narratorCount++;
					return_value=false;
					break;
				}
			}
		#endif
			else if (currentData.nmcCount>hadithParameters.nmc_max
					#ifdef PUNCTUATION
						 || stateInfo.number ||
						 (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine)
					#endif
				) {
				if (currentData.nmcCount>hadithParameters.nmc_max && currentData.nmcValid) {//number is severe condition no tolerance
					currentData.nmcValid=false;
					stateInfo.nextState=NMC_S;
					currentData.nmcCount=0;
				} else {
					stateInfo.nextState=TEXT_S;
				#ifdef CHAIN_BUILDING
					/*currentChain->nameConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(start_index);//later check for out of bounds
					currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
					for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
						currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));//check to see if we should also add the narrator to chain
					*/
					if (currentChain->narrator->m_narrator.size()>0)
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

					display("{check}");
					currentData.narratorEndIndex=(currentChain->narrator->m_narrator.size()!=0?
												  currentChain->narrator->getEnd():
												  (currentChain->chain->m_chain.size()==0?
												   stateInfo.lastEndPos:
												   currentChain->chain->m_chain.last()->getEnd()));
					return_value= false;
					break;
				}
				//currentData.narratorEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(start_index); check this case

			}
			else if (stateInfo.currentType==NMC) {
			#ifdef CHAIN_BUILDING
				currentChain->nameConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
				currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
				currentChain->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
				currentChain->nameConnectorPrim->m_end=stateInfo.endPos;
			#ifdef REFINEMENTS
				if (stateInfo.familyNMC) {
					currentChain->nameConnectorPrim->setFamilyConnector();
					if (stateInfo.ibn)
						currentChain->nameConnectorPrim->setIbn();
				}else if (stateInfo.possessivePlace)
					currentChain->nameConnectorPrim->setPossessive();
			#endif
			#endif
				currentData.nmcCount++;
				if (stateInfo.isFamilyConnectorOrPossessivePlace())
					currentData.nmcValid=true;
				stateInfo.nextState=NMC_S;
			#ifdef STATS
				map_entry * entry=new map_entry;
				entry->exact=current_exact;
				entry->stem=current_stem;
				entry->frequency=1;
				temp_nmc_s.append(entry);
				temp_nmc_count++;
			#endif
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
					stateInfo.nextState=TEXT_S;
				#ifdef CHAIN_BUILDING
					currentChain->nameConnectorPrim->m_end=stateInfo.endPos;
					currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
					for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
						currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));
					assert(currentChain->narrator->m_narrator.size()>0);
					currentChain->chain->m_chain.append(currentChain->narrator);
				#endif
					currentData.narratorEndIndex=stateInfo.endPos;
					currentData.narratorCount++;
					return_value=false;
					break;
				}
			#endif
			}
			else {
				stateInfo.nextState=NMC_S; //maybe modify later
				currentChain->nameConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
				currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
				currentChain->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
				currentChain->nameConnectorPrim->m_end=stateInfo.endPos;
			#ifdef STATS
				map_entry * entry=new map_entry;
				entry->exact=current_exact;
				entry->stem=current_stem;
				entry->frequency=1;
				temp_nmc_s.append(entry);
				temp_nmc_count++;
			#endif
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
					stateInfo.nextState=TEXT_S;
				#ifdef CHAIN_BUILDING
					currentChain->nameConnectorPrim->m_end=stateInfo.endPos;
					currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
					for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
						currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));
					assert(currentChain->narrator->m_narrator.size()>0);
					currentChain->chain->m_chain.append(currentChain->narrator);
				#endif
					currentData.narratorEndIndex=stateInfo.endPos;
					currentData.narratorCount++;
					return_value=false;
					break;
				}
			#endif
			}
			break;

		case NRC_S:
		#ifdef REFINEMENTS
			if(should_stop) {
				display("<STOP3>");
			#ifdef CHAIN_BUILDING
				//1-first add thus stop word as NMC
				currentChain->narratorConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
				currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);
				if (currentChain->narrator!=NULL)
					delete currentChain->narrator;
				currentChain->narrator=new Narrator(text);
				currentChain->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
				currentChain->nameConnectorPrim->m_end=stateInfo.endPos;
				currentChain->temp_nameConnectors->clear();
				//currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);
			#endif
				currentData.nrcEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
				currentData.nmcCount=1;
			#ifdef STATS
				map_entry * entry=new map_entry;
				entry->exact=current_exact;
				entry->stem=current_stem;
				entry->frequency=1;
				temp_nmc_s.append(entry);
				temp_nmc_count++;
			#endif
				currentData.narratorStartIndex=stateInfo.startPos;
				currentData.nmcStartIndex=stateInfo.startPos;

				//2-add this narrator and end lookup
				stateInfo.nextState=STOP_WORD_S;
				currentData.narratorCount++;
			#ifdef CHAIN_BUILDING
				currentChain->nameConnectorPrim->m_end=stateInfo.endPos;
				currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);
				assert(currentChain->narrator->m_narrator.size()>0);
				currentChain->narrator->isRasoul=true;
				currentChain->chain->m_chain.append(currentChain->narrator);
			#endif
				currentData.narratorEndIndex=stateInfo.endPos;
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
			if(stateInfo.currentType==NAME || stateInfo.currentType ==NRC)
				currentData.nrcPunctuation=false;
		#endif
		#ifdef REFINEMENTS
			if (stateInfo.currentType==NAME || stateInfo.possessivePlace) {
		#else
			if (stateInfo.currentType==NAME) {
		#endif
				stateInfo.nextState=NAME_S;
				currentData.nrcCount=1;
				//currentData.nameStartIndex=start_index;
			#ifdef CHAIN_BUILDING
				//currentChain->namePrim->m_start=start_index;

				currentChain->narratorConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
				currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);
			#ifdef REFINEMENTS
				if (stateInfo.currentType==NAME){
			#endif
					currentChain->namePrim=new NamePrim(text,stateInfo.startPos);
					currentChain->namePrim->m_end=stateInfo.endPos;
			#ifdef REFINEMENTS
					currentChain->namePrim->learnedName=stateInfo.learnedName;
				} else {
					currentChain->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
					currentChain->nameConnectorPrim->m_end=stateInfo.endPos;
					currentChain->nameConnectorPrim->setPossessive();
					currentChain->namePrim=new NamePrim(text,stateInfo.startPos);//added this now
					currentData.narratorStartIndex=stateInfo.startPos;
					currentData.nmcStartIndex=stateInfo.startPos;
				}
			#endif
			#endif
				currentData.nrcEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
			#ifdef STATS
				temp_names_per_narrator++;//found another name name
			#endif
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.has_punctuation)
				{
					display("<punc4>");
					currentData.narratorCount++;
					stateInfo.nextState=NRC_S;
					currentData.nrcCount=0;
					currentData.narratorEndIndex=stateInfo.endPos;
					currentData.nrcStartIndex=stateInfo.nextPos;//next_positon(stateInfo.endPos,stateInfo.followedByPunctuation);
				/*#ifdef TRYTOLEARN
					stateInfo.nrcIsPunctuation=true;
				#endif*/
				#ifdef CHAIN_BUILDING
				#ifdef REFINEMENTS
					if (stateInfo.currentType==NAME)
					{
				#endif
						currentChain->namePrim->m_end=stateInfo.endPos;
						currentChain->narrator->m_narrator.append(currentChain->namePrim);
				#ifdef REFINEMENTS
					}
					else
					{
						currentChain->nameConnectorPrim->m_end=stateInfo.endPos;
						currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);
					}
				#endif

					currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,currentData.nrcStartIndex);
					if (currentChain->narrator->m_narrator.size()>0)
						currentChain->chain->m_chain.append(currentChain->narrator);
					currentChain->narrator=new Narrator(text);
				#endif
					if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
						currentData.narratorEndIndex=stateInfo.endPos;
						stateInfo.nextState=TEXT_S;
						return_value=false;
						break;
					}
					break;
				}
			#endif
			}
		#ifdef PUNCTUATION
			else if (currentData.nrcCount>=hadithParameters.nrc_max || currentData.nrcPunctuation ||stateInfo.number) { //if not in refinements mode stateInfo.number will always remain false
		#else
			else if (currentData.nrcCount>=hadithParameters.nrc_max) {
		#endif
				stateInfo.nextState=TEXT_S;
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
				//currentData.nrcEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(start_index);
				//currentChain->narratorConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(start_index);
				//currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);

				//currentData.narratorEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(start_index); //check this case
				return_value= false;
				break;
			}
		#ifdef IBN_START
			else if (stateInfo.currentType==NMC && stateInfo.familyNMC) {
				display("<IBN3>");
			#ifdef CHAIN_BUILDING
				/*currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
				for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
					currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));*/
				currentChain->narratorConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
				currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);
				if (currentChain->narrator!=NULL)
					delete currentChain->narrator;
				currentChain->narrator=new Narrator(text);
				currentChain->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
				currentChain->nameConnectorPrim->m_end=stateInfo.endPos;
				currentChain->nameConnectorPrim->setIbn();
				currentChain->temp_nameConnectors->clear();
				//currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);
			#endif
				currentData.nrcEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
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
				currentData.narratorStartIndex=stateInfo.startPos;
				currentData.nmcStartIndex=stateInfo.startPos;
				stateInfo.nextState=NMC_S;
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.has_punctuation)
				{
					display("<punc5>");
					currentData.narratorCount++;
					stateInfo.nextState=NRC_S;
					currentData.nrcCount=0;
					currentData.narratorEndIndex=stateInfo.endPos;
					currentData.nrcStartIndex=stateInfo.nextPos;//next_positon(stateInfo.endPos,stateInfo.followedByPunctuation);
				#ifdef TRYTOLEARN
					stateInfo.nrcIsPunctuation=true;
				#endif
				#ifdef CHAIN_BUILDING
					currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);
					currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,currentData.nrcStartIndex);
					assert(currentChain->narrator->m_narrator.size()>0);
					currentChain->chain->m_chain.append(currentChain->narrator);
					currentChain->narrator=new Narrator(text);
				#endif
					if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
						currentData.narratorEndIndex=stateInfo.endPos;
						stateInfo.nextState=TEXT_S;
						return_value=false;
						break;
					}
					break;
				}
			#endif
			}
		#endif
			else {
				stateInfo.nextState=NRC_S;
				currentData.nrcCount++;
			#ifdef STATS
				map_entry * entry=new map_entry;
				entry->exact=current_exact;
				entry->stem=current_stem;
				entry->frequency=1;
				temp_nrc_s.append(entry);
				temp_nrc_count++;
			#endif
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
					stateInfo.nextState=TEXT_S;
				#ifdef CHAIN_BUILDING
					currentChain->narratorConnectorPrim->m_end=stateInfo.lastEndPos;
					currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);
				#endif
					currentData.narratorEndIndex=stateInfo.endPos;
					return_value=false;
					break;
				}
			#endif
			#ifdef REFINEMENTS
				if (stateInfo._3an) {
					currentData.nrcCount=1;
					currentData.nrcEndIndex=stateInfo.endPos;
				#ifdef CHAIN_BUILDING
					currentChain->narratorConnectorPrim->m_end=stateInfo.endPos;
					currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);
					currentChain->namePrim=new NamePrim(text,stateInfo.nextPos);
					currentChain->narrator=new Narrator(text);
				#endif
					stateInfo.nextState=NAME_S;
					break;
				}
			#endif
			}
		#ifdef PUNCTUATION
			if (stateInfo.currentPunctuationInfo.has_punctuation && stateInfo.nextState==NRC_S && stateInfo.currentType!=NAME && stateInfo.currentType!=NRC)
				currentData.nrcPunctuation=true;
		#endif
			break;
	#ifdef REFINEMENTS
		case STOP_WORD_S:
			if (stateInfo.currentType==STOP_WORD) {
				stateInfo.nextState=STOP_WORD_S;
				Narrator * n=((Narrator*)currentChain->chain->m_chain[currentChain->chain->m_chain.length()-1]);
				n->m_narrator.last()->m_end=stateInfo.endPos;
				display(n->getString());
				n->isRasoul=true;
				currentData.narratorEndIndex=stateInfo.endPos;
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
					stateInfo.nextState=TEXT_S;
					return_value=false;
					break;
				}
			#endif
			} else {
				stateInfo.nextState=TEXT_S;
				return_value=false;
			}
			break;
	#endif
		default:
			break;
		}
		display("\n");
	#ifdef REFINEMENTS
		currentData.ibn_or_3abid=stateInfo.ibnOr3abid(); //for it to be saved for next time use
	#endif
		return return_value;
	}
	inline bool result(wordType t, StateInfo &  stateInfo,chainDataBiography *currentChain){display(t); stateInfo.currentType=t; return getNextState(stateInfo,currentChain);}
#ifndef BUCKWALTER_INTERFACE
	bool proceedInStateMachine(StateInfo &  stateInfo,chainDataBiography *currentChain) //does not fill stateInfo.currType
	{
		stateInfo.resetCurrentWordInfo();
		long  finish;
		stateInfo.possessivePlace=false;
		stateInfo.resetCurrentWordInfo();
	#if 0
		static hadith_stemmer * s_p=NULL;
		if (s_p==NULL)
			s_p=new hadith_stemmer(text,stateInfo.startPos);
		else
			s_p->init(stateInfo.startPos);
		hadith_stemmer & s=*s_p;
	#endif
		hadith_stemmer s(text,stateInfo.startPos);
	#ifdef REFINEMENTS
	#ifdef TRYTOLEARN
		if (stateInfo.currentState==NRC_S && currentData.nrcCount<=1
			#ifdef PUNCTUATION
				&& !stateInfo.nrcIsPunctuation
			#endif
			)
			s.tryToLearnNames=true;
	#endif
	#ifdef PUNCTUATION
		if (isNumber(text,stateInfo.startPos,finish)) {
			display("Number ");
			stateInfo.number=true;
			stateInfo.endPos=finish;
			stateInfo.nextPos=next_positon(text,finish+1,stateInfo.currentPunctuationInfo);
			display(text->mid(stateInfo.startPos,finish-stateInfo.startPos+1)+":");
			return result(NMC,stateInfo,currentChain);
		}
	#endif

		QString c;
		bool found,phrase=false,stop_word=false;
		foreach (c, rasoul_words)
		{
		#if 1
			int pos;
			if (startsWith(text->midRef(stateInfo.startPos),c,pos))
			{
				stop_word=true;
				found=true;
				finish=pos+stateInfo.startPos;
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
				if (startsWith(text->midRef(stateInfo.startPos),c,pos))
				{
					phrase=true;
					found=true;
					finish=pos+stateInfo.startPos;
				#ifdef STATS
					current_stem=c;
					current_exact=c;
				#endif
					break;
				}
			#else
				found=true;
				int pos=stateInfo.startPos;
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
			if (finish==stateInfo.startPos) {
				finish=getLastLetter_IN_currentWord(text,stateInfo.startPos);
			#ifdef REFINEMENTS
				if (s.tryToLearnNames && removeDiacritics(text->mid(stateInfo.startPos,finish-stateInfo.startPos+1)).count()>=3) {
					s.name=true;
					s.finishStem=finish;
					s.startStem=stateInfo.startPos;
					s.learnedName=true;
				}
			#endif
			}
			#ifdef STATS
				current_exact=removeDiacritics(s.info.text->mid(stateInfo.startPos,finish-stateInfo.startPos+1));
				current_stem=s.stem;
				if (current_stem=="")
					current_stem=choose_stem(s.stems);
				if (current_stem=="")
					current_stem=current_exact;
			#endif
	#ifdef REFINEMENTS
		}
	#endif
		stateInfo.endPos=finish;
		stateInfo.nextPos=next_positon(text,finish,stateInfo.currentPunctuationInfo);
		display(text->mid(stateInfo.startPos,finish-stateInfo.startPos+1)+":");
	#ifdef REFINEMENTS
		if (stop_word || s.stopword) {
			return result(STOP_WORD,stateInfo,currentChain);
		}
		if (phrase)
		{
			display("PHRASE ");
			//isBinOrPossessive=true; //same behaviour as Bin
			return result(NMC,stateInfo,currentChain);
		}
		stateInfo._3abid=s._3abid;
	#endif
	#ifdef TRYTOLEARN
		if (!s.name && !s.nmc && !s.nrc &&!s.stopword) {
			QString word=s.getString().toString();
			if (stateInfo.currentPunctuationInfo.has_punctuation && s.tryToLearnNames && removeDiacritics(word).count()>=3) {
				display("{learned}");
				s.name=true;
				s.nmc=false;
				s.learnedName=true;
				s.startStem=s.info.start;
				s.finishStem=s.info.finish;
			}
		}
	#endif
		stateInfo.possessivePlace=s.possessive;
		if (s.nrc ) {
		#ifdef REFINEMENTS
			if (s.is3an) {
			#if 0
				PunctuationInfo copyPunc=stateInfo.punctuationInfo;
				display(" [An] ");
				stateInfo._3an=true;
				stateInfo.startPos=s.startStem;
				stateInfo.endPos=s.finishStem;
				stateInfo.punctuationInfo.reset();
				if (s.finishStem<finish)
					stateInfo.nextPos=s.finishStem+1;
				else
					stateInfo.punctuationInfo=copyPunc;
				if (!result(NRC,stateInfo,currentChain))
					return false;
				stateInfo.currentState=stateInfo.nextState;
				stateInfo.lastEndPos=stateInfo.endPos;
				if (s.finishStem<finish) {
					stateInfo._3an=false;
					stateInfo.startPos=s.finishStem+1;
					stateInfo.endPos=finish;
					return result(NAME,stateInfo,currentChain);
				}
				return true;
			#else
				if (s.finishStem==finish) {
					display(" [An] ");
					stateInfo._3an=true;
				}
			#endif
			}
		#endif
			return result(NRC,stateInfo,currentChain);
		}
		else if (s.nmc)
		{
			if (s.familyNMC) {
			#if defined(GET_WAW) || defined(REFINEMENTS)
				PunctuationInfo copyPunc=stateInfo.currentPunctuationInfo;
			#endif
			#ifdef GET_WAW
				if (s.has_waw && (stateInfo.currentState==NAME_S ||stateInfo.currentState==NRC_S)) {
					display("waw ");
					stateInfo.isWaw=true;
					stateInfo.startPos=s.wawStart;
					stateInfo.endPos=s.wawEnd;
					stateInfo.nextPos=s.wawEnd+1;
					stateInfo.currentPunctuationInfo.reset();
					if (!result(NRC,stateInfo,currentChain))
						return false;
					stateInfo.isWaw=false;
					stateInfo.currentState=stateInfo.nextState;
					stateInfo.lastEndPos=stateInfo.endPos;
				}
			#endif
			#ifdef REFINEMENTS
				display("FamilyNMC ");
				stateInfo.familyNMC=true;
				if (s.ibn)
					stateInfo.ibn=true;
				stateInfo.startPos=s.startStem;
				stateInfo.endPos=s.finishStem;
				stateInfo.currentPunctuationInfo.reset();
				if (s.finishStem<finish)
					stateInfo.nextPos=s.finishStem+1;
				else
					stateInfo.currentPunctuationInfo=copyPunc;
				if (!result(NMC,stateInfo,currentChain))
					return false;
				stateInfo.currentState=stateInfo.nextState;
				stateInfo.lastEndPos=stateInfo.endPos;
				if (s.finishStem<finish) {
					stateInfo.familyNMC=false;
					stateInfo.ibn=false;
					stateInfo.startPos=s.finishStem+1;
					stateInfo.endPos=finish;
					return result(NAME,stateInfo,currentChain);
				}
				return true;
			#else
				return result(NMC,stateInfo,currentChain);
			#endif
			}
			if (s.possessive) {
				display("Possessive ");
				return result(NMC,stateInfo,currentChain);
			}
			return result(NMC,stateInfo,currentChain);
		}
		else if (s.name){
		#ifdef GET_WAW
			PunctuationInfo copyPunc=stateInfo.currentPunctuationInfo;
			if (s.has_waw && (stateInfo.currentState==NAME_S ||stateInfo.currentState==NRC_S) ) {
				display("waw ");
				stateInfo.isWaw=true;
				stateInfo.startPos=s.wawStart;
				stateInfo.endPos=s.wawEnd;
				stateInfo.nextPos=s.wawEnd+1;
				stateInfo.currentPunctuationInfo.reset();
				if (!result(NRC,stateInfo,currentChain))
					return false;
				stateInfo.isWaw=false;
				stateInfo.currentState=stateInfo.nextState;
				stateInfo.lastEndPos=stateInfo.endPos;
			}
			stateInfo.currentPunctuationInfo=copyPunc;
		#endif
		#ifdef REFINEMENTS
			stateInfo.learnedName=s.learnedName;
			stateInfo.startPos=s.startStem;
			stateInfo.endPos=s.finishStem;
		#endif
			return result(NAME,stateInfo,currentChain);
		}
		else
			return result(NMC,stateInfo,currentChain);
	}
#endif
public:
	int segment(QString input_str,ATMProgressIFC *prg)  {
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
		QFile input(input_str);
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
		chainDataBiography *currentChain=new chainDataBiography();
		currentChain->initialize(text);
		display(QString("\ninit%1\n").arg(currentChain->narrator->m_narrator.size()));
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
			if((proceedInStateMachine(stateInfo,currentChain)==false)) {
				if (currentData.narratorCount>=hadithParameters.narr_min) {
					sanadEnd=currentData.narratorEndIndex;
				#ifdef DISPLAY_HADITH_OVERVIEW
					newHadithStart=currentData.sanadStartIndex;
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
		//delete text;
		if (currentChain!=NULL)
			delete currentChain;
		return 0;
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
	s.segment(input_str,prg);
	return 0;
}

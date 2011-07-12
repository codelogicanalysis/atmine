#include "narratordetector.h"
#include "graph.h"
#include "narratorHash.h"

#ifdef NARRATORDEBUG
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

class NarratorDetector
{
private:
	typedef struct stateData_ {
		long  biographyStartIndex, narratorCount,narratorStartIndex,narratorEndIndex;
		long  nmcCount, nrcCount,nameStartIndex,nmcStartIndex;
		bool nmcValid;
		bool ibn_or_3abid;

		void initialize() {
			nmcCount=0;
			narratorCount=0;
			nrcCount=0;
			narratorStartIndex=0;
			narratorEndIndex=0;
			nmcValid=false;
			ibn_or_3abid=false;
			nameStartIndex=0;
			nmcStartIndex=0;
			biographyStartIndex=0;
		}

	} stateData;
	class BiographyData {
	public:
		NamePrim *namePrim;
		NameConnectorPrim *nameConnectorPrim;
		TempConnectorPrimList * temp_nameConnectors;
		Narrator *narrator;
		Biography *biography;

		void initialize(QString * text) {
			if (namePrim!=NULL)
				delete namePrim;
			if (nameConnectorPrim!=NULL)
				delete nameConnectorPrim;
			if (narrator!=NULL)
				delete narrator;
			int s=0;
			if (biography!=NULL) {
				s=biography->getStart();
				delete biography;
			}
			/*for (int i=0;i<temp_nameConnectors->count()-1;i++)
				delete (*temp_nameConnectors)[i];*/
			delete temp_nameConnectors;
			namePrim=new NamePrim(text);
			nameConnectorPrim=new NameConnectorPrim(text);
			narrator=new Narrator (text);
			biography=new Biography(text,s);
			temp_nameConnectors=new TempConnectorPrimList();
		}
		BiographyData(){
			namePrim=NULL;
			nameConnectorPrim=NULL;
			temp_nameConnectors=NULL;
			narrator=NULL;
			biography=NULL;
		}
	};
	typedef QList<ChainNarratorNode *> ChainNodeList;

	stateData currentData;
	QString * text;
	long current_pos;
#ifdef SEGMENT_BIOGRAPHY_USING_POR
	NarratorGraph * graph;
#endif

public:
	BiographyList * biographies;

private:
	bool getNextState(StateInfo &  stateInfo,BiographyData *currentBiography) {
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
	#ifdef REFINEMENTS
		bool reachedRasoul= (stateInfo.currentType== STOP_WORD && !stateInfo.ibnOr3abid());//stop_word not preceeded by 3abid or ibn
	#endif

		if (stateInfo.currentType==NRC)//here NRC has no meaning
			stateInfo.currentType=NMC;

		bool return_value=true;
		switch(stateInfo.currentState)
		{
		case TEXT_S:
			if(stateInfo.currentType==NAME) {
				currentData.initialize();
				stateInfo.nextState=NAME_S;
				currentData.biographyStartIndex=stateInfo.startPos;
				currentData.narratorStartIndex=stateInfo.startPos;
			#ifdef CHAIN_BUILDING
				currentBiography->initialize(text);
				display(QString("\ninit%1\n").arg(currentBiography->narrator->m_narrator.size()));
				currentBiography->namePrim->m_start=stateInfo.startPos; //changed this now
				currentBiography->namePrim->m_end=stateInfo.endPos;
			#endif
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.has_punctuation) {
					display("<punc1>");
					currentData.narratorCount++;
					stateInfo.nextState=NRC_S;
					currentData.nrcCount=0;//punctuation is zero
					currentData.narratorEndIndex=stateInfo.endPos;
				#ifdef CHAIN_BUILDING
					currentBiography->namePrim->m_end=stateInfo.endPos;
					currentBiography->narrator->m_narrator.append(currentBiography->namePrim);
					assert(currentBiography->narrator->m_narrator.size()>0);
					currentBiography->biography->addNarrator(currentBiography->narrator);
					currentBiography->narrator=new Narrator(text);
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
	#ifdef IBN_START//needed in case a hadith starts by ibn such as "ibn yousef qal..."
			else if (stateInfo.currentType==NMC && stateInfo.familyNMC) {
				display("<IBN1>");
				currentData.initialize();
				currentData.nmcStartIndex=stateInfo.startPos;
				currentData.narratorStartIndex=stateInfo.startPos;
				currentData.nmcCount=1;
				stateInfo.nextState=NMC_S;
				currentData.nmcValid=true;
			#ifdef CHAIN_BUILDING
				currentBiography->initialize(text);
				display(QString("\ninit%1\n").arg(currentBiography->narrator->m_narrator.size()));
				currentBiography->nameConnectorPrim->m_start=stateInfo.startPos;
				currentBiography->nameConnectorPrim->m_end=stateInfo.endPos;
				currentBiography->nameConnectorPrim->setIbn();
				currentBiography->namePrim->m_start=stateInfo.nextPos;//added this now
			#endif
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
					stateInfo.nextState=TEXT_S;
				#ifdef CHAIN_BUILDING
					currentBiography->narrator->m_narrator.append(currentBiography->nameConnectorPrim);
					assert(currentBiography->narrator->m_narrator.size()>0);
					currentBiography->biography->addNarrator(currentBiography->narrator);
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
			if(reachedRasoul)
			{
				display("<STOP1>");
				stateInfo.nextState=STOP_WORD_S;
			#ifdef COUNT_RASOUL
				currentData.narratorCount++;
			#endif
			#ifdef CHAIN_BUILDING
				currentBiography->namePrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
				currentBiography->narrator->m_narrator.append(currentBiography->namePrim);
				delete currentBiography->temp_nameConnectors;
				currentBiography->temp_nameConnectors= new TempConnectorPrimList();

				currentBiography->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos); //we added this to previous name bc assumed this will only happen if it is muhamad and "sal3am"
				currentBiography->nameConnectorPrim->m_end=stateInfo.endPos;
				currentBiography->narrator->m_narrator.append(currentBiography->nameConnectorPrim);
				assert(currentBiography->narrator->m_narrator.size()>0);
				currentBiography->narrator->isRasoul=true;
				currentBiography->biography->addNarrator(currentBiography->narrator);
			#endif
				currentData.narratorEndIndex=stateInfo.endPos;

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
				currentBiography->namePrim->m_end=stateInfo.lastEndPos;
				currentBiography->narrator->m_narrator.append(currentBiography->namePrim);
				currentBiography->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
				currentBiography->nameConnectorPrim->m_end=stateInfo.endPos;
			#ifdef REFINEMENTS
				if (stateInfo.familyNMC) {
					currentBiography->nameConnectorPrim->setFamilyConnector();
					if (stateInfo.ibn)
						currentBiography->nameConnectorPrim->setIbn();
				} else if (stateInfo.possessivePlace)
					currentBiography->nameConnectorPrim->setPossessive();
			#endif
				/*for (int i=0;i<currentChain->temp_nameConnectors->count()-1;i++)
					delete (*currentChain->temp_nameConnectors)[i];*/
				delete currentBiography->temp_nameConnectors;
				currentBiography->temp_nameConnectors= new TempConnectorPrimList();
			#endif
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.has_punctuation) {
					currentData.nmcCount=hadithParameters.bio_nmc_max+1;
					if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
						stateInfo.nextState=TEXT_S;
					#ifdef CHAIN_BUILDING
						if (currentBiography->narrator->m_narrator.size()>0) //not needed check
							currentBiography->biography->addNarrator(currentBiography->narrator);
					#endif
						currentData.narratorEndIndex=stateInfo.endPos;
						currentData.narratorCount++;
						return_value=false;
					}
				}
			#endif
			}
			else {
				stateInfo.nextState=NAME_S;
				currentData.nmcValid=false;
				currentBiography->namePrim->m_end=stateInfo.endPos;
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.has_punctuation) {
					display("<punc2>");
					currentData.narratorCount++;
					stateInfo.nextState=NRC_S;
					currentData.nrcCount=0; //punctuation not counted
					currentData.narratorEndIndex=stateInfo.endPos;
				#ifdef CHAIN_BUILDING
					currentBiography->namePrim->m_end=stateInfo.endPos;
					currentBiography->narrator->m_narrator.append(currentBiography->namePrim);
					assert(currentBiography->narrator->m_narrator.size()>0);
					currentBiography->biography->addNarrator(currentBiography->narrator);
					currentBiography->narrator=new Narrator(text);
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
			if(reachedRasoul) {
				display("<STOP2>");
				//1-finish old narrator and use last nmc as nrc
				currentData.narratorCount++;
				display(QString("counter%1\n").arg(currentData.narratorCount));
				stateInfo.nextState=NAME_S;
				currentData.nrcCount=0;

				currentData.narratorEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(currentData.nmcStartIndex);
			#ifdef CHAIN_BUILDING
				/*for (int i=0;i<currentBiography->temp_nameConnectors->count();i++)
					currentBiography->narrator->m_narrator.append(currentBiography->temp_nameConnectors->at(i));*/ //changed this
				if (currentBiography->narrator->m_narrator.size()>0)
					currentBiography->biography->addNarrator(currentBiography->narrator);
				display(currentBiography->narrator->getString()+"\n");
			#endif
			//2-create a new narrator of just this stop word as name connector
			#ifdef CHAIN_BUILDING
				currentBiography->nameConnectorPrim->m_start=stateInfo.startPos;
				currentBiography->nameConnectorPrim->m_end=stateInfo.endPos;
				currentBiography->narrator=new Narrator(text);
				currentBiography->narrator->m_narrator.append(currentBiography->nameConnectorPrim);
				currentBiography->narrator->isRasoul=true;
				assert(currentBiography->narrator->m_narrator.size()>0);
				currentBiography->biography->addNarrator(currentBiography->narrator);
			#endif

				currentData.narratorEndIndex=stateInfo.endPos;
				stateInfo.nextState=STOP_WORD_S;
			#ifdef COUNT_RASOUL
				currentData.narratorCount++;
			#endif
				//return_value= false;
				break;
			}
		#endif
			else if(stateInfo.currentType==NAME) {
				currentData.nmcCount=0;
				stateInfo.nextState=NAME_S;
				currentData.nmcValid=false;
			#ifdef CHAIN_BUILDING
				currentBiography->nameConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
				currentBiography->temp_nameConnectors->append(currentBiography->nameConnectorPrim);
				for (int i=0;i<currentBiography->temp_nameConnectors->count();i++)
					currentBiography->narrator->m_narrator.append(currentBiography->temp_nameConnectors->at(i));
				currentBiography->namePrim=new NamePrim(text,stateInfo.startPos);
				currentBiography->namePrim->m_end=stateInfo.endPos;
			#endif
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.has_punctuation) {
					display("<punc3>");
					currentData.narratorCount++;
					stateInfo.nextState=NRC_S;
					currentData.nrcCount=0;
					currentData.narratorEndIndex=stateInfo.endPos;
				#ifdef CHAIN_BUILDING
					currentBiography->namePrim->m_end=stateInfo.endPos;
					currentBiography->narrator->m_narrator.append(currentBiography->namePrim);
					assert(currentBiography->narrator->m_narrator.size()>0);
					currentBiography->biography->addNarrator(currentBiography->narrator);
					currentBiography->narrator=new Narrator(text);
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
				currentData.nmcCount=hadithParameters.bio_nmc_max+1;
				currentData.nmcValid=false;
				if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
					stateInfo.nextState=TEXT_S;
				#ifdef CHAIN_BUILDING
					/*currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
					for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
						currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));*/
					if (currentBiography->narrator->m_narrator.size()>0)
						currentBiography->biography->addNarrator(currentBiography->narrator);
					display("{this}");
				#endif
					currentData.narratorEndIndex=(currentBiography->narrator->m_narrator.size()!=0?
												  currentBiography->narrator->getEnd():
												  (currentBiography->narrator->m_narrator.size()==0?
												   stateInfo.lastEndPos:
												   currentBiography->biography->getLastNarrator()->getEnd()));
					currentData.narratorCount++;
					return_value=false;
					break;
				}
			}
		#endif
			else if (currentData.nmcCount>hadithParameters.bio_nmc_max
					#ifdef PUNCTUATION
						 || stateInfo.number ||
						 (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine)
					#endif
				) {
				if (currentData.nmcCount>hadithParameters.bio_nmc_max && currentData.nmcValid) {//number is severe condition no tolerance
					currentData.nmcValid=false;
					stateInfo.nextState=NMC_S;
					currentData.nmcCount=0;
				} else {
					stateInfo.nextState=NRC_S;//was TEXT_S;
				#ifdef CHAIN_BUILDING
					/*currentChain->nameConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(start_index);//later check for out of bounds
					currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
					for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
						currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));//check to see if we should also add the narrator to chain
					*/
					if (currentBiography->narrator->m_narrator.size()>0)
						currentBiography->biography->addNarrator(currentBiography->narrator);
					currentBiography->narrator=new Narrator(text);
				#endif

					// TODO: added this later to the code, check if really is in correct place, but seemed necessary
					currentData.narratorCount++;
					//till here was added later

					display("{check}");
					currentData.narratorEndIndex=(currentBiography->narrator->m_narrator.size()!=0?
												  currentBiography->narrator->getEnd():
												  (currentBiography->narrator->m_narrator.size()==0?
												   stateInfo.lastEndPos:
												   currentBiography->biography->getLastNarrator()->getEnd()));
					//return_value= false; //changed this now
					break;
				}
				//currentData.narratorEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(start_index); check this case

			}
			else if (stateInfo.currentType==NMC) {
			#ifdef CHAIN_BUILDING
				currentBiography->nameConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
				currentBiography->temp_nameConnectors->append(currentBiography->nameConnectorPrim);
				currentBiography->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
				currentBiography->nameConnectorPrim->m_end=stateInfo.endPos;
			#ifdef REFINEMENTS
				if (stateInfo.familyNMC) {
					currentBiography->nameConnectorPrim->setFamilyConnector();
					if (stateInfo.ibn)
						currentBiography->nameConnectorPrim->setIbn();
				}else if (stateInfo.possessivePlace)
					currentBiography->nameConnectorPrim->setPossessive();
			#endif
			#endif
				currentData.nmcCount++;
				if (stateInfo.isFamilyConnectorOrPossessivePlace())
					currentData.nmcValid=true;
				stateInfo.nextState=NMC_S;
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
					stateInfo.nextState=TEXT_S;
				#ifdef CHAIN_BUILDING
					currentBiography->nameConnectorPrim->m_end=stateInfo.endPos;
					currentBiography->temp_nameConnectors->append(currentBiography->nameConnectorPrim);
					for (int i=0;i<currentBiography->temp_nameConnectors->count();i++)
						currentBiography->narrator->m_narrator.append(currentBiography->temp_nameConnectors->at(i));
					assert(currentBiography->narrator->m_narrator.size()>0);
					currentBiography->biography->addNarrator(currentBiography->narrator);
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
				currentBiography->nameConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
				currentBiography->temp_nameConnectors->append(currentBiography->nameConnectorPrim);
				currentBiography->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
				currentBiography->nameConnectorPrim->m_end=stateInfo.endPos;
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
					stateInfo.nextState=TEXT_S;
				#ifdef CHAIN_BUILDING
					currentBiography->nameConnectorPrim->m_end=stateInfo.endPos;
					currentBiography->temp_nameConnectors->append(currentBiography->nameConnectorPrim);
					for (int i=0;i<currentBiography->temp_nameConnectors->count();i++)
						currentBiography->narrator->m_narrator.append(currentBiography->temp_nameConnectors->at(i));
					assert(currentBiography->narrator->m_narrator.size()>0);
					currentBiography->biography->addNarrator(currentBiography->narrator);
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
			if(reachedRasoul) {
				display("<STOP3>");
			#ifdef CHAIN_BUILDING
				//1-first add thus stop word as NMC
				/*if (currentBiography->narrator!=NULL)
					delete currentBiography->narrator;*/
				currentBiography->narrator=new Narrator(text);
				currentBiography->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
				currentBiography->nameConnectorPrim->m_end=stateInfo.endPos;
				currentBiography->temp_nameConnectors->clear();
				//currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);
			#endif
				currentData.nmcCount=1;
				currentData.narratorStartIndex=stateInfo.startPos;
				currentData.nmcStartIndex=stateInfo.startPos;

				//2-add this narrator and end lookup
				stateInfo.nextState=STOP_WORD_S;
			#ifdef COUNT_RASOUL
				currentData.narratorCount++;
			#endif
			#ifdef CHAIN_BUILDING
				currentBiography->nameConnectorPrim->m_end=stateInfo.endPos;
				currentBiography->narrator->m_narrator.append(currentBiography->nameConnectorPrim);
				assert(currentBiography->narrator->m_narrator.size()>0);
				currentBiography->narrator->isRasoul=true;
				currentBiography->biography->addNarrator(currentBiography->narrator);
			#endif
				currentData.narratorEndIndex=stateInfo.endPos;
				//return_value= false;
				break;
			}
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
			#ifdef REFINEMENTS
				if (stateInfo.currentType==NAME){
			#endif
					currentBiography->namePrim=new NamePrim(text,stateInfo.startPos);
					currentBiography->namePrim->m_end=stateInfo.endPos;
			#ifdef REFINEMENTS
					currentBiography->namePrim->learnedName=stateInfo.learnedName;
				} else {
					currentBiography->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
					currentBiography->nameConnectorPrim->m_end=stateInfo.endPos;
					currentBiography->nameConnectorPrim->setPossessive();
					currentBiography->namePrim=new NamePrim(text,stateInfo.startPos);//added this now
					currentData.narratorStartIndex=stateInfo.startPos;
					currentData.nmcStartIndex=stateInfo.startPos;
				}
			#endif
			#endif
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.has_punctuation)
				{
					display("<punc4>");
					currentData.narratorCount++;
					stateInfo.nextState=NRC_S;
					currentData.nrcCount=0;
					currentData.narratorEndIndex=stateInfo.endPos;
				#ifdef CHAIN_BUILDING
				#ifdef REFINEMENTS
					if (stateInfo.currentType==NAME) {
				#endif
						currentBiography->namePrim->m_end=stateInfo.endPos;
						currentBiography->narrator->m_narrator.append(currentBiography->namePrim);
				#ifdef REFINEMENTS
					} else {
						currentBiography->nameConnectorPrim->m_end=stateInfo.endPos;
						currentBiography->narrator->m_narrator.append(currentBiography->nameConnectorPrim);
					}
				#endif

					if (currentBiography->narrator->m_narrator.size()>0)
						currentBiography->biography->addNarrator(currentBiography->narrator);
					currentBiography->narrator=new Narrator(text);
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
			else if ( stateInfo.number || currentData.nrcCount>=hadithParameters.bio_nrc_max) { //if not in refinements mode stateInfo.number will always remain false
				stateInfo.nextState=TEXT_S;

				//currentData.narratorEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(start_index); //check this case
				return_value= false;
				break;
			}
		#endif
		#ifdef IBN_START
			else if (stateInfo.currentType==NMC && stateInfo.familyNMC) {
				display("<IBN3>");
			#ifdef CHAIN_BUILDING
				/*currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
				for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
					currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));*/
				/*if (currentBiography->narrator!=NULL)
					delete currentBiography->narrator;*/
				currentBiography->narrator=new Narrator(text);
				currentBiography->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
				currentBiography->nameConnectorPrim->m_end=stateInfo.endPos;
				currentBiography->nameConnectorPrim->setFamilyConnector();
				if (stateInfo.ibn)
					currentBiography->nameConnectorPrim->setIbn();
				currentBiography->temp_nameConnectors->clear();
				//currentChain->narrator->m_narrator.append(currentChain->nameConnectorPrim);
			#endif
				currentData.nmcValid=true;
				currentData.nmcCount=1;
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
				#ifdef TRYTOLEARN
					stateInfo.nrcIsPunctuation=true;
				#endif
				#ifdef CHAIN_BUILDING
					currentBiography->narrator->m_narrator.append(currentBiography->nameConnectorPrim);
					assert(currentBiography->narrator->m_narrator.size()>0);
					currentBiography->biography->addNarrator(currentBiography->narrator);
					currentBiography->narrator=new Narrator(text);
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
			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
					stateInfo.nextState=TEXT_S;
					currentData.narratorEndIndex=stateInfo.endPos;
					return_value=false;
					break;
				}
			#endif
			}
			break;
	#ifdef REFINEMENTS
		case STOP_WORD_S:
			if (stateInfo.currentType==STOP_WORD) {
				stateInfo.nextState=STOP_WORD_S;
				Narrator * n=currentBiography->biography->getLastNarrator();
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
				stateInfo.nextState=NRC_S; //was TEXT_S
				currentBiography->narrator=new Narrator(text);
				//return_value=false;
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
	inline bool result(wordType t, StateInfo &  stateInfo,BiographyData *currentBiography){display(t); stateInfo.currentType=t; return getNextState(stateInfo,currentBiography);}
	bool proceedInStateMachine(StateInfo &  stateInfo,BiographyData *currentBiography) { //does not fill stateInfo.currType
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
	#ifdef PUNCTUATION
		if (isNumber(text,stateInfo.startPos,finish)) {
			display("Number ");
			stateInfo.number=true;
			stateInfo.endPos=finish;
			stateInfo.nextPos=next_positon(text,finish+1,stateInfo.currentPunctuationInfo);
			display(text->mid(stateInfo.startPos,finish-stateInfo.startPos+1)+":");
			return result(NMC,stateInfo,currentBiography);
		}
	#endif

		QString c;
		bool found,phrase=false,stop_word=false;
		foreach (c, rasoul_words) {
			int pos;
			if (startsWith(text->midRef(stateInfo.startPos),c,pos))	{
				stop_word=true;
				found=true;
				finish=pos+stateInfo.startPos;
				break;
			}
		}
		if (!stop_word) {//TODO: maybe modified to be set as a utility function, and just called from here
			foreach (c, compound_words)	{
				int pos;
				if (startsWith(text->midRef(stateInfo.startPos),c,pos))	{
					phrase=true;
					found=true;
					finish=pos+stateInfo.startPos;
					break;
				}
			}
		}
		if (!stop_word && !phrase)	{
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
	#ifdef REFINEMENTS
		}
	#endif
		stateInfo.endPos=finish;
		stateInfo.nextPos=next_positon(text,finish,stateInfo.currentPunctuationInfo);
		display(text->mid(stateInfo.startPos,finish-stateInfo.startPos+1)+":");
	#ifdef REFINEMENTS
		if (stop_word || s.stopword) {
			return result(STOP_WORD,stateInfo,currentBiography);
		}
		if (phrase)
		{
			display("PHRASE ");
			//isBinOrPossessive=true; //same behaviour as Bin
			return result(NMC,stateInfo,currentBiography);
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
			return result(NRC,stateInfo,currentBiography);
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
					if (!result(NRC,stateInfo,currentBiography))
						return false;
					stateInfo.isWaw=false;
					stateInfo.currentState=stateInfo.nextState;
					stateInfo.lastEndPos=stateInfo.endPos;
				}
			#endif
			#ifdef REFINEMENTS
				display("Bin ");
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
				if (!result(NMC,stateInfo,currentBiography))
					return false;
				stateInfo.currentState=stateInfo.nextState;
				stateInfo.lastEndPos=stateInfo.endPos;
				if (s.finishStem<finish) {
					stateInfo.familyNMC=false;
					stateInfo.startPos=s.finishStem+1;
					stateInfo.endPos=finish;
					return result(NAME,stateInfo,currentBiography);
				}
				return true;
			#else
				return result(NMC,stateInfo,currentChain);
			#endif
			}
			if (s.possessive) {
				display("Possessive ");
				return result(NMC,stateInfo,currentBiography);
			}
			return result(NMC,stateInfo,currentBiography);
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
				if (!result(NRC,stateInfo,currentBiography))
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
			return result(NAME,stateInfo,currentBiography);
		}
		else
			return result(NMC,stateInfo,currentBiography);
	}
#ifdef SEGMENT_BIOGRAPHY_USING_POR
	class RealNarratorAction:public NarratorHash::FoundAction {
	private:
		ChainNodeList & list;
		bool found;
	public:
		RealNarratorAction(ChainNodeList & nodeList):list(nodeList) {}
		virtual void action(const QString & , ChainNarratorNode * node, double similarity){
			if (similarity>hadithParameters.equality_threshold) {
				found =true;
				if (!list.contains(node))
					list.append(node);
			}
		}
		void resetFound() {found=false;}
		bool isFound() {return found;}
	};
	bool near(ChainNarratorNode * c1, ChainNarratorNode * c2) {
		NarratorNodeIfc & n1=c1->getCorrespondingNarratorNode();
		NarratorNodeIfc & n2=c2->getCorrespondingNarratorNode();
		ChainNodeIterator itr=n1.begin();
		for (;!itr.isFinished();++itr) {
			if (&itr.getChild()==&n2)
				return true;
			if (&itr.getParent()==&n2)
				return true;
		}
		return false;
	}
	bool near(ChainNarratorNode * c, const ChainNodeList & list) {
		for (int i=0;i<list.size();i++) {
			if (near(c,list[i]))
				return true;
		}
		return false;
	}
	int getNearestNodesNumber(ChainNodeList & list) {
		QList<ChainNodeList> nearNodesLists;
		//1-each node put it in its own list or merge it with a group of already found if it is near them
		for (int i=0;i<list.size();i++) {
			bool nearSomeNode=false;
			for (int j=0;j<nearNodesLists.size();j++) {
				if (near(list[i],nearNodesLists[j])) {
					nearSomeNode=true;
					nearNodesLists[j].append(list[i]);
				}
			}
			if (!nearSomeNode) {
				ChainNodeList newList;
				newList.append(list[i]);
				nearNodesLists.append(newList);
			}
		}
		//2-merge seperate lists in case they turn to overlap at last in case they are near each other
		for (int i=0;i<nearNodesLists.size();i++) {
			for (int j=i+1;j<nearNodesLists.size();j++) {
				for (int k=0;k<nearNodesLists[j].size();k++) {
					if (near(nearNodesLists[j][k],nearNodesLists[i])) {
						for (int r=0;r<nearNodesLists[j].size();r++) {
							if (!nearNodesLists[i].contains(nearNodesLists[j][r])) //dont append nodes already there
								nearNodesLists[i].append(nearNodesLists[j][r]);
						}
						nearNodesLists.removeAt(j);
						j--;
						break;
					}
				}
			}
		}
		//3-return largest list size
		int largest=0;
		int index=-1;
		for (int i=0;i<nearNodesLists.size();i++) {
			if (nearNodesLists[i].size()>largest) {
				largest=nearNodesLists[i].size();
				index=i;
			}
		}
		if (index>=0) {
			qDebug()<<largest<<"\n";
			for (int i=0;i<nearNodesLists[index].size();i++){
				qDebug()<<nearNodesLists[index][i]->CanonicalName()<<"\n";
			}
			qDebug()<<"\n";
		}
		return largest;
	}
	ChainNodeList getRealNarrators(Biography * biography) {
		ChainNodeList list;
		RealNarratorAction v(list);
		for (int i=0;i<biography->size();i++) {
			Narrator * n=(*biography)[i];
			if (n->isRasoul) {
				biography->removeNarrator(i);
				i--;
			} else {
				v.resetFound();
				graph->performActionToAllCorrespondingNodes(n,v);
				if (!v.isFound()) {
					biography->removeNarrator(i);
					i--;
				}
			}
		}
		return list;
	}
#endif

public:
#ifdef SEGMENT_BIOGRAPHY_USING_POR
	NarratorDetector(NarratorGraph * graph) {this->graph=graph; }
	NarratorDetector() {}
#endif
	int segment(QString input_str,ATMProgressIFC *prg)  {
		QFile chainOutput(chainDataStreamFileName);

		chainOutput.remove();
		if (!chainOutput.open(QIODevice::ReadWrite))
			return 1;
		QDataStream chainOut(&chainOutput);
		QFile input(input_str);
		if (!input.open(QIODevice::ReadOnly)) {
			out << "File not found\n";
			return 1;
		}
		QTextStream file(&input);
		file.setCodec("utf-8");
		text=new QString(file.readAll());
		if (text==NULL)	{
			out<<"file error:"<<input.errorString()<<"\n";
			return 1;
		}
		if (text->isEmpty()) { //ignore empty files
			out<<"empty file\n";
			return 0;
		}
		long text_size=text->size();
		long  biographyStart=-1;
		currentData.initialize();

	#ifdef CHAIN_BUILDING
		BiographyData *currentBiography=new BiographyData();
		currentBiography->initialize(text);
		display(QString("\ninit%1\n").arg(currentBiography->narrator->m_narrator.size()));
	#else
		chainData *currentBiography=NULL;
	#endif
		long  biographyEnd;
		int biography_Counter=1;
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
		while(stateInfo.startPos<text->length() && isDelimiter(text->at(stateInfo.startPos)))
			stateInfo.startPos++;
	#ifdef PROGRESSBAR
		prg->setCurrentAction("Parsing Biography");
	#endif
		for (;stateInfo.startPos<text_size;) {
			if((proceedInStateMachine(stateInfo,currentBiography)==false)) {
			#ifdef SEGMENT_BIOGRAPHY_USING_POR
				ChainNodeList realNarrators=getRealNarrators(currentBiography->biography);
				int num=getNearestNodesNumber(realNarrators);
				if (num>=hadithParameters.bio_narr_min) {
			#else
				if (currentData.narratorCount>=hadithParameters.bio_narr_min) {
			#endif
					//biographyEnd=currentData.narratorEndIndex;
					biographyEnd=stateInfo.endPos;
					currentBiography->biography->setEnd(biographyEnd);
				#ifdef DISPLAY_HADITH_OVERVIEW
					//biographyStart=currentData.biographyStartIndex;
					biographyStart=currentBiography->biography->getStart();
					//long end=text->indexOf(QRegExp(delimiters),sanadEnd);//sanadEnd is first letter of last word in sanad
					//long end=stateInfo.endPos;
					out<<"\n"<<biography_Counter<<" new biography start: "<<text->mid(biographyStart,display_letters)<<endl;
					out<<"sanad end: "<<text->mid(biographyEnd-display_letters+1,display_letters)<<endl<<endl;
				#ifdef CHAIN_BUILDING
					currentBiography->biography->serialize(chainOut);
					currentBiography->biography->setStart(stateInfo.nextPos);
					//currentChain->chain->serialize(displayed_error);
				#endif
				#endif
					biography_Counter++;
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
			/*if (stateInfo.previousPunctuationInfo.has_punctuation)
				stateInfo.previousPunctuationInfo.fullstop=true;*/
			if (stateInfo.previousPunctuationInfo.fullstop) {
				if (currentBiography->biography!=NULL)
					delete currentBiography->biography;
				currentBiography->biography=new Biography(text,stateInfo.startPos);
			}


		#endif
	#ifdef PROGRESSBAR
			prg->report((double)stateInfo.startPos/text_size*100+0.5);
			if (stateInfo.startPos==text_size-1)
				break;
	#endif
		}
	#if defined(DISPLAY_HADITH_OVERVIEW)
		if (biographyStart<0)
		{
			out<<"no biography found\n";
			chainOutput.close();
			return 2;
		}
		chainOutput.close();
	#endif
	#ifdef CHAIN_BUILDING //just for testing deserialize
		QFile f("biography_chains.txt");
		if (!f.open(QIODevice::WriteOnly))
			return 1;
		QTextStream file_biography(&f);
			file_biography.setCodec("utf-8");

		if (!chainOutput.open(QIODevice::ReadWrite))
			return 1;
		QDataStream tester(&chainOutput);
		int tester_Counter=1;
	#ifdef TEST_BIOGRAPHIES
		biographies=new BiographyList;
		biographies->clear();
	#endif
	#if defined(TAG_HADITH)
		prg->startTaggingText(*text);
	#endif
		while (!tester.atEnd())
		{
			Biography * s=new Biography(text);
			s->deserialize(tester);
		#ifdef TEST_BIOGRAPHIES
			biographies->append(s);
		#endif
		#if defined(TAG_HADITH)
			for (int j=0;j<s->size();j++)
			{
				const Narrator * n=(*s)[j];
				if (n->m_narrator.size()==0) {
					out<<"found a problem an empty narrator in ("<<tester_Counter<<","<<j<<")\n";
					continue;
				}
				prg->tag(n->getStart(),n->getLength(),Qt::darkYellow,false);
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
		#else
			hadith_out<<tester_Counter<<" ";
			s->serialize(hadith_out);
		#endif
			tester_Counter++;
			s->serialize(file_biography);
		}
		chainOutput.close();
		f.close();
	#endif
	#ifndef TAG_HADITH
		prg->startTaggingText(*hadith_out.string()); //we will not tag but this will force a text to be written there
	#else
		prg->finishTaggingText();
	#endif


		//delete text;
		if (currentBiography!=NULL)
			delete currentBiography;
		return 0;
	}
		
};

int biographyHelper(QString input_str,ATMProgressIFC *prg) {
	input_str=input_str.split("\n")[0];
	NarratorDetector s;
	s.segment(input_str,prg);
#ifdef TEST_BIOGRAPHIES
	for (int i=0;i<s.biographies->size();i++)
		delete (*s.biographies)[i];
#endif
	return 0;
}

#ifdef TEST_BIOGRAPHIES
BiographyList * getBiographies(QString input_str,NarratorGraph* graph,ATMProgressIFC *prg) {
	input_str=input_str.split("\n")[0];
#ifdef SEGMENT_BIOGRAPHY_USING_POR
	NarratorDetector s(graph);
#else
	NarratorDetector s;
#endif
	s.segment(input_str,prg);
	return s.biographies;
}
#endif

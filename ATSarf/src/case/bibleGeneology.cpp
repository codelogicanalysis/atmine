#include "bibleGeneology.h"
#include <QTextBrowser>
#include <assert.h>

#include "ATMProgressIFC.h"
#include "Math_functions.h"
#include "graph.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <assert.h>
#include <QFile>
#include <QRegExp>
#include <QString>
#include <QStringList>

#include "stemmer.h"
#include "letters.h"
#include "diacritics.h"
#include "text_handling.h"
#include "textParsing.h"
#include "common.h"
#include "Retrieve_Template.h"
#include "Search_by_item_locally.h"
#include "narrator_abstraction.h"


#ifdef GENEOLOGYDEBUG
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

	class GeneologyParameters {
	public:
		unsigned int nrc_max:10;
		unsigned int narr_min:10;
		unsigned int unused:12;
		GeneologyParameters() {
			nrc_max=20;
			narr_min=10;
		}
	};

	GeneologyParameters geneologyParameters;
	QString chainDataStreamFileName=".chainOutput";


	enum WordType { NAME, NRC};
	enum StateType { TEXT_S , NAME_S, SONS};

	typedef struct  {
		long lastEndPos;
		long startPos;
		long endPos;
		long nextPos;
		WordType currentType:1;
		StateType currentState:2;
		StateType nextState:2;
		int unused:27;
		PunctuationInfo previousPunctuationInfo,currentPunctuationInfo;
		void resetCurrentWordInfo()	{currentPunctuationInfo.reset();}
	} StateInfo;

	QList<int> bits_NAME;

#define display_letters 30


	class geneology_stemmer: public Stemmer { //TODO: seperate ibn from possessive from 3abid and later seperate between ibn and bin
	private:
		bool place;
	public:
		long finish_pos;
		bool name:1;
	#ifdef GET_WAW
		bool has_waw:1;
	#endif
		long startStem, finishStem,wawStart,wawEnd;

		geneology_stemmer(QString * word, int start)
	#ifndef COMPARE_TO_BUCKWALTER
			:Stemmer(word,start,false)
	#else
			:Stemmer(word,start,true)
	#endif
		{
			setSolutionSettings(M_ALL);
			init(start);
		}
		void init(int start) {
			this->info.start=start;
			this->info.finish=start;
			name=false;
			finish_pos=start;
			startStem=start;
			finishStem=start;
		#ifdef GET_WAW
			wawStart=start;
			wawEnd=start;
			has_waw=false;
		#endif
		}
	#ifndef COMPARE_TO_BUCKWALTER
		bool on_match()	{
			solution_position * S_inf=Stem->computeFirstSolution();
			do
			{
				stem_info=Stem->solution;
			#ifdef GET_AFFIXES_ALSO
				solution_position * p_inf=Prefix->computeFirstSolution();
				do
				{
					prefix_infos=&Prefix->affix_info;
					solution_position * s_inf=Suffix->computeFirstSolution();
					do
					{
						suffix_infos=&Suffix->affix_info;
			#endif
						if (!analyze())
							return false;
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

	#ifdef GET_WAW
		void checkForWaw() {
			has_waw=false;
		#ifndef GET_AFFIXES_ALSO
			solution_position * p_inf=Prefix->computeFirstSolution();
			do {
				prefix_infos=&Prefix->affix_info;
		#endif
				for (int i=0;i<prefix_infos->size();i++)
					if (prefix_infos->at(i).POS=="wa/CONJ+") {
						wawStart=(i==0?Prefix->info.start:Prefix->getSplitPositions().at(i-1)-1);
						wawEnd=Prefix->getSplitPositions().at(i)-1;
						has_waw=true;
						break;
					}
		#ifndef GET_AFFIXES_ALSO
			}while (Prefix->computeNextSolution(p_inf));
			delete p_inf;
		#endif
		}
	#endif

		bool analyze()	{
			if (info.finish>info.start) { //more than one letter to be tested for being a name
				int bitsNamesSize=bits_NAME.size();
				for (int i=0;i<bitsNamesSize;i++) {
					if (stem_info->abstract_categories.getBit(bits_NAME[i])
						#ifdef REFINEMENTS
							&& Suffix->info.finish-Suffix->info.start<0 && Stem->info.finish>Stem->info.start) //i.e. has no suffix and stem > a letter
						#else
							)
						#endif
					{
						name=true;
						if (info.finish>finish_pos) {
							finish_pos=info.finish;
							finishStem=Stem->info.finish;
							startStem=Stem->info.start;
						#ifdef GET_WAW
							checkForWaw();
						#endif
						}
						return true;
					}
				}
			}
		}
	};


#if 1
	typedef struct StateData_ {
		long  mainStructureStartIndex;
		long narratorStartIndex,narratorEndIndex,nrcStartIndex,nrcEndIndex,nameStartIndex;
		int  nrcCount,narratorCount; //bio_nrcCount needed bc dont want finding an NRC to change the tolerance breaking count


		void initialize() {
			narratorCount=0;
			nrcCount=0;
			narratorStartIndex=0;
			narratorEndIndex=0;
			nameStartIndex=0;
			mainStructureStartIndex=0;
			nrcStartIndex=0;
			nrcEndIndex=0;
		}

	} StateData;
	class HadithData {
	public:
		QString * text;
		NarratorGraph *graph;
		bool hadith;

		NamePrim *namePrim;
		NameConnectorPrim *nameConnectorPrim;
		NarratorConnectorPrim *narratorConnectorPrim;
		TempConnectorPrimList temp_nameConnectors;
		Narrator *narrator;
		Chain *chain;
		Biography *biography;

		void initialize(QString * text) {
			if (namePrim!=NULL) {
				delete namePrim;
				namePrim=NULL;
			}
			if (nameConnectorPrim!=NULL) {
				delete nameConnectorPrim;
				nameConnectorPrim=NULL;
			}
			if (narratorConnectorPrim!=NULL) {
				delete narratorConnectorPrim;
				narratorConnectorPrim=NULL;
			}
			if (narrator!=NULL) {
				delete narrator;
				narrator=NULL;
			}
			for (int i=0;i<temp_nameConnectors.size();i++)
				delete temp_nameConnectors[i];
			temp_nameConnectors.clear();
			if (hadith) {
				if (chain!=NULL)
					delete chain;
				chain=new Chain(text);
			} else {
				int s=0;
				if (biography!=NULL) {
					s=biography->getStart();
					delete biography;
				}
				biography=new Biography(graph,text,s);
			}
		}
		HadithData(QString * text, bool hadith, NarratorGraph * graph){
			this->text=text;
			this->hadith=hadith;
			this->graph=graph;
			namePrim=NULL;
			nameConnectorPrim=NULL;
			narrator=NULL;
			narratorConnectorPrim=NULL;
			biography=NULL;
			chain=NULL;
		}
	};


void geneology_initialize() {
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
	int bit_NAME=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_NAME);
	bits_NAME.append(bit_NAME);
}

inline void fillStructure(StateInfo &  stateInfo,const Structure & currentStructure,HadithData *structures, StateData & currentData,bool punc=false,bool ending_punc=false) {
#ifdef CHAIN_BUILDING

#define addNarrator(narrator) \
	if (structures->hadith) \
		structures->chain->m_chain.append(narrator); \
	else {\
		if (structures->biography->addNarrator(narrator)) \
			currentData.bio_nrcCount=0; \
	}


	assert(!ending_punc || (punc&& ending_punc));

	if (punc) {
		switch(stateInfo.processedStructure) {
		case INITIALIZE: {
			assert(false);//must not happen probably
			break;
		}
		case NAME_PRIM: {
			assert(structures->narrator!=NULL);
			assert(structures->narrator->m_narrator.size()>0);
			addNarrator(structures->narrator);
			structures->narrator=NULL;
			assert(currentStructure==NARRATOR_CONNECTOR);
			break;
		}
		case NARRATOR_CONNECTOR: {
			if (structures->hadith) {
				if (!ending_punc) {
					assert (structures->narratorConnectorPrim!=NULL);
					structures->chain->m_chain.append(structures->narratorConnectorPrim);
					structures->narratorConnectorPrim=NULL;
					assert(currentStructure==NAME_PRIM);
				} else { //used to indicate that tolerance is over (not used for punctuation)
					if (structures->narratorConnectorPrim!=NULL) {
						delete structures->narratorConnectorPrim;
						structures->narratorConnectorPrim=NULL;
					}
				}
			} else {
				if (!ending_punc)
					assert(currentStructure==NAME_PRIM);
			}
			break;
		}
		case NAME_CONNECTOR: {
			int size=structures->temp_nameConnectors.size();
			if (!ending_punc) {//check if we should add these
				assert(structures->narrator!=NULL);
				for (int i=0;i<size;i++)
					structures->narrator->m_narrator.append(structures->temp_nameConnectors[i]);
			} else {
				for (int i=0;i<size;i++)
					delete structures->temp_nameConnectors[i];
			}
			structures->temp_nameConnectors.clear();
			if (structures->narrator!=NULL) {
				assert(structures->narrator->m_narrator.size()>0);
				addNarrator(structures->narrator);
				structures->narrator=NULL;
			}
			break;
		}
		default:
			assert(false);
		}
	}else {
		if (currentStructure!=INITIALIZE) {
			if (!structures->hadith)
				currentData.bio_nrcCount++;
			switch(stateInfo.processedStructure) {
			case INITIALIZE:
				structures->initialize(structures->text);
				display(structures->hadith?QString("\ninit%1\n").arg(structures->chain->m_chain.size()):"");
				assert(currentStructure!=INITIALIZE); //must not happen
				break;
			case NARRATOR_CONNECTOR:
				if (structures->hadith && currentStructure!=NARRATOR_CONNECTOR && currentStructure!=INITIALIZE) {
					if(structures->narratorConnectorPrim==NULL) {
						structures->narratorConnectorPrim=new NarratorConnectorPrim(structures->text,stateInfo.lastEndPos+1);
						structures->narratorConnectorPrim->m_end=stateInfo.startPos-1;
						//qDebug()<<currentChain->narratorConnectorPrim->getString();
					}
					structures->chain->m_chain.append(structures->narratorConnectorPrim);
					structures->narratorConnectorPrim=NULL;
				}
				break;
			case NAME_CONNECTOR:
				if (currentStructure!=NAME_CONNECTOR) {
					if (structures->narrator==NULL)
						structures->narrator=new Narrator(structures->text);
					int size=structures->temp_nameConnectors.size();
					for (int i=0;i<size;i++)
						structures->narrator->m_narrator.append(structures->temp_nameConnectors[i]);
					structures->temp_nameConnectors.clear();
					if (currentStructure==NARRATOR_CONNECTOR) {
						addNarrator(structures->narrator);
						structures->narrator=NULL;
					}
				}
				break;
			case RASOUL_WORD:
				if (currentStructure!=RASOUL_WORD) {
					if (!structures->hadith && currentStructure==NARRATOR_CONNECTOR) {
						fillStructure(stateInfo,INITIALIZE,structures,currentData,punc,ending_punc);
						stateInfo.processedStructure=NARRATOR_CONNECTOR;
						return;
					} else
						assert(currentStructure==INITIALIZE);
				}
				break;
			case NAME_PRIM:
				if (currentStructure==NARRATOR_CONNECTOR) {
					if (structures->narrator!=NULL) {
						addNarrator(structures->narrator);
						structures->narrator=NULL;
					}
				}
				break;
			default:
				assert(false);
			}
		}
		switch(currentStructure) {
		case INITIALIZE: {
			switch(stateInfo.processedStructure) {
			case RASOUL_WORD:
				if (structures->narrator==NULL)
					structures->narrator=new Narrator(structures->text);
				assert(structures->nameConnectorPrim!=NULL);
				structures->narrator->m_narrator.append(structures->nameConnectorPrim);
				structures->nameConnectorPrim=NULL;
				structures->narrator->isRasoul=true;
				addNarrator(structures->narrator);
				structures->narrator=NULL;
				break;
			case NARRATOR_CONNECTOR:
				if (structures->hadith && structures->narratorConnectorPrim!=NULL) {
					delete structures->narratorConnectorPrim;
					structures->narratorConnectorPrim=NULL;
				}
				break;
			}
			break;
		}
		case NAME_PRIM: {
			if (structures->namePrim==NULL)
				structures->namePrim=new NamePrim(structures->text,stateInfo.startPos);
			structures->namePrim->m_end=stateInfo.endPos;
		#ifdef REFINEMENTS
			structures->namePrim->learnedName=stateInfo.learnedName;
		#endif
			if (structures->narrator==NULL)
				structures->narrator=new Narrator(structures->text);
			structures->narrator->m_narrator.append(structures->namePrim);
			structures->namePrim=NULL;
			break;
		}
		case NARRATOR_CONNECTOR: {
			if (structures->hadith) {
				if (structures->narratorConnectorPrim==NULL)
					structures->narratorConnectorPrim=new NarratorConnectorPrim(structures->text,stateInfo.startPos);
				structures->narratorConnectorPrim->m_end=stateInfo.endPos;
			}
			break;
		}
		case NAME_CONNECTOR: {
			if (structures->nameConnectorPrim==NULL)
				structures->nameConnectorPrim=new NameConnectorPrim(structures->text,stateInfo.startPos);
			structures->nameConnectorPrim->m_end=stateInfo.endPos;
		#ifdef REFINEMENTS
			if (stateInfo.familyNMC){
				assert(structures->nameConnectorPrim->isOther());
				structures->nameConnectorPrim->setFamilyConnector();
				if (stateInfo.ibn)
					structures->nameConnectorPrim->setIbn();
				else if (stateInfo._2ab)
					structures->nameConnectorPrim->setAB();
				else if (stateInfo._2om)
					structures->nameConnectorPrim->setOM();
			} else if (stateInfo.possessivePlace) {
				assert(structures->nameConnectorPrim->isOther());
				structures->nameConnectorPrim->setPossessive();
			}
		#endif
			if (stateInfo.isFamilyConnectorOrPossessivePlace()) {
				if (structures->narrator==NULL)
					structures->narrator=new Narrator(structures->text);
				int size=structures->temp_nameConnectors.size();
				for (int i=0;i<size;i++)
					structures->narrator->m_narrator.append(structures->temp_nameConnectors[i]);
				structures->narrator->m_narrator.append(structures->nameConnectorPrim);
				structures->nameConnectorPrim=NULL;
				structures->temp_nameConnectors.clear();
			} else {
				structures->temp_nameConnectors.append(structures->nameConnectorPrim);
				structures->nameConnectorPrim=NULL;
			}
			break;
		}
		case RASOUL_WORD: {
			switch(stateInfo.processedStructure) {
			case NAME_CONNECTOR: {
				//1-finish old narrator and use last nmc as nrc
				if (structures->narrator==NULL)
					structures->narrator=new Narrator(structures->text);
				int size=structures->temp_nameConnectors.size();
				for (int i=0;i<size-1;i++)
					structures->narrator->m_narrator.append(structures->temp_nameConnectors[i]);
				if (size>1) {
					if (structures->narrator->m_narrator.size()>0) {
						structures->chain->m_chain.append(structures->narrator);
						structures->narrator=NULL;
						NameConnectorPrim *n=structures->temp_nameConnectors[size-1];
						if (structures->hadith) {
							structures->narratorConnectorPrim=new NarratorConnectorPrim(structures->text,n->getStart());
							structures->narratorConnectorPrim->m_end=n->getEnd();
							structures->chain->m_chain.append(structures->narratorConnectorPrim);
							structures->narratorConnectorPrim=NULL;
						}
						delete n;
					}
				} else {
					if (structures->narrator->m_narrator.size()>0) {
						addNarrator(structures->narrator);
						structures->narrator=NULL;
					}
				}
				structures->temp_nameConnectors.clear();
				//display(currentChain->narratorConnectorPrim->getString()+"\n");
			}
				//2-create a new narrator of just this stop word as name connector, so we dont insert "break;"
			case NARRATOR_CONNECTOR:
			case NAME_PRIM: {
				assert(structures->nameConnectorPrim==NULL);
				structures->nameConnectorPrim=new NameConnectorPrim(structures->text,stateInfo.startPos); //we added this to previous name bc assumed this will only happen if it is muhamad and "sal3am"
				structures->nameConnectorPrim->m_end=stateInfo.endPos;
				if (structures->narrator!=NULL) {
					if (!structures->narrator->m_narrator.isEmpty()) {
						NarratorPrim * n=structures->narrator->m_narrator.last();
						if (n->isNamePrim() && !((NamePrim *)n)->learnedName) {//if last has not a learned name, we split both narrators. maybe this must happen to all narrators
							addNarrator(structures->narrator);
							structures->narrator=NULL;
						}
					}
				}
				break;
			}
			case RASOUL_WORD: {
				assert(structures->nameConnectorPrim!=NULL);
				structures->nameConnectorPrim->m_end=stateInfo.endPos;
				break;
			}
			default:
				assert(false);
			}
			break;
		}
		default:
			assert(false);
		}
	}
	stateInfo.processedStructure=currentStructure;
#undef addNarrator

#endif
}
inline int removeLastSpuriousNarrators(HadithData *structures) { // returns number of narrators removed
#ifdef CHAIN_BUILDING
	if (structures->hadith) {
		assert(structures->chain!=NULL);
		ChainPrim *c;
		int removed=0;
		while(structures->chain->m_chain.size()>0) {
			c=structures->chain->m_chain.last();
			if (c->isNarrator()) {
				Narrator * n=(Narrator *)c;
				if (n->isRasoul)
					return removed;
				for (int i=0;i<n->m_narrator.size();i++) {
					if (n->m_narrator[i]->isNamePrim())
						return removed;
				}
				structures->chain->m_chain.removeLast();
				removed++;
			}
			else {
				structures->chain->m_chain.removeLast();
			}
		}
		return removed;
	} else
		return 0;
#endif
}

inline void assertStructure(StateInfo & stateInfo,const Structure s) {
#ifdef CHAIN_BUILDING
	assert(stateInfo.processedStructure==s);
#endif
}


bool getNextState(StateInfo &  stateInfo,HadithData *structures, StateData & currentData) {
	display(QString(" nrcsize: %1 ").arg(currentData.nrcCount));
	display(stateInfo.currentState);
	bool ending_punc=false;
#ifdef PUNCTUATION
	if (stateInfo.number) {
		stateInfo.currentPunctuationInfo.has_punctuation=true;
		stateInfo.currentPunctuationInfo.fullstop=true;
		stateInfo.currentPunctuationInfo.newLine=true;
	}
	if (stateInfo.currentPunctuationInfo.has_punctuation) {
		display("<has punctuation>");
		if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
			ending_punc=true;
			display("<ending Punctuation>");
		}
	}
#endif
	int nrc_max= geneologyParameters.nrc_max;
	bool return_value=true;
	switch(stateInfo.currentState) {
	case TEXT_S:
		//assertStructure(stateInfo,INITIALIZE);
		if(stateInfo.currentType==NAME) {
		#ifdef PUNCTUATION
			if ( !stateInfo.previousPunctuationInfo.fullstop) {
				stateInfo.nextState=TEXT_S;
				break;
			}
		#endif

			currentData.initialize();
			stateInfo.nextState=NAME_S;
			currentData.mainStructureStartIndex=stateInfo.startPos;
			currentData.narratorStartIndex=stateInfo.startPos;

			fillStructure(stateInfo,NAME_PRIM,structures,currentData);

		#ifdef PUNCTUATION
			if (stateInfo.currentPunctuationInfo.has_punctuation) {
				display("<punc1>");
				currentData.narratorCount++;
				stateInfo.nextState=NAME_S;
				currentData.nrcCount=0;//punctuation is zero
				currentData.narratorEndIndex=stateInfo.endPos;
				currentData.nrcStartIndex=stateInfo.nextPos;//next_positon(stateInfo.endPos,stateInfo.followedByPunctuation);

				fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,currentData,true);

				if (ending_punc) {
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
			currentData.mainStructureStartIndex=stateInfo.startPos;
			currentData.nrcStartIndex=stateInfo.startPos;
			stateInfo.nextState=NRC_S;
			currentData.nrcCount=1;

			fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,currentData);
		} else {
			stateInfo.nextState=TEXT_S;
		}
		break;

	case NAME_S:
		//assertStructure(stateInfo,NAME_PRIM);
		if (stateInfo.currentType==NRC) {
			stateInfo.nextState=NAME_S;
			//display(QString("counter%1\n").arg(currentData.narratorCount));
			currentData.nrcCount++;
			currentData.narratorEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
			currentData.nrcStartIndex=stateInfo.startPos;

			fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,currentData);

		#ifdef PUNCTUATION
			if (ending_punc) {
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

			fillStructure(stateInfo,NAME_PRIM,structures,currentData);

		#ifdef PUNCTUATION
			if (stateInfo.currentPunctuationInfo.has_punctuation) {
				display("<punc2>");
				currentData.narratorCount++;
				stateInfo.nextState=NRC_S;
				currentData.nrcCount=0; //punctuation not counted
				currentData.narratorEndIndex=stateInfo.endPos;
				currentData.nrcStartIndex=stateInfo.nextPos;//next_positon(stateInfo.endPos,stateInfo.followedByPunctuation);

				fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,currentData,true);

				if (ending_punc) {
					currentData.narratorEndIndex=stateInfo.endPos;
					stateInfo.nextState=TEXT_S;
					return_value=false;
				}
			}
		#endif
		}
		break;

	case NMC_S:
		assertStructure(stateInfo,NAME_CONNECTOR);
	#ifdef REFINEMENTS
		if(reachedRasoul) {
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

			//2-create a new narrator of just this stop word as name connector
			currentData.narratorEndIndex=stateInfo.endPos;
			stateInfo.nextState=STOP_WORD_S;
			currentData.narratorCount++;

			fillStructure(stateInfo,RASOUL_WORD,structures,currentData);

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

			fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,currentData);

		#ifdef REFINEMENTS
			if (stateInfo._3an) {
				currentData.nrcCount=1;
				currentData.nrcEndIndex=stateInfo.endPos;

				assertStructure(stateInfo,NARRATOR_CONNECTOR);
				fillStructure(stateInfo,NAME_PRIM,structures,currentData,true);

				stateInfo.nextState=NAME_S;
			}
		#endif
		#ifdef PUNCTUATION
			if (ending_punc) {
				currentData.narratorEndIndex=stateInfo.endPos;
				stateInfo.nextState=TEXT_S;
				return_value=false;
			}
		#endif
		}
		else if(stateInfo.currentType==NAME) {
			currentData.nmcCount=0;
			stateInfo.nextState=NAME_S;

			fillStructure(stateInfo,NAME_PRIM,structures,currentData);

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
				fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,currentData,true);

				if (ending_punc) {
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
			currentData.nmcCount=nmc_max+1;
			currentData.nmcValid=false;

			fillStructure(stateInfo,NAME_CONNECTOR,structures,currentData);

			if (ending_punc) {
				stateInfo.nextState=TEXT_S;

				fillStructure(stateInfo,INITIALIZE,structures,currentData,true,true);

				currentData.narratorEndIndex=stateInfo.lastEndPos;//TODO: find a better representation
				currentData.narratorCount++;
				return_value=false;
				break;
			}
		}
	#endif
		else if (currentData.nmcCount>nmc_max
				#ifdef PUNCTUATION
					 || stateInfo.number ||
					 (ending_punc)
				#endif
			) {
			if (currentData.nmcCount>nmc_max && currentData.nmcValid) {//number is severe condition no tolerance
				currentData.nmcValid=false;
				stateInfo.nextState=NMC_S;
				currentData.nmcCount=0;

				fillStructure(stateInfo,NAME_CONNECTOR,structures,currentData);

			} else {
				if (structures->hadith) {
					stateInfo.nextState=TEXT_S;
					fillStructure(stateInfo,INITIALIZE,structures,currentData,true,true);
					return_value= false;
				} else {
					stateInfo.nextState=NRC_S;
					fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,currentData,true,true);
				}

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
				currentData.narratorEndIndex=stateInfo.lastEndPos;//TODO: find a better representation
				break;
			}
			//currentData.narratorEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(start_index); check this case

		} else  { //NMC

			fillStructure(stateInfo,NAME_CONNECTOR,structures,currentData);

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
			if (ending_punc) {
				stateInfo.nextState=TEXT_S;

				//TODO: check why in previous implementation we added the temp_nameConnectors here but not in previous ending_punc
				fillStructure(stateInfo,INITIALIZE,structures,currentData,true,true);

				currentData.narratorEndIndex=stateInfo.endPos;
				currentData.narratorCount++;
				return_value=false;
				break;
			}
		#endif
		}
		break;

	case NRC_S:
		assertStructure(stateInfo,NARRATOR_CONNECTOR);
	#ifdef REFINEMENTS
		if(reachedRasoul) {
			display("<STOP3>");

			fillStructure(stateInfo,RASOUL_WORD,structures,currentData);

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

			stateInfo.nextState=STOP_WORD_S;
			currentData.narratorCount++;

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
		if(structures->hadith && (stateInfo.currentType==NAME || stateInfo.currentType ==NRC))
			currentData.nrcPunctuation=false;
	#endif
	#ifdef REFINEMENTS
		if (stateInfo.currentType==NAME || stateInfo.possessivePlace) {
	#else
		if (stateInfo.currentType==NAME) {
	#endif
			stateInfo.nextState=NAME_S;
			currentData.nrcCount=1;

			currentData.narratorStartIndex=stateInfo.startPos;
			if (stateInfo.currentType==NAME)
				currentData.nameStartIndex=stateInfo.startPos;
			else
				currentData.nmcStartIndex=stateInfo.startPos;

			fillStructure(stateInfo,(stateInfo.currentType==NAME?NAME_PRIM:NAME_CONNECTOR),structures,currentData);
			stateInfo.processedStructure=NAME_PRIM; //to have consistency with nextState in case it was NAME_CONNECTOR

			currentData.nrcEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
		#ifdef STATS
			temp_names_per_narrator++;//found another name
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
				fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,currentData,true);

				if (ending_punc) {
					currentData.narratorEndIndex=stateInfo.endPos;
					stateInfo.nextState=TEXT_S;
					return_value=false;
					break;
				}
				break;
			}
		#endif
		}
	#ifdef IBN_START
		else if (stateInfo.currentType==NMC && stateInfo.familyNMC) {
			display("<Family3>");

			fillStructure(stateInfo,NAME_CONNECTOR,structures,currentData);

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

				fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,currentData,true);

				if (ending_punc) {
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
	#ifdef PUNCTUATION
		else if ((structures->hadith?currentData.nrcCount:currentData.bio_nrcCount)>=nrc_max || currentData.nrcPunctuation ||stateInfo.number) {
			//if in biography mode bio_nrcCount has meaning else is zero
			//if not in refinements mode stateInfo.number will always remain false
	#else
		else if (currentData.nrcCount>=nrc_max|| currentData.bio_nrcCount>=nrc_max) {
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
			fillStructure(stateInfo,INITIALIZE,structures,currentData,true,true); //just to delete dangling NARRATOR_CONNECTOR

			return_value= false;
			break;
		}
		else {
			stateInfo.nextState=NRC_S;

			fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,currentData);

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
			if (ending_punc) {
				stateInfo.nextState=TEXT_S;

				fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,currentData,true,true);
				stateInfo.processedStructure=INITIALIZE; //TODO: check if correct

				currentData.narratorEndIndex=stateInfo.endPos;
				return_value=false;
				break;
			}
		#endif
		#ifdef REFINEMENTS
			if (stateInfo._3an) {
				currentData.nrcCount=1;
				currentData.nrcEndIndex=stateInfo.endPos;

				assertStructure(stateInfo,NARRATOR_CONNECTOR);
				fillStructure(stateInfo,NAME_PRIM,structures,currentData,true);

				stateInfo.nextState=NAME_S;
				break;
			}
		#endif
		}
	#ifdef PUNCTUATION
		if (structures->hadith && stateInfo.currentPunctuationInfo.has_punctuation && stateInfo.nextState==NRC_S && stateInfo.currentType!=NAME && stateInfo.currentType!=NRC)
			currentData.nrcPunctuation=true;
	#endif
		break;
#ifdef REFINEMENTS
	case STOP_WORD_S:
		assertStructure(stateInfo,RASOUL_WORD);
		if (stateInfo.currentType==STOP_WORD) {
			stateInfo.nextState=STOP_WORD_S;

			fillStructure(stateInfo,RASOUL_WORD,structures,currentData);

			currentData.narratorEndIndex=stateInfo.endPos;
		#ifdef PUNCTUATION
			if (ending_punc) {

				fillStructure(stateInfo,INITIALIZE,structures,currentData);
				stateInfo.nextState=TEXT_S;
				return_value=false;
				break;
			}
		#endif
		} else {

			if (structures->hadith) {
				fillStructure(stateInfo,INITIALIZE,structures,currentData);
				stateInfo.nextState=TEXT_S;
				return_value=false;
			} else {
				fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,currentData);
				stateInfo.nextState=NRC_S;
			}
		}
		break;
#endif
	default:
		break;
	}
	display("\n");
#ifdef REFINEMENTS
	currentData.ibn_or_3abid=stateInfo.familyConnectorOr3abid(); //for it to be saved for next time use
#endif
	if (!return_value /*&& stateInfo.processedStructure!=INITIALIZE*/) {
		fillStructure(stateInfo,INITIALIZE,structures,currentData);
		currentData.narratorCount-=removeLastSpuriousNarrators(structures);
	} else if (return_value==false) {
		assert (structures->narrator==NULL);
	}
	return return_value;
}
inline bool result(WordType t, StateInfo &  stateInfo,HadithData *currentChain, StateData & currentData){display(t); stateInfo.currentType=t; return getNextState(stateInfo,currentChain,currentData);}

bool proceedInStateMachine(StateInfo &  stateInfo,HadithData *structures, StateData & currentData ) {//does not fill stateInfo.currType
	hadith_stemmer s(structures->text,stateInfo.startPos);
	stateInfo.resetCurrentWordInfo();
	long  finish;

#ifdef PUNCTUATION
	if (isNumber(structures->text,stateInfo.startPos,finish)) {
		display("Number ");
		stateInfo.number=true;
		stateInfo.endPos=finish;
		stateInfo.nextPos=next_positon(structures->text,finish+1,stateInfo.currentPunctuationInfo);
		display(structures->text->mid(stateInfo.startPos,finish-stateInfo.startPos+1)+":");
		return result(NMC,stateInfo,structures,currentData);
	}
#endif
	s();
	finish=max(s.info.finish,s.finish_pos);
	if (finish==stateInfo.startPos) {
		finish=getLastLetter_IN_currentWord(structures->text,stateInfo.startPos);
	}
	stateInfo.endPos=finish;
	stateInfo.nextPos=next_positon(structures->text,finish,stateInfo.currentPunctuationInfo);
	display(structures->text->mid(stateInfo.startPos,finish-stateInfo.startPos+1)+":");
	if (s.name){
	#ifdef GET_WAW
		long nextpos=stateInfo.nextPos;
		PunctuationInfo copyPunc=stateInfo.currentPunctuationInfo;
		if (s.has_waw && stateInfo.currentState==NAME_S ||stateInfo.currentState==SONS_S ) {
			display("waw ");
			stateInfo.isWaw=true;
			stateInfo.startPos=s.wawStart;
			stateInfo.endPos=s.wawEnd;
			stateInfo.nextPos=s.wawEnd+1;
			stateInfo.currentPunctuationInfo.reset();
			if (!result(NRC,stateInfo,structures,currentData))
				return false;
			stateInfo.isWaw=false;
			stateInfo.currentState=stateInfo.nextState;
			stateInfo.lastEndPos=stateInfo.endPos;
		}
		stateInfo.currentPunctuationInfo=copyPunc;
	#endif
		stateInfo.startPos=s.startStem;
		stateInfo.endPos=s.finishStem;
		stateInfo.nextPos=nextpos;
		return result(NAME,stateInfo,structures,currentData);
	}
	else
		return result(NMC,stateInfo,structures,currentData);
}



class GeneologySegmentor {
private:

	QString fileName;
	StateData currentData;
	QString * text;
	long current_pos;

	int segmentHelper(QString * text,int (*functionUsingChains)(ChainsContainer &, ATMProgressIFC *, QString),ATMProgressIFC *prg) {
		QFile chainOutput(chainDataStreamFileName);

		chainOutput.remove();
		if (!chainOutput.open(QIODevice::ReadWrite))
			return 1;
		QDataStream chainOut(&chainOutput);
		if (text==NULL)
			return -1;
		long text_size=text->size();
		long  newHadithStart=-1;
		currentData.initialize();

	#ifdef CHAIN_BUILDING
		HadithData *currentChain=new HadithData(text,true,NULL);
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
		stateInfo.processedStructure=INITIALIZE;
	#ifdef PUNCTUATION
		stateInfo.previousPunctuationInfo.fullstop=true;
	#endif
		while(stateInfo.startPos<text->length() && isDelimiter(text->at(stateInfo.startPos)))
			stateInfo.startPos++;
	#ifdef PROGRESSBAR
		prg->setCurrentAction("Parsing Hadith");
	#endif
		for (;stateInfo.startPos<text_size;) {
			if((proceedInStateMachine(stateInfo,currentChain,currentData)==false)) {
				assert (currentChain->narrator==NULL);
				if (currentData.narratorCount>=geneologyParameters.narr_min) {
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
	#ifdef PROGRESSBAR
			prg->report((double)stateInfo.startPos/text_size*100+0.5);
			if (stateInfo.startPos==text_size-1)
				break;
	#endif
		}
		prg->report(100);
	#if  defined(DISPLAY_HADITH_OVERVIEW)
		if (newHadithStart<0)	{
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
		while (!tester.atEnd()) {
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

int geneologyHelper(QString input_str,ATMProgressIFC *prgs){
	input_str=input_str.split("\n")[0];
	GeneologySegmentor s;
	s.segment(input_str,&test_GraphFunctionalities,prg);
	return 0;
}
#if 0
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

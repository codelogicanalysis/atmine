#ifndef HADITHCOMMON_H
#define HADITHCOMMON_H

#include <assert.h>
#include <QFile>
#include <QRegExp>
#include <QString>
#include <QStringList>

#include "hadith.h"
#include "stemmer.h"
#include "letters.h"
#include "diacritics.h"
#include "text_handling.h"
#include "textParsing.h"
#include "common.h"
#include "Retrieve_Template.h"
#include "Search_by_item_locally.h"
#include "narrator_abstraction.h"



	class HadithParameters
	{
	public:
		unsigned int nmc_max:5;
		unsigned int nrc_max:5;
		unsigned int narr_min:5;
		unsigned int equality_radius:5;
		bool display_chain_num:1;
		bool break_cycles:1;
		unsigned int bio_nmc_max:5;
		unsigned int bio_nrc_max:10;
		unsigned int bio_narr_min:5;
		unsigned int bio_max_reachability:5;
		unsigned int unused_int:17;
		double equality_delta;
		double equality_threshold;
		HadithParameters()
		{
			nmc_max=3;
			narr_min=3;
			nrc_max=5;
			bio_nmc_max=1;
			bio_narr_min=4;
			bio_nrc_max=100;
			equality_radius=3;
			equality_delta=0.4;
			equality_threshold=1;
			bio_max_reachability=0;
			display_chain_num=true;
			break_cycles=true;
		}
	};

	extern HadithParameters hadithParameters;
	extern QString chainDataStreamFileName;

	extern QString chainDataStreamFileName;
#ifdef PREPROCESS_DESCRIPTIONS
	extern QString preProcessedDescriptionsFileName;
#endif
	extern QString PhrasesFileName;
	extern QString StopwordsFileName;

#ifndef REFINEMENTS
	enum WordType { NAME, NRC,NMC};
	enum StateType { TEXT_S , NAME_S, NMC_S , NRC_S};
#else
	#define GET_WAW

	enum WordType { NAME, NRC,NMC,STOP_WORD};
	enum StateType { TEXT_S , NAME_S, NMC_S , NRC_S, STOP_WORD_S};
	enum Structure { INITIALIZE, NAME_CONNECTOR, NARRATOR_CONNECTOR, NAME_PRIM, RASOUL_WORD, UNDETERMINED_STRUCTURE};
#endif

#ifdef COUNT_RUNON
	extern bool runon;
#endif

	typedef struct  {
		long lastEndPos;
		long startPos;
		long endPos;
		long nextPos;
		Structure processedStructure:3;
		WordType currentType:3;
		StateType currentState:4;
		StateType nextState:4;
		bool familyNMC:1;
		bool ibn:1;
		bool _2ab:1;
		bool _2om:1;
		bool _3abid:1;
		bool possessivePlace:1;
		bool number:1;
		bool isWaw:1;
		bool _3an:1;
		bool learnedName:1;
		bool nrcIsPunctuation:1; //if state is NRC and that is caused soley by punctuation
		int unused:12;
		PunctuationInfo previousPunctuationInfo,currentPunctuationInfo;
		void resetCurrentWordInfo()	{familyNMC=false;ibn=false;_2ab=false;_2om=false;_3abid=false;possessivePlace=false;number=false;isWaw=false;_3an=false;learnedName=false;currentPunctuationInfo.reset();}
		bool familyConnectorOr3abid() { return familyNMC || _3abid;}
		bool isFamilyConnectorOrPossessivePlace(){return familyNMC || possessivePlace;}
	} StateInfo;

#ifdef REFINEMENTS
	extern QStringList compound_words, rasoul_words,suffixNames;
	extern QString alrasoul,abyi;
#endif
	extern QString hadath,abid,alrasoul,abyi,_3an,_2ama,_3ama;
	extern int bit_POSSESSIVE, bit_PLACE,bit_CITY,bit_COUNTRY,bit_NOUN_PROP;
	extern QList<int> bits_NAME;

#ifdef PREPROCESS_DESCRIPTIONS
	extern QHash<long,bool> familyNMC_descriptions;
	extern QHash<long,bool> NRC_descriptions;
	extern QHash<long,bool> IBN_descriptions;
	extern QHash<long,bool> AB_descriptions;
	extern QHash<long,bool> OM_descriptions;

#endif
#ifdef COMPARE_TO_BUCKWALTER
	extern QTextStream * myoutPtr;
#endif
#ifdef COUNT_AVERAGE_SOLUTIONS
	extern long total_solutions;
	extern long stemmings;
#endif

	typedef QList<NameConnectorPrim *> TempConnectorPrimList;

#define display_letters 30

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
#endif


#ifdef PREPROCESS_DESCRIPTIONS
	void readFromDatabasePreProcessedDescriptions();
	void readFromFilePreprocessedDescriptions();
#endif

	void hadith_initialize();

	class hadith_stemmer: public Stemmer { //TODO: seperate ibn from possessive from 3abid and later seperate between ibn and bin
	private:
		bool place;
	#ifdef TEST_WITHOUT_SKIPPING
		bool finished;
	#endif
	public:
		long finish_pos;
		bool name:1, nrc:1, nmc,possessive:1, familyNMC:1,ibn:1,_2ab:1,_2om:1,_3abid:1,stopword:1;
	#ifdef GET_WAW
		bool has_waw:1,is3an:1;
	#endif
	#ifdef REFINEMENTS
		bool tryToLearnNames:1,learnedName:1;
		long startStem, finishStem,wawStart,wawEnd;
	#endif
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
			familyNMC=false;
			ibn=false;
			_2ab=false;
			_2om=false;
			_3abid=false;
			stopword=false;
			finish_pos=start;
		#ifdef REFINEMENTS
			tryToLearnNames=false;
			learnedName=false;
			is3an=false;
			startStem=start;
			finishStem=start;
		#endif
		#ifdef GET_WAW
			wawStart=start;
			wawEnd=start;
			has_waw=false;
		#endif
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
					prefix_infos=&Prefix->affix_info;
					solution_position * s_inf=Suffix->computeFirstSolution();
					do
					{
						suffix_infos=&Suffix->affix_info;
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

		bool analyze()
		{
		#ifdef STATS
			QString temp_stem=removeDiacritics(info.text->mid(Stem->starting_pos, Suffix->info.start-Stem->starting_pos));//removeDiacritics(stem_info->raw_data);
		#endif
		#ifndef PREPROCESS_DESCRIPTIONS
			QString description=stem_info->description();
		#endif
		#ifdef REFINEMENTS
			_3abid= (equal_ignore_diacritics(stem_info->raw_data,abid));
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
			else if (familyNMC_descriptions.contains(stem_info->description_id) &&
					#ifdef REFINEMENTS
					 !equal_ignore_diacritics(stem_info->raw_data,_2ama) && !equal_ignore_diacritics(stem_info->raw_data,_3ama)
					#endif
				)
		#endif
			{
			#ifdef STATS
				stem=temp_stem;
			#endif
			#ifdef REFINEMENTS
				familyNMC=true;
				if (IBN_descriptions.contains(stem_info->description_id))
					ibn=true;
				if (AB_descriptions.contains(stem_info->description_id))
					_2ab=true;
				if (OM_descriptions.contains(stem_info->description_id))
					_2om=true;
				finishStem=Stem->info.finish;
				startStem=Stem->info.start;
			#endif
			#ifdef GET_WAW
				checkForWaw();
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
			#ifdef REFINEMENTS
				if (equal_ignore_diacritics(stem_info->raw_data,_3an)) {
					is3an=true;
					startStem=Stem->info.start;
					finishStem=Stem->info.finish;
				}
			#endif
				nrc=true;
				finish_pos=info.finish;
				return false;
			}
			if (info.finish>info.start) { //more than one letter to be tested for being a name
			#ifdef REFINEMENTS
				if (tryToLearnNames)
					bits_NAME.append(bit_NOUN_PROP);
			#endif
				int bitsNamesSize=bits_NAME.size();
				for (int i=0;i<bitsNamesSize;i++) {
					if (stem_info->abstract_categories.getBit(bits_NAME[i])
						#ifdef REFINEMENTS
							&& Suffix->info.finish-Suffix->info.start<0 && Stem->info.finish>Stem->info.start) //i.e. has no suffix and stem > a letter
						#else
							)
						#endif
					{
					#ifdef REFINEMENTS
						if (removeDiacritics(Stem->info.getString()).count()<3) //bit_NOUN_PROP==bits_NAME[i] &&
							continue;
					#endif
						name=true;
						if (info.finish>finish_pos)
						{
							finish_pos=info.finish;
						#ifdef REFINEMENTS
							if (bit_NOUN_PROP==bits_NAME[i])
								learnedName=true;
							else
								learnedName=false;
							finishStem=Stem->info.finish;
							startStem=Stem->info.start;
						#ifdef COUNT_RUNON
							runon=false;
							if (equal(Stem->info.getString(),stem_info->raw_data)) {
								if (runon)
									error<<Stem->info.getString()<<"-"<<stem_info->raw_data<<"\n";
							}
						#endif
						#endif
						#ifdef GET_WAW
							checkForWaw();
						#endif
						#ifdef STATS
							stem=temp_stem;
						#endif
						}
					#ifdef REFINEMENTS
						else if (info.finish==finish_pos && bit_NOUN_PROP!=bits_NAME[i]){
							learnedName=false;
						}
						if (tryToLearnNames)
							bits_NAME.removeLast();
					#endif
						return true;
					}
				}
			#ifdef REFINEMENTS
				if (tryToLearnNames)
					bits_NAME.removeLast();
			#endif
			}
	#ifndef JUST_BUCKWALTER
	#if 1
			if (stem_info->abstract_categories.getBit(bit_POSSESSIVE) && stem_info->abstract_categories.getBit(bit_PLACE))
			{
				possessive=true;
				place=true;
			#ifdef STATS
				stem=temp_stem;
			#endif
				nmc=true;
				finish_pos=info.finish;
				return false;
			}
		#ifdef REFINEMENTS
			QString c;
			foreach(c,rasoul_words) {
				int i1=0,i2=Stem->info.start;
				if (checkIfSmallestIsPrefixOfLargest(c.rightRef(-1),info.text->midRef(i2),i1,i2) && i1==c.size()-1) {
					int pos=i2+Stem->info.start;
					if (pos+1==info.text->size() || isDelimiter(info.text->at(pos+1))) {
						finish_pos=pos;
						stopword=true;
						return false;
					}
				}
			}
		#endif
	#else
			if (stem_info->abstract_categories.getBit(bit_POSSESSIVE) )
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

	inline QString choose_stem(QList<QString> stems) { //rule can be modified later
		if (stems.size()==0)
			return "";
		QString result=stems[0];
		for (int i=1;i<stems.size();i++)
			if (result.length()>stems[i].length())
				result=stems[i];
		return result;
	}

#ifdef STATS
	inline void show_according_to_frequency(QList<int> freq,QList<QString> words) {
		QList<QPair<int, QString> > l;
		for (int i=0;i<freq.size() && i<words.size();i++)
			l.append(QPair<int,QString>(freq[i],words[i]));
		qSort(l.begin(),l.end());
		for (int i=l.size()-1;i>=0;i--)
			displayed_error <<"("<<l[i].first<<") "<<l[i].second<<"\n";
	}
#endif

	inline bool isRelativeNarrator(const Narrator & n) { //needed in equality and narrator Hash
		QString n_str=n.getString();
		if (equal(n_str,abyi))
			return true;
		for (int i=1;i<n.m_narrator.size();i++) {
			QString s=n.m_narrator[i]->getString();
			for (int j=0;j<suffixNames.size();j++) {
				if (equal_ignore_diacritics(s,suffixNames[j]))
					return true;
			}
		}
		return false;
	}

#if 0
	typedef struct stateData_ {
		long  mainStructureStartIndex;
		long narratorCount,narratorStartIndex,narratorEndIndex,nrcStartIndex,nrcEndIndex;
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
			nmcValid=false;
			ibn_or_3abid=false;
			nameStartIndex=0;
			nmcStartIndex=0;
			mainStructureStartIndex=0;
			nrcStartIndex=0;
			nrcEndIndex=0;
		#ifdef PUNCTUATION
			nrcPunctuation=false;
		#endif
		}

	} stateData;
	class HadithData {
	public:
		QString * text;
		bool hadith;
		NamePrim *namePrim;
		NameConnectorPrim *nameConnectorPrim;
		NarratorConnectorPrim *narratorConnectorPrim;
		TempConnectorPrimList temp_nameConnectors;
		Narrator *narrator;
		Chain *chain;
		Biography *biography;

		void initialize(NarratorGraph *graph,QString * text) {
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
		HadithData(QString * text, bool hadith){
			this->text=text;
			this->hadith=hadith;
			namePrim=NULL;
			nameConnectorPrim=NULL;
			narrator=NULL;
			narratorConnectorPrim=NULL;
			biography=NULL;
			chain=NULL;
		}
	};

	inline void fillStructure(StateInfo &  stateInfo,const Structure & currentStructure,HadithData *structures,bool punc=false,bool ending_punc=false) {
	#ifdef CHAIN_BUILDING

	#define addNarrator(narrator) \\
		if (structures->hadith) \\
			structures->chain->m_chain.append(narrator); \\
		else \\
			structures->biography->addNarrator(narrator);


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
				switch(stateInfo.processedStructure) {
				case INITIALIZE:
					structures->initialize(text);
					display(QString("\ninit%1\n").arg(structures->chain->m_chain.size()));
					assert(currentStructure!=INITIALIZE); //must not happen
					break;
				case NARRATOR_CONNECTOR:
					if (structures->hadith && currentStructure!=NARRATOR_CONNECTOR && currentStructure!=INITIALIZE) {
						if(structures->narratorConnectorPrim==NULL) {
							structures->narratorConnectorPrim=new NarratorConnectorPrim(text,stateInfo.lastEndPos+1);
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
							structures->narrator=new Narrator(text);
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
							fillStructure(stateInfo,INITIALIZE,structures,punc,ending_punc);
							stateInfo.processedStructure=NARRATOR_CONNECTOR;
							return;
						} else
							assert(currentStructure==INITIALIZE);
					}
					break;
				case NAME_PRIM:
					if (currentStructure==NARRATOR_CONNECTOR) {
						addNarrator(structures->narrator);
						structures->narrator=NULL;
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
						structures->narrator=new Narrator(text);
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
					structures->namePrim=new NamePrim(text,stateInfo.startPos);
				structures->namePrim->m_end=stateInfo.endPos;
			#ifdef REFINEMENTS
				structures->namePrim->learnedName=stateInfo.learnedName;
			#endif
				if (structures->narrator==NULL)
					structures->narrator=new Narrator(text);
				structures->narrator->m_narrator.append(structures->namePrim);
				structures->namePrim=NULL;
				break;
			}
			case NARRATOR_CONNECTOR: {
				if (structures->hadith) {
					if (structures->narratorConnectorPrim==NULL)
						structures->narratorConnectorPrim=new NarratorConnectorPrim(text,stateInfo.startPos);
					structures->narratorConnectorPrim->m_end=stateInfo.endPos;
				}
				break;
			}
			case NAME_CONNECTOR: {
				if (structures->nameConnectorPrim==NULL)
					structures->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
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
					//TODO: learn that word that comes after is a name
				} else if (stateInfo.possessivePlace) {
					assert(structures->nameConnectorPrim->isOther());
					structures->nameConnectorPrim->setPossessive();
				}
			#endif
				if (stateInfo.isFamilyConnectorOrPossessivePlace()) {
					if (structures->narrator==NULL)
						structures->narrator=new Narrator(text);
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
						structures->narrator=new Narrator(text);
					int size=structures->temp_nameConnectors.size();
					for (int i=0;i<size-1;i++)
						structures->narrator->m_narrator.append(structures->temp_nameConnectors[i]);
					if (size>1) {
						if (structures->narrator->m_narrator.size()>0) {
							structures->chain->m_chain.append(structures->narrator);
							structures->narrator=NULL;
							NameConnectorPrim *n=structures->temp_nameConnectors[size-1];
							if (structures->hadith) {
								structures->narratorConnectorPrim=new NarratorConnectorPrim(text,n->getStart());
								structures->narratorConnectorPrim->m_end=n->getEnd();
								addNarrator(structures->narratorConnectorPrim);
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
					structures->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos); //we added this to previous name bc assumed this will only happen if it is muhamad and "sal3am"
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
	inline void assertStructure(StateInfo & stateInfo,const Structure s) {
	#ifdef CHAIN_BUILDING
		assert(stateInfo.processedStructure==s);
	#endif
	}

	bool getNextState(StateInfo &  stateInfo,HadithData *structures, stateData & currentData) {
		display(QString(" nmcsize: %1 ").arg(currentData.nmcCount));
		display(QString(" nrcsize: %1 ").arg(currentData.nrcCount));
		display(stateInfo.currentState);
		bool ending_punc=false;
	#ifdef PUNCTUATION
		if (stateInfo.currentPunctuationInfo.has_punctuation) {
			display("<has punctuation>");
			if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
				ending_punc=true;
				display("<ending Punctuation>");
			}
		}
		if (structures->hadith) {
			if (stateInfo.number) {
				stateInfo.currentPunctuationInfo.has_punctuation=true;
				stateInfo.currentPunctuationInfo.fullstop=true;
				stateInfo.currentPunctuationInfo.newLine=true;
			}
		} else {
			stateInfo.number=false;//numbers may appear in many places in biography
		}

	#endif
	#ifdef TRYTOLEARN
		stateInfo.nrcIsPunctuation=false;
	#endif
	#ifdef REFINEMENTS
		bool reachedRasoul= (stateInfo.currentType== STOP_WORD && !stateInfo.familyConnectorOr3abid());//stop_word not preceeded by 3abid or ibn
		if (stateInfo.currentType== STOP_WORD && !reachedRasoul)
			stateInfo.currentType=NAME;
	#endif
		bool return_value=true;
		switch(stateInfo.currentState)
		{
		case TEXT_S:
			assertStructure(stateInfo,INITIALIZE);
			if(stateInfo.currentType==NAME) {
			#ifdef PUNCTUATION
				if (structures->hadith && !stateInfo.previousPunctuationInfo.fullstop) {
					stateInfo.nextState=TEXT_S;
					break;
				}
			#endif

				currentData.initialize();
				stateInfo.nextState=NAME_S;
				currentData.sanadStartIndex=stateInfo.startPos;
				currentData.narratorStartIndex=stateInfo.startPos;

				fillStructure(stateInfo,NAME_PRIM,structures);

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

					fillStructure(stateInfo,NAME_CONNECTOR,structures,true);

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
				currentData.sanadStartIndex=stateInfo.startPos;
				currentData.nrcStartIndex=stateInfo.startPos;
				stateInfo.nextState=NRC_S;
				currentData.nrcCount=1;

				fillStructure(stateInfo,NARRATOR_CONNECTOR,structures);

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

					assertStructure(stateInfo,NARRATOR_CONNECTOR);
					fillStructure(stateInfo,NAME_PRIM,structures,true);

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
	#ifdef IBN_START//needed in case a hadith starts by familyConnector such as "ibn yousef qal..."
			else if (stateInfo.currentType==NMC && stateInfo.familyNMC) {
			#ifdef PUNCTUATION
				if (!stateInfo.previousPunctuationInfo.fullstop) {
					stateInfo.nextState=TEXT_S;
					break;
				}
			#endif
				display("<Family1>");
				currentData.initialize();
				currentData.sanadStartIndex=stateInfo.startPos;
				currentData.nmcStartIndex=stateInfo.startPos;
				currentData.narratorStartIndex=stateInfo.startPos;
				currentData.nmcCount=1;
				stateInfo.nextState=NMC_S;
				currentData.nmcValid=true;

				fillStructure(stateInfo,NAME_CONNECTOR,structures);

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
				if (ending_punc) {
					stateInfo.nextState=TEXT_S;

					fillStructure(stateInfo,INITIALIZE,structures,true,true); //futureStructure=INITIALIZE will reset the structure for next time and will not hurt flow since anyways resetting here

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
			assertStructure(stateInfo,NAME_PRIM);
		#ifdef REFINEMENTS
			if(reachedRasoul)
			{
				display("<STOP1>");
				stateInfo.nextState=STOP_WORD_S;
				currentData.narratorCount++;

				fillStructure(stateInfo,RASOUL_WORD,structures);

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

				//check why in previous implementation, were clearing currentChain->temp_nameConnectors without adding them in even if NMC was ibn or possessive
				fillStructure(stateInfo,NAME_CONNECTOR,structures);

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
					if (ending_punc) {
						stateInfo.nextState=TEXT_S;

						fillStructure(stateInfo,INITIALIZE,structures,true,true);

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

				fillStructure(stateInfo,NARRATOR_CONNECTOR,structures);

			#ifdef REFINEMENTS
				if (stateInfo._3an) {
					currentData.nrcCount=1;
					currentData.nrcEndIndex=stateInfo.endPos;

					assertStructure(stateInfo,NARRATOR_CONNECTOR);
					fillStructure(stateInfo,NAME_PRIM,structures,true);

					stateInfo.nextState=NAME_S;
				}
			#endif
			#ifdef PUNCTUATION
			#if 0
				if (stateInfo.punctuationInfo.has_punctuation)
					currentData.nrcCount=parameters.nrc_max;
			#endif
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

				fillStructure(stateInfo,NAME_PRIM,structures);

			#ifdef PUNCTUATION
				if (stateInfo.currentPunctuationInfo.has_punctuation) {
					display("<punc2>");
					currentData.narratorCount++;
					stateInfo.nextState=NRC_S;
					currentData.nrcCount=0; //punctuation not counted
					currentData.narratorEndIndex=stateInfo.endPos;
					currentData.nrcStartIndex=stateInfo.nextPos;//next_positon(stateInfo.endPos,stateInfo.followedByPunctuation);

					fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,true);

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

				fillStructure(stateInfo,RASOUL_WORD,structures);

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

				fillStructure(stateInfo,NARRATOR_CONNECTOR,structures);

			#ifdef REFINEMENTS
				if (stateInfo._3an) {
					currentData.nrcCount=1;
					currentData.nrcEndIndex=stateInfo.endPos;

					assertStructure(stateInfo,NARRATOR_CONNECTOR);
					fillStructure(stateInfo,NAME_PRIM,structures,true);

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

				fillStructure(stateInfo,NAME_PRIM,structures);

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
					fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,true);

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
				currentData.nmcCount=hadithParameters.nmc_max+1;
				currentData.nmcValid=false;

				fillStructure(stateInfo,NAME_CONNECTOR,structures);

				if (ending_punc) {
					stateInfo.nextState=TEXT_S;

					fillStructure(stateInfo,INITIALIZE,structures,true,true);

					currentData.narratorEndIndex=stateInfo.lastEndPos;//TODO: find a better representation
					currentData.narratorCount++;
					return_value=false;
					break;
				}
			}
		#endif
			else if (currentData.nmcCount>hadithParameters.nmc_max
					#ifdef PUNCTUATION
						 || stateInfo.number ||
						 (ending_punc)
					#endif
				) {
				if (currentData.nmcCount>hadithParameters.nmc_max && currentData.nmcValid) {//number is severe condition no tolerance
					currentData.nmcValid=false;
					stateInfo.nextState=NMC_S;
					currentData.nmcCount=0;

					fillStructure(stateInfo,NAME_CONNECTOR,structures);

				} else {
					if (structures->hadith) {
						stateInfo.nextState=TEXT_S;
						fillStructure(stateInfo,INITIALIZE,structures,true,true);
						return_value= false;
					} else {
						stateInfo.nextState=NRC_S;
						fillStructure(stateInfo,NARRATOR_CONNECTOR,currentChain,true,true);
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

				fillStructure(stateInfo,NAME_CONNECTOR,structures);

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
					fillStructure(stateInfo,INITIALIZE,structures,true,true);

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

				fillStructure(stateInfo,RASOUL_WORD,structures);

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
			if(structures->hadith && stateInfo.currentType==NAME || stateInfo.currentType ==NRC)
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

				fillStructure(stateInfo,(stateInfo.currentType==NAME?NAME_PRIM:NAME_CONNECTOR),structures);
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
					fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,true);

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
				fillStructure(stateInfo,INITIALIZE,structures,true,true); //just to delete dangling NARRATOR_CONNECTOR

				return_value= false;
				break;
			}
		#ifdef IBN_START
			else if (stateInfo.currentType==NMC && stateInfo.familyNMC) {
				display("<Family3>");

				fillStructure(stateInfo,NAME_CONNECTOR,structures);

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

					fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,true);

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
			else {
				stateInfo.nextState=NRC_S;

				fillStructure(stateInfo,NARRATOR_CONNECTOR,structures);

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

					fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,true,true);

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
					fillStructure(stateInfo,NAME_PRIM,structures,true);

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

				fillStructure(stateInfo,RASOUL_WORD,structures);

				currentData.narratorEndIndex=stateInfo.endPos;
			#ifdef PUNCTUATION
				if (ending_punc) {

					if (structures->hadith) {
						fillStructure(stateInfo,INITIALIZE,structures);
						stateInfo.nextState=TEXT_S;
						return_value=false;
					} else {
						fillStructure(stateInfo,NARRATOR_CONNECTOR,currentChain);
						stateInfo.nextState=NRC_S;
					}
					break;
				}
			#endif
			} else {
				fillStructure(stateInfo,INITIALIZE,structures);

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
		currentData.ibn_or_3abid=stateInfo.familyConnectorOr3abid(); //for it to be saved for next time use
	#endif
		if (!return_value /*&& stateInfo.processedStructure!=INITIALIZE*/)
			fillStructure(stateInfo,INITIALIZE,structures);
		else if (return_value==false)
			assert (structures->narrator==NULL);
		return return_value;
	}
	inline bool result(WordType t, StateInfo &  stateInfo,HadithData *currentChain){display(t); stateInfo.currentType=t; return getNextState(stateInfo,currentChain);}
#ifndef BUCKWALTER_INTERFACE
	bool proceedInStateMachine(StateInfo &  stateInfo,HadithData *structures, stateData & currentData ) //does not fill stateInfo.currType
	{
		hadith_stemmer s(text,stateInfo.startPos);
		if (stateInfo.familyNMC)
			s.tryToLearnNames=true;
		stateInfo.resetCurrentWordInfo();
		long  finish;
		stateInfo.possessivePlace=false;
	#if 0
		static hadith_stemmer * s_p=NULL;
		if (s_p==NULL)
			s_p=new hadith_stemmer(text,stateInfo.startPos);
		else
			s_p->init(stateInfo.startPos);
		hadith_stemmer & s=*s_p;
	#endif

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
			return result(NMC,stateInfo,structures);
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
		QStringRef startText=text->midRef(stateInfo.startPos);
		if (!stop_word)//TODO: maybe modified to be set as a utility function, and just called from here
		{
			foreach (c, compound_words)
			{
			#if 1
				int pos;
				if (startsWith(startText,c,pos))
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
			return result(STOP_WORD,stateInfo,structures);
		}
		if (phrase)
		{
			display("PHRASE ");
			//isBinOrPossessive=true; //same behaviour as Bin
			return result(NMC,stateInfo,structures);
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
			return result(NRC,stateInfo,structures);
		}
		else if (s.nmc)
		{
			if (s.familyNMC) {
			#if defined(GET_WAW) || defined(REFINEMENTS)
				PunctuationInfo copyPunc=stateInfo.currentPunctuationInfo;
			#endif
			#ifdef GET_WAW
				long nextpos=stateInfo.nextPos;
				if (s.has_waw && (stateInfo.currentState==NAME_S ||stateInfo.currentState==NRC_S)) {
					display("waw ");
					stateInfo.isWaw=true;
					stateInfo.startPos=s.wawStart;
					stateInfo.endPos=s.wawEnd;
					stateInfo.nextPos=s.wawEnd+1;
					stateInfo.currentPunctuationInfo.reset();
					if (!result(NRC,stateInfo,structures))
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
				if (s._2ab)
					stateInfo._2ab=true;
				if (s._2om)
					stateInfo._2om=true;
				stateInfo.startPos=s.startStem;
				stateInfo.endPos=s.finishStem;
				stateInfo.currentPunctuationInfo.reset();
				long startSuffix=s.finishStem+1;
				bool isRelativeReference=s.finishStem<finish && suffixNames.contains(text->mid(startSuffix,finish-startSuffix+1));
				if (isRelativeReference)
					stateInfo.nextPos=startSuffix;
				else {
					stateInfo.currentPunctuationInfo=copyPunc;
					stateInfo.nextPos=nextpos;
				}
				if (!result(NMC,stateInfo,structures))
					return false;
				stateInfo.currentState=stateInfo.nextState;
				stateInfo.lastEndPos=stateInfo.endPos;
				if (isRelativeReference) {
					stateInfo.familyNMC=false;
					stateInfo.ibn=false;
					stateInfo.startPos=startSuffix;
					stateInfo.endPos=finish;
					stateInfo.nextPos=nextpos;
					stateInfo.currentPunctuationInfo=copyPunc;
					return result(NAME,stateInfo,structures);
				}
				return true;
			#else
				return result(NMC,stateInfo,currentChain);
			#endif
			}
			if (s.possessive) {
				display("Possessive ");
				return result(NMC,stateInfo,structures);
			}
			return result(NMC,stateInfo,structures);
		}
		else if (s.name){
		#ifdef GET_WAW
			long nextpos=stateInfo.nextPos;
			PunctuationInfo copyPunc=stateInfo.currentPunctuationInfo;
			if (s.has_waw && (stateInfo.currentState==NAME_S ||stateInfo.currentState==NRC_S) ) {
				display("waw ");
				stateInfo.isWaw=true;
				stateInfo.startPos=s.wawStart;
				stateInfo.endPos=s.wawEnd;
				stateInfo.nextPos=s.wawEnd+1;
				stateInfo.currentPunctuationInfo.reset();
				if (!result(NRC,stateInfo,structures))
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
			stateInfo.nextPos=nextpos;
		#endif
			return result(NAME,stateInfo,structures);
		}
		else
			return result(NMC,stateInfo,structures);
	}
#endif

#endif

#endif // HADITHCOMMON_H

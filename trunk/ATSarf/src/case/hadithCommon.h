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
		unsigned int unused_int:10;
		double equality_delta;
		double equality_threshold;
		HadithParameters()
		{
			nmc_max=3;
			narr_min=3;
			nrc_max=5;
			equality_radius=3;
			equality_delta=0.4;
			equality_threshold=1;
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
	enum wordType { NAME, NRC,NMC};
	enum stateType { TEXT_S , NAME_S, NMC_S , NRC_S};
#else
	#define GET_WAW

	enum wordType { NAME, NRC,NMC,STOP_WORD};
	enum stateType { TEXT_S , NAME_S, NMC_S , NRC_S, STOP_WORD_S};
#endif

#ifdef COUNT_RUNON
	extern bool runon;
#endif

	typedef struct  {
		long lastEndPos;
		long startPos;
		long endPos;
		long nextPos;
		wordType currentType:3;
		stateType currentState:4;
		stateType nextState:4;
		bool familyNMC:1;
		bool ibn;
		bool _3abid:1;
		bool possessivePlace:1;
		bool number:1;
		bool isWaw:1;
		bool _3an:1;
		bool learnedName:1;
		bool nrcIsPunctuation:1; //if state is NRC and that is caused soley by punctuation
		int unused:17;
		PunctuationInfo previousPunctuationInfo,currentPunctuationInfo;
		void resetCurrentWordInfo()	{familyNMC=false;ibn=false;_3abid=false;possessivePlace=false;number=false;isWaw=false;_3an=false;learnedName=false;currentPunctuationInfo.reset();}
		bool ibnOr3abid() { return familyNMC || _3abid;}
		bool isFamilyConnectorOrPossessivePlace(){return familyNMC || possessivePlace;}
	} StateInfo;

#ifdef REFINEMENTS
	extern QStringList compound_words, rasoul_words,suffixNames;
	extern QString alrasoul,abyi;
#endif
	extern QString hadath,abid,alrasoul,abyi,_3an;
	extern int bit_POSSESSIVE, bit_PLACE,bit_CITY,bit_COUNTRY,bit_NOUN_PROP;
	extern QList<int> bits_NAME;

#ifdef PREPROCESS_DESCRIPTIONS
	extern QHash<long,bool> familyNMC_descriptions;
	extern QHash<long,bool> NRC_descriptions;
	extern QHash<long,bool> IBN_descriptions;
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
		bool name:1, nrc:1, nmc,possessive:1, familyNMC:1,ibn:1,_3abid:1,stopword:1;
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
			else if (familyNMC_descriptions.contains(stem_info->description_id))
		#endif
			{
			#ifdef STATS
				stem=temp_stem;
			#endif
			#ifdef REFINEMENTS
				familyNMC=true;
				if (IBN_descriptions.contains(stem_info->description_id))
					ibn=true;
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
			for (int j=0;j<suffixNames.size();j++) {
				if (equal_ignore_diacritics(n.m_narrator[i]->getString(),suffixNames[j]))
					return true;
			}
		}
		return false;
	}

#endif // HADITHCOMMON_H

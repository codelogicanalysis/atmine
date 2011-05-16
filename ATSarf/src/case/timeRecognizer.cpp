#include "ATMProgressIFC.h"
#include "stemmer.h"

unsigned int bit_ABSOLUTE_TIME, bit_RELATIVE_TIME;

enum wordType { ABS_T, REL_T};
enum stateType { NOTHING_S , MAYBE_TIME_S, TIME_S};

typedef struct  {
	long lastEndPos;
	long startPos;
	long endPos;
	long nextPos;
	wordType currentType:2;
	stateType currentState:2;
	stateType nextState:2;
	bool followedByPunctuation:1;
	int unused:25;

	void resetCurrentWordInfo()	{followedByPunctuation=false;}
} StateInfo;

void initialize(){
	long abstract_ABSOLUTE_TIME=database_info.comp_rules->getAbstractCategoryID("Absolute Time");
	bit_ABSOLUTE_TIME=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_ABSOLUTE_TIME);
	long abstract_RELATIVE_TIME=database_info.comp_rules->getAbstractCategoryID("Relative Time");
	bit_RELATIVE_TIME=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_RELATIVE_TIME);
}

class time_stemmer: public Stemmer
{
public:
	bool absoluteTime,relativeTime;
	long finish_pos;

	time_stemmer(QString * word, int start):Stemmer(word,start,false) {
		setSolutionSettings(M_ALL);
		init(start);
	}
	void init(int start) {
		this->info.start=start;
		this->info.finish=start;
		absoluteTime=false;
		relativeTime=false;
		finish_pos=start;
	}
	bool on_match() {
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
	}

	bool analyze() {
		if (stem_info->abstract_categories.getBit(bit_ABSOLUTE_TIME))
		{
			absoluteTime=true;
			if (info.finish>finish_pos)
				finish_pos=info.finish;
			return true;
		}
		if (stem_info->abstract_categories.getBit(bit_RELATIVE_TIME))
		{
			relativeTime=true;
			finish_pos=info.finish;
			return false;
		}
	}
};

class TimeEntity {
private:
	QString * text;
	long start, end;
public:
	TimeEntity(QString * text) { this->text=text; start=0;end=0;}
	TimeEntity(QString * text,long start) { this->text=text; this->start=start;end=start;}
	void setStart(long s) { start=s;}
	void setEnd(long e) { end=e; }
	int getStart() const {return start;}
	int getLength() const {	return end - start + 1;}
	virtual int getEnd() const { return end;}
	QString getString() const {
		int length=getLength();
		if (length<0)
			return "";
		return text->mid(getStart(), length);
	}
};

typedef QVector<TimeEntity> TimeEntityVector;

#if 0
//bool getNextState(stateType currentState,wordType currentType,stateType & nextState,long  startPos,bool isIbnOrPossessivePlace,bool possessivePlace,bool ibnOR3abid,bool followedByPunctuation, long endPos,chainData *currentChain)
bool getNextState(StateInfo &  stateInfo,TimeEntityVector * timeVector)
{
	display(QString(" nmcsize: %1 ").arg(currentData.nmcCount));
	display(QString(" nrcsize: %1 ").arg(currentData.nrcCount));
	display(stateInfo.currentState);
	display("\n");
#ifdef PUNCTUATION
	if (stateInfo.followedByPunctuation)
	{
		display("<has punctuation>");
	}
#endif
#ifdef REFINEMENTS
	bool should_stop= (stateInfo.currentType== STOP_WORD && !stateInfo.ibnOr3abid());//stop_word not preceeded by 3abid or ibn
#endif
	bool return_value=true;
	switch(stateInfo.currentState)
	{
	case TEXT_S:
		if(stateInfo.currentType==NAME)
		{
			initializeStateData();
			stateInfo.nextState=NAME_S;
			currentData.sanadStartIndex=stateInfo.startPos;
			currentData.narratorStartIndex=stateInfo.startPos;
		#ifdef CHAIN_BUILDING
			initializeChainData(timeVector);
			timeVector->namePrim=new NamePrim(text,stateInfo.startPos);
		#endif
		#ifdef STATS
			temp_names_per_narrator=1;
		#endif
		#ifdef PUNCTUATION
			if (stateInfo.followedByPunctuation)
			{
				display("<punc1>");
				currentData.narratorCount++;
				stateInfo.nextState=NRC_S;
				currentData.narratorEndIndex=stateInfo.endPos;
				currentData.nrcStartIndex=stateInfo.nextPos;//next_positon(stateInfo.endPos,stateInfo.followedByPunctuation);
			#ifdef CHAIN_BUILDING
				timeVector->namePrim->m_end=stateInfo.endPos;
				timeVector->narrator->m_narrator.append(timeVector->namePrim);
				timeVector->narratorConnectorPrim=new NarratorConnectorPrim(text,currentData.nrcStartIndex);
				timeVector->chain->m_chain.append(timeVector->narrator);
				timeVector->narrator=new Narrator(text);
			#endif
			}
		#endif
		}
		else if (stateInfo.currentType==NRC)
		{
			initializeStateData();
			currentData.sanadStartIndex=stateInfo.startPos;
			currentData.nrcStartIndex=stateInfo.startPos;
			stateInfo.nextState=NRC_S;
		#ifdef CHAIN_BUILDING
			initializeChainData(timeVector);
			timeVector->narratorConnectorPrim=new NarratorConnectorPrim(text,stateInfo.startPos);
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
		else if (stateInfo.currentType==NMC && stateInfo.ibn)
		{
			display("<IBN1>");
			initializeStateData();
			currentData.sanadStartIndex=stateInfo.startPos;
			currentData.nmcStartIndex=stateInfo.startPos;
			currentData.narratorStartIndex=stateInfo.startPos;
			currentData.nmcCount=1;
			stateInfo.nextState=NMC_S;
			currentData.nmcValid=true;
		#ifdef CHAIN_BUILDING
			initializeChainData(timeVector);
			timeVector->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
			timeVector->nameConnectorPrim->setIbn();
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
			timeVector->namePrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
			timeVector->narrator->m_narrator.append(timeVector->namePrim);
			delete timeVector->temp_nameConnectors;
			timeVector->temp_nameConnectors= new TempConnectorPrimList();

			timeVector->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos); //we added this to previous name bc assumed this will only happen if it is muhamad and "sal3am"
			timeVector->nameConnectorPrim->m_end=stateInfo.endPos;
			timeVector->narrator->m_narrator.append(timeVector->nameConnectorPrim);
			timeVector->chain->m_chain.append(timeVector->narrator);
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
		if(stateInfo.currentType==NMC)
		{
			stateInfo.nextState=NMC_S;
			currentData.nmcValid=stateInfo.isIbnOrPossessivePlace();
			currentData.nmcCount=1;
			currentData.nmcStartIndex=stateInfo.startPos;
		#ifdef CHAIN_BUILDING
			timeVector->namePrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
			timeVector->narrator->m_narrator.append(timeVector->namePrim);
			timeVector->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
		#ifdef REFINEMENTS
			if (stateInfo.ibn)
				timeVector->nameConnectorPrim->setIbn();
			else if (stateInfo.possessivePlace)
				timeVector->nameConnectorPrim->setPossessive();
		#endif
			/*for (int i=0;i<currentChain->temp_nameConnectors->count()-1;i++)
				delete (*currentChain->temp_nameConnectors)[i];*/
			delete timeVector->temp_nameConnectors;
			timeVector->temp_nameConnectors= new TempConnectorPrimList();
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
			if (stateInfo.followedByPunctuation)
				currentData.nmcCount=hadithParameters.nmc_max+1;
		#endif
		}
		else if (stateInfo.currentType==NRC)
		{
			stateInfo.nextState=NRC_S;
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
			timeVector->namePrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
			timeVector->narrator->m_narrator.append(timeVector->namePrim);
			timeVector->narratorConnectorPrim=new NarratorConnectorPrim(text,stateInfo.startPos);
			timeVector->chain->m_chain.append(timeVector->narrator);
			timeVector->narrator=new Narrator(text);
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
			stateInfo.nextState=NAME_S;
		#ifdef PUNCTUATION
			if (stateInfo.followedByPunctuation)
			{
				display("<punc2>");
				currentData.narratorCount++;
				stateInfo.nextState=NRC_S;
				currentData.narratorEndIndex=stateInfo.endPos;
				currentData.nrcStartIndex=stateInfo.nextPos;//next_positon(stateInfo.endPos,stateInfo.followedByPunctuation);
			#ifdef CHAIN_BUILDING
				timeVector->namePrim->m_end=stateInfo.endPos;
				timeVector->narrator->m_narrator.append(timeVector->namePrim);
				timeVector->narratorConnectorPrim=new NarratorConnectorPrim(text,currentData.nrcStartIndex);
				timeVector->chain->m_chain.append(timeVector->narrator);
				timeVector->narrator=new Narrator(text);
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
			stateInfo.nextState=NRC_S;

			currentData.narratorEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(currentData.nmcStartIndex);
			currentData.nrcStartIndex=currentData.nmcStartIndex;
			currentData.nrcEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);

		#ifdef CHAIN_BUILDING
			for (int i=0;i<timeVector->temp_nameConnectors->count();i++)
				timeVector->narrator->m_narrator.append(timeVector->temp_nameConnectors->at(i));
			timeVector->chain->m_chain.append(timeVector->narrator);
			display(timeVector->narrator->getString()+"\n");
			timeVector->narratorConnectorPrim=new NarratorConnectorPrim(text,currentData.nrcStartIndex);
			timeVector->narratorConnectorPrim->m_end=currentData.nrcEndIndex;
			timeVector->chain->m_chain.append(timeVector->narratorConnectorPrim);
			display(timeVector->narratorConnectorPrim->getString()+"\n");
		#endif
		//2-create a new narrator of just this stop word as name connector
		#ifdef CHAIN_BUILDING
			timeVector->nameConnectorPrim->m_start=stateInfo.startPos;
			timeVector->nameConnectorPrim->m_end=stateInfo.endPos;
			timeVector->narrator=new Narrator(text);
			timeVector->narrator->m_narrator.append(timeVector->nameConnectorPrim);
			display(timeVector->narrator->getString()+"\n");
			timeVector->chain->m_chain.append(timeVector->narrator);
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
		if (stateInfo.currentType==NRC)
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
			stateInfo.nextState=NRC_S;

			currentData.narratorEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
			currentData.nrcStartIndex=stateInfo.startPos;
		#ifdef CHAIN_BUILDING
			timeVector->nameConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
			timeVector->temp_nameConnectors->append(timeVector->nameConnectorPrim);
			for (int i=0;i<timeVector->temp_nameConnectors->count();i++)
				timeVector->narrator->m_narrator.append(timeVector->temp_nameConnectors->at(i));
			timeVector->narratorConnectorPrim=new NarratorConnectorPrim(text,stateInfo.startPos);
			timeVector->chain->m_chain.append(timeVector->narrator);
			timeVector->narrator=new Narrator(text);
		#endif
		}
		else if(stateInfo.currentType==NAME)
		{
			currentData.nmcCount=0;
			stateInfo.nextState=NAME_S;
		#ifdef CHAIN_BUILDING
			timeVector->nameConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
			timeVector->temp_nameConnectors->append(timeVector->nameConnectorPrim);
			for (int i=0;i<timeVector->temp_nameConnectors->count();i++)
				timeVector->narrator->m_narrator.append(timeVector->temp_nameConnectors->at(i));
			timeVector->namePrim=new NamePrim(text,stateInfo.startPos);
		#endif
		#ifdef STATS
			temp_names_per_narrator++;//found another name name
		#endif
		#ifdef PUNCTUATION
			if (stateInfo.followedByPunctuation)
			{
				display("<punc3>");
				currentData.narratorCount++;
				stateInfo.nextState=NRC_S;
				currentData.narratorEndIndex=stateInfo.endPos;
				currentData.nrcStartIndex=stateInfo.nextPos;//next_positon(stateInfo.endPos,stateInfo.followedByPunctuation);
			#ifdef CHAIN_BUILDING
				timeVector->namePrim->m_end=stateInfo.endPos;
				timeVector->narrator->m_narrator.append(timeVector->namePrim);
				timeVector->narratorConnectorPrim=new NarratorConnectorPrim(text,currentData.nrcStartIndex);
				timeVector->chain->m_chain.append(timeVector->narrator);
				timeVector->narrator=new Narrator(text);
			#endif
			}
		#endif
		}
	#ifdef PUNCTUATION
		else if (stateInfo.followedByPunctuation) //TODO: if punctuation check is all what is required
		{
			stateInfo.nextState=NMC_S;
			currentData.nmcCount=hadithParameters.nmc_max+1;
			currentData.nmcValid=false;
		}
	#endif
		else if (currentData.nmcCount>hadithParameters.nmc_max)
		{
			if (currentData.nmcValid)
			{
				currentData.nmcValid=false;
				stateInfo.nextState=NMC_S;
				currentData.nmcCount=0;
			}
			else
			{
				stateInfo.nextState=TEXT_S;
			#ifdef CHAIN_BUILDING
				/*currentChain->nameConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(start_index);//later check for out of bounds
				currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
				for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
					currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));//check to see if we should also add the narrator to chain
				*/
				timeVector->chain->m_chain.append(timeVector->narrator);
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


				currentData.narratorEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(currentData.nmcStartIndex); //check this case
				return_value= false;
				break;
			}
			//currentData.narratorEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(start_index); check this case

		}
		else if (stateInfo.currentType==NMC)
		{
		#ifdef CHAIN_BUILDING
			timeVector->nameConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
			timeVector->temp_nameConnectors->append(timeVector->nameConnectorPrim);
			timeVector->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
		#ifdef REFINEMENTS
			if (stateInfo.ibn)
				timeVector->nameConnectorPrim->setIbn();
			else if (stateInfo.possessivePlace)
				timeVector->nameConnectorPrim->setPossessive();
		#endif
		#endif
			currentData.nmcCount++;
			if (stateInfo.isIbnOrPossessivePlace())
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
		}
		else
		{
			stateInfo.nextState=NMC_S; //maybe modify later
			timeVector->nameConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
			timeVector->temp_nameConnectors->append(timeVector->nameConnectorPrim);
			timeVector->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
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
			timeVector->narratorConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
			timeVector->chain->m_chain.append(timeVector->narratorConnectorPrim);
			if (timeVector->narrator!=NULL)
				delete timeVector->narrator;
			timeVector->narrator=new Narrator(text);
			timeVector->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
			timeVector->temp_nameConnectors->clear();
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
			timeVector->nameConnectorPrim->m_end=stateInfo.endPos;
			timeVector->narrator->m_narrator.append(timeVector->nameConnectorPrim);
			timeVector->chain->m_chain.append(timeVector->narrator);
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
		if (stateInfo.currentType==NAME || stateInfo.possessivePlace)
	#else
		if (stateInfo.currentType==NAME)
	#endif
		{
			stateInfo.nextState=NAME_S;
			currentData.nrcCount=0;
			//currentData.nameStartIndex=start_index;
		#ifdef CHAIN_BUILDING
			//currentChain->namePrim->m_start=start_index;

			timeVector->narratorConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
			timeVector->chain->m_chain.append(timeVector->narratorConnectorPrim);
		#ifdef REFINEMENTS
			if (stateInfo.currentType==NAME)
		#endif
				timeVector->namePrim=new NamePrim(text,stateInfo.startPos);
		#ifdef REFINEMENTS
			else
			{
				timeVector->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
				timeVector->nameConnectorPrim->setPossessive();
			}
		#endif
		#endif
			currentData.nrcEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
		#ifdef STATS
			temp_names_per_narrator++;//found another name name
		#endif
		#ifdef PUNCTUATION
			if (stateInfo.followedByPunctuation)
			{
				display("<punc4>");
				currentData.narratorCount++;
				stateInfo.nextState=NRC_S;
				currentData.narratorEndIndex=stateInfo.endPos;
				currentData.nrcStartIndex=stateInfo.nextPos;//next_positon(stateInfo.endPos,stateInfo.followedByPunctuation);
			#ifdef CHAIN_BUILDING
			#ifdef REFINEMENTS
				if (stateInfo.currentType==NAME)
				{
			#endif
					timeVector->namePrim->m_end=stateInfo.endPos;
					timeVector->narrator->m_narrator.append(timeVector->namePrim);
			#ifdef REFINEMENTS
				}
				else
				{
					timeVector->nameConnectorPrim->m_end=stateInfo.endPos;
					timeVector->narrator->m_narrator.append(timeVector->nameConnectorPrim);
				}
			#endif

				timeVector->narratorConnectorPrim=new NarratorConnectorPrim(text,currentData.nrcStartIndex);
				timeVector->chain->m_chain.append(timeVector->narrator);
				timeVector->narrator=new Narrator(text);
			#endif
				break;
			}
		#endif
		}
	#ifdef PUNCTUATION
		else if (currentData.nrcCount>=hadithParameters.nrc_max || currentData.nrcPunctuation)
	#else
		else if (currentData.nrcCount>=parameters.nrc_max)
	#endif
		{
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
#ifdef TENTATIVE ////needed in case 2 3an's appear after each other intervened by a name which is unknown
		else if (currentType==NRC)
		{
			if(currentData.nrcCount>1) //in order not to have two NRCs without a name suspect between them
			{
			currentData.narratorCount++;
			display(QString("counter%1\n").arg(currentData.narratorCount));
			currentData.nrcCount=1;
			nextState=NRC_S;
			currentData.narratorEndIndex=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(start_index);
			currentData.nrcStartIndex=start_index;

			currentChain->narratorConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(start_index);
			currentChain->chain->m_chain.append(currentChain->narratorConnectorPrim);

			currentChain->narratorConnectorPrim=new NarratorConnectorPrim(text,start_index);
			}
		}
#endif
#ifdef IBN_START
		else if (stateInfo.currentType==NMC && stateInfo.ibn)
		{
			display("<IBN3>");
		#ifdef CHAIN_BUILDING
			/*currentChain->temp_nameConnectors->append(currentChain->nameConnectorPrim);
			for (int i=0;i<currentChain->temp_nameConnectors->count();i++)
				currentChain->narrator->m_narrator.append(currentChain->temp_nameConnectors->at(i));*/
			timeVector->narratorConnectorPrim->m_end=stateInfo.lastEndPos;//getLastLetter_IN_previousWord(stateInfo.startPos);
			timeVector->chain->m_chain.append(timeVector->narratorConnectorPrim);
			if (timeVector->narrator!=NULL)
				delete timeVector->narrator;
			timeVector->narrator=new Narrator(text);
			timeVector->nameConnectorPrim=new NameConnectorPrim(text,stateInfo.startPos);
			timeVector->nameConnectorPrim->setIbn();
			timeVector->temp_nameConnectors->clear();
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
		}
#endif
		else
		{
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
		}
	#ifdef PUNCTUATION
		if (stateInfo.followedByPunctuation && stateInfo.nextState==NRC_S && stateInfo.currentType!=NAME && stateInfo.currentType!=NRC)
			currentData.nrcPunctuation=true;
	#endif
		break;
#ifdef REFINEMENTS
	case STOP_WORD_S:
		if (stateInfo.currentType==STOP_WORD)
		{
			stateInfo.nextState=STOP_WORD_S;
			((Narrator*)timeVector->chain->m_chain[timeVector->chain->m_chain.length()-1])->m_narrator[0]->m_end=stateInfo.endPos;
			currentData.narratorEndIndex=stateInfo.endPos;
		}
		else
		{
			stateInfo.nextState=TEXT_S;
			return_value=false;
		}
		break;
#endif
	default:
		break;
	}

#ifdef REFINEMENTS
	currentData.ibn_or_3abid=stateInfo.ibnOr3abid(); //for it to be saved for next time use
#endif
	return return_value;
}

wordType getWordType(StateInfo &  stateInfo)
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
#endif


int timeRecognizeHelper(QString input_str,ATMProgressIFC *prg) {
	return 0;
}

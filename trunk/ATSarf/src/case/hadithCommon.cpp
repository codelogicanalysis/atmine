#include "biographyGraphUtilities.h"
#include "hadithCommon.h"
#include "AbstractTwoLevelAgreement.h"
#include "Math_functions.h"
#include "OneLevelAgreement.h"
#include <QStringList>



HadithParameters hadithParameters;

#ifdef REFINEMENTS
	QStringList rasoul_words;
	QStringList compound_words,suffixNames;
#endif

	QString chainDataStreamFileName= ".chainOutput";
#ifdef PREPROCESS_DESCRIPTIONS
	QString preProcessedDescriptionsFileName= ".HadithPreProcessedDescriptions";
#endif
#ifndef SUBMISSION
	QString PhrasesFileName="../src/case/phrases";
	QString StopwordsFileName="../src/case/stop_words";
#else
	QString PhrasesFileName=".phrases";
	QString StopwordsFileName=".stop_words";
#endif

#ifdef COUNT_RUNON
	bool runon;
#endif

	QString hadath,abid,alrasoul,abyi,_3an,_2ama,_3ama;
	int bit_POSSESSIVE, bit_PLACE,bit_CITY,bit_COUNTRY,bit_NOUN_PROP,bit_ENARRATOR_NAMES;
	QList<int> bits_NAME;

#ifdef PREPROCESS_DESCRIPTIONS
	QHash<long,bool> familyNMC_descriptions;
	QHash<long,bool> NRC_descriptions;
	QHash<long,bool> IBN_descriptions;
	QHash<long,bool> OM_descriptions;
	QHash<long,bool> AB_descriptions;
#endif
#ifdef COMPARE_TO_BUCKWALTER
	QTextStream * myoutPtr;
#endif
#ifdef COUNT_AVERAGE_SOLUTIONS
	long total_solutions=0;
	long stemmings=0;
#endif


#ifdef PREPROCESS_DESCRIPTIONS
void readFromDatabasePreProcessedHadithDescriptions() {
	Retrieve_Template nrc_s("description","id","name='said' OR name='say' OR name='notify/communicate' OR name LIKE '%/listen' OR name LIKE 'listen/%' OR name LIKE 'listen %' OR name LIKE '% listen' OR name = 'listen' OR name LIKE '%/inform' OR name LIKE 'inform/%' OR name LIKE 'inform %' OR name LIKE '% inform' OR name = 'inform' OR name LIKE '%from/about%' OR name LIKE '%narrate%'");
	while (nrc_s.retrieve())
		NRC_descriptions.insert(nrc_s.get(0).toULongLong(),true);
	Retrieve_Template nmc_s("description","id","name='son' or name LIKE '% father' or name LIKE 'father' or name LIKE 'father %' or name LIKE '% mother' or name LIKE 'mother' or name LIKE 'mother/%' or name LIKE 'grandfather' or name LIKE 'grandmother' or name LIKE 'father-in-law' or name LIKE 'mother-in-law' or name LIKE '% uncle' or name LIKE '% uncles'");
	while (nmc_s.retrieve())
		familyNMC_descriptions.insert(nmc_s.get(0).toULongLong(),true);
	Retrieve_Template ibn_s("description","id","name='son'");
	while (ibn_s.retrieve())
		IBN_descriptions.insert(ibn_s.get(0).toULongLong(),true);
	Retrieve_Template ab_s("description","id","name LIKE '% father' or name LIKE 'father' or name LIKE 'father %'");
	while (ab_s.retrieve())
		AB_descriptions.insert(ab_s.get(0).toULongLong(),true);
	Retrieve_Template om_s("description","id","name LIKE '% mother' or name LIKE 'mother' or name LIKE 'mother/%'");
	while (om_s.retrieve())
		OM_descriptions.insert(om_s.get(0).toULongLong(),true);


	QFile file(preProcessedDescriptionsFileName.toStdString().data());
	if (file.open(QIODevice::WriteOnly))
	{
		QDataStream out(&file);   // we will serialize the data into the file
		out	<< NRC_descriptions
			<< familyNMC_descriptions
			<< IBN_descriptions
			<< AB_descriptions
			<< OM_descriptions;
		file.close();
	}
	else
		error <<"Unexpected Error: Unable to write PreProcessed Descriptions to file\n";
}
void readFromFilePreprocessedHadithDescriptions() {
#ifndef LOAD_FROM_FILE
	readFromDatabasePreProcessedHadithDescriptions();
#else
	QFile file(preProcessedDescriptionsFileName.toStdString().data());
	if (file.open(QIODevice::ReadOnly))
	{
		QDataStream in(&file);    // read the data serialized from the file
		in	>> NRC_descriptions
			>> familyNMC_descriptions
			>> IBN_descriptions
			>> AB_descriptions
			>> OM_descriptions;
		file.close();
	}
	else
		readFromDatabasePreProcessedHadithDescriptions();
#endif
}
#endif

void hadith_initialize() {
	hadath.append(_7a2).append(dal).append(tha2);
	abid.append(_3yn).append(ba2).append(dal);
	_2ama.append(alef).append(meem).append(alef);
	_3ama.append(_3yn).append(meem).append(alef);
	//abihi.append(alef).append(ba2).append(ya2).append(ha2);
	abyi.append(alef).append(ba2).append(ya2);
	suffixNames.append(QString("")+ha2);
	suffixNames.append(QString("")+ha2+meem);
	suffixNames.append(QString("")+ha2+meem+alef);
	alrasoul.append(alef).append(lam).append(ra2).append(seen).append(waw).append(lam);
	_3an.append(_3yn).append(noon);
#if defined(REFINEMENTS) && !defined(JUST_BUCKWALTER)
	long abstract_NAME=database_info.comp_rules->getAbstractCategoryID("Male Names");
	long abstract_COMPOUND_NAMES=database_info.comp_rules->getAbstractCategoryID("Compound Names");
	int bit_COMPOUND_NAMES=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_COMPOUND_NAMES);
	if (bit_COMPOUND_NAMES>=0)
		bits_NAME.append(bit_COMPOUND_NAMES);
	long abstract_NOUN_PROP=database_info.comp_rules->getAbstractCategoryID("Female Names");//"NOUN_PROP"
	bit_NOUN_PROP=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_NOUN_PROP);

	long abstract_ENARRATOR_NAMES=database_info.comp_rules->getAbstractCategoryID("eNarrator Names");
	bit_ENARRATOR_NAMES=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_ENARRATOR_NAMES);
#ifdef ADD_ENARRATOR_NAMES
	if (bit_ENARRATOR_NAMES>=0)
		bits_NAME.append(bit_ENARRATOR_NAMES);
#endif
#elif defined(JUST_BUCKWALTER)
	long abstract_NAME=database_info.comp_rules->getAbstractCategoryID("NOUN_PROP");
#else
	long abstract_NAME=database_info.comp_rules->getAbstractCategoryID("Name of Person");
#endif
#ifndef JUST_BUCKWALTER
	long abstract_POSSESSIVE=database_info.comp_rules->getAbstractCategoryID("POSSESSIVE");
	long abstract_PLACE=database_info.comp_rules->getAbstractCategoryID("Name of Place");
	long abstract_CITY=database_info.comp_rules->getAbstractCategoryID("City/Town");
	long abstract_COUNTRY=database_info.comp_rules->getAbstractCategoryID("Country");
	bit_POSSESSIVE=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_POSSESSIVE);
	bit_PLACE=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_PLACE);
	bit_CITY=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_CITY);
	bit_COUNTRY=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_COUNTRY);
#else
	abstract_POSSESSIVE=-1;
	abstract_PLACE=-1;
	abstract_CITY=-1;
	abstract_COUNTRY=-1;
#endif
	int bit_NAME=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_NAME);
	if (bit_NAME>=0)
		bits_NAME.append(bit_NAME);
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
	readFromFilePreprocessedHadithDescriptions();
#endif
}

#if defined(HADITHDEBUG) || defined(NARRATORDEBUG)
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

inline void fillStructure(StateInfo &  stateInfo,const Structure & currentStructure,HadithData *structures, StateData & currentData,bool punc=false,bool ending_punc=false) {
#ifdef CHAIN_BUILDING

#define addNarrator(narrator) \
	if (structures->hadith) { \
		structures->chain->m_chain.append(narrator); \
		if (!narrator->isRasoul) {\
			Name n(structures->text,narrator->getStart(),narrator->getEnd());\
			structures->learningEvaluator.addNonContextNarrator(n); \
		} \
	} else {\
		if ((structures->segment!=NULL && structures->segment->addNarrator(narrator)) || \
			(structures->segment==NULL && structures->biography->addNarrator(narrator))) {\
			currentData.bio_nrcCount=0; \
			Name n(structures->text,narrator->getStart(),narrator->getEnd()); /*just to check the benefit of using POR*/ \
			structures->learningEvaluator.addContextNarrator(n); \
		} else if (!narrator->isRasoul) { \
			Name n(structures->text,narrator->getStart(),narrator->getEnd());\
			structures->learningEvaluator.addNonContextNarrator(n); \
		}\
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
				//assert(structures->narrator!=NULL); //check if correct to have removed this and added the if after
				if (structures->narrator!=NULL) {
					for (int i=0;i<size;i++)
						structures->narrator->m_narrator.append(structures->temp_nameConnectors[i]);
				}
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
			case INITIALIZE: {
				if (structures->segmentNarrators) {
					if (structures->narrator==NULL)
						structures->narrator=new Narrator(structures->text);
					structures->narrator->isRasoul=true;
					if (structures->nameConnectorPrim==NULL)
						structures->nameConnectorPrim=new NameConnectorPrim(structures->text,stateInfo.startPos);
					structures->nameConnectorPrim->m_end=stateInfo.endPos;
					break;
				}
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
	display(QString(" nmcsize: %1 ").arg(currentData.nmcCount));
	display(QString(" nrcsize: %1 ").arg(currentData.nrcCount));
	display(stateInfo.currentState);
	bool ending_punc=false;
#ifdef PUNCTUATION
	if (structures->hadith) {
		if (stateInfo.number) {
			stateInfo.currentPunctuationInfo.has_punctuation=true;
			stateInfo.currentPunctuationInfo.fullstop=true;
			stateInfo.currentPunctuationInfo.newLine=true;
		}
	} else {
		stateInfo.number=false;//numbers may appear in many places in biography
	}
	if (stateInfo.currentPunctuationInfo.has_punctuation) {
		display("<has punctuation>");
		if (stateInfo.currentPunctuationInfo.fullstop && stateInfo.currentPunctuationInfo.newLine) {
			ending_punc=true;
			display("<ending Punctuation>");
		}
	}
#endif
	int nrc_max= (structures->hadith?hadithParameters.nrc_max:hadithParameters.bio_nrc_max);
	int nmc_max= (structures->hadith?hadithParameters.nmc_max:hadithParameters.bio_nmc_max);
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
		if(reachedRasoul && structures->segmentNarrators)
		{
			display("<STOP0>");
			stateInfo.nextState=STOP_WORD_S;
			currentData.narratorCount=1;

			fillStructure(stateInfo,RASOUL_WORD,structures,currentData);

			currentData.narratorEndIndex=stateInfo.endPos;

			//return_value= false;
			break;
		}
		if(stateInfo.currentType==NAME) {
		#ifdef PUNCTUATION
			if (structures->hadith && !stateInfo.previousPunctuationInfo.fullstop) {
				stateInfo.nextState=TEXT_S;
				break;
			}
		#endif

			currentData.initialize();
			stateInfo.nextState=NAME_S;
			currentData.mainStructureStartIndex=stateInfo.startPos;
			currentData.narratorStartIndex=stateInfo.startPos;

			fillStructure(stateInfo,NAME_PRIM,structures,currentData);

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
			currentData.mainStructureStartIndex=stateInfo.startPos;
			currentData.nmcStartIndex=stateInfo.startPos;
			currentData.narratorStartIndex=stateInfo.startPos;
			currentData.nmcCount=1;
			stateInfo.nextState=NMC_S;
			currentData.nmcValid=true;

			fillStructure(stateInfo,NAME_CONNECTOR,structures,currentData);

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

				fillStructure(stateInfo,INITIALIZE,structures,currentData,true,true); //futureStructure=INITIALIZE will reset the structure for next time and will not hurt flow since anyways resetting here

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

			fillStructure(stateInfo,RASOUL_WORD,structures,currentData);

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
			fillStructure(stateInfo,NAME_CONNECTOR,structures,currentData);

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

				/*added all this now*/
				//currentData.nmcCount=nmc_max+1;
				currentData.narratorCount++;
				stateInfo.nextState=NRC_S;
				currentData.nrcCount=0; //punctuation not counted
				currentData.narratorEndIndex=stateInfo.endPos;
				currentData.nrcStartIndex=stateInfo.nextPos;//next_positon(stateInfo.endPos,stateInfo.followedByPunctuation);

				fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,currentData,true);
				/*till here*/

				if (ending_punc) {
					stateInfo.nextState=TEXT_S;

					fillStructure(stateInfo,INITIALIZE,structures,currentData,true,true);

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

			/*added all this now*/
			//stateInfo.nextState=NMC_S;
			//currentData.nmcCount=nmc_max+1;
			//currentData.nmcValid=false;
			currentData.narratorCount++;
			stateInfo.nextState=NRC_S;
			currentData.nrcCount=0; //punctuation not counted
			currentData.narratorEndIndex=stateInfo.endPos;
			currentData.nrcStartIndex=stateInfo.nextPos;//next_positon(stateInfo.endPos,stateInfo.followedByPunctuation);

			fillStructure(stateInfo,NARRATOR_CONNECTOR,structures,currentData,true);
			/*till here*/

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
inline bool result(WordType t, StateInfo &  stateInfo,HadithData *currentChain, StateData & currentData){
	display(t);
	stateInfo.currentType=t;
	bool val= getNextState(stateInfo,currentChain,currentData);

#ifdef NONCONTEXT_LEARNING
	if (!val) {
		if (currentChain->hadith && !currentChain->segmentNarrators &&
				(( currentData.narratorCount>=hadithParameters.narr_min)
					|| (!currentChain->hadith && currentData.narratorCount>=hadithParameters.bio_narr_min))	) {
			for (int i=0;i<currentChain->chain->m_chain.size();i++) {
				ChainPrim * c=currentChain->chain->m_chain[i];
				if (c->isNarrator()) {
					Narrator * n=(Narrator *)c;
					for (int j=0;j<n->m_narrator.size();j++) {
						NarratorPrim * np=n->m_narrator[j];
						if (np->isNamePrim()) {
							NamePrim * name=(NamePrim *)np;
							if (name->learnedName) {
								Name p(currentChain->text,name->getStart(),name->getEnd());
								currentChain->learningEvaluator.addContextLearnedName(p);
							}
						}
					}
					if (!n->isRasoul) {
						Name p(currentChain->text,n->getStart(),n->getEnd());
						currentChain->learningEvaluator.addContextNarrator(p);
					}
				}
			}
		}
	}
#endif
	return val;
}
#ifndef BUCKWALTER_INTERFACE
	bool proceedInStateMachine(StateInfo &  stateInfo,HadithData *structures, StateData & currentData ) //does not fill stateInfo.currType
	{
		hadith_stemmer s(structures->text,stateInfo.startPos);
	#ifdef NONCONTEXT_LEARNING
		hadith_stemmer nameLearner(structures->text,stateInfo.startPos);
	#endif
		bool family=false;
		if (stateInfo.familyNMC) {
			s.tryToLearnNames=true;
		#ifdef NONCONTEXT_LEARNING
			nameLearner.tryToLearnNames=true;
		#endif
			family=true;
		}
		stateInfo.resetCurrentWordInfo();
		long  finish;
		stateInfo.possessivePlace=false;
	#if 0
		static hadith_stemmer * s_p=NULL;
		if (s_p==NULL)
			s_p=new hadith_stemmer(structures->text,stateInfo.startPos);
		else
			s_p->init(stateInfo.startPos);
		hadith_stemmer & s=*s_p;
	#endif

	#ifdef REFINEMENTS
	#ifdef TRYTOLEARN
		bool nrcLearning=false;
		if (stateInfo.currentState==NRC_S && currentData.nrcCount<=1
			#ifdef PUNCTUATION
				&& !stateInfo.nrcIsPunctuation
			#endif
				) {
			nrcLearning=true;
			s.tryToLearnNames=true;
		}
	#endif
	#ifdef NONCONTEXT_LEARNING
		if (stateInfo.nrcPreviousType)
			nameLearner.tryToLearnNames=true;
	#endif
		//assert(!s.tryToLearnNames || nameLearner.tryToLearnNames);
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

		QString c;
		bool found,phrase=false,stop_word=false;
		foreach (c, rasoul_words)
		{
		#if 1
			int pos;
			if (startsWith(structures->text->midRef(stateInfo.startPos),c,pos))
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
				if (!isDiacritic(structures->text->at(pos)))
				{
					if (!equal(c[i],structures->text->at(pos)))
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
		QStringRef startText=structures->text->midRef(stateInfo.startPos);
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
					if (!isDiacritic(structures->text->at(pos)))
					{
						if (!equal(c[i],structures->text->at(pos)))
						{
							found=false;
							break;
						}
						i++;
					}
					pos++;
				}
				while (isDiacritic(structures->text->at(pos)))
					pos++;
				if (isDiacritic(structures->text->at(pos)))
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
		#ifdef TRYTOLEARN
			QString n=s.getString().toString();
			if (!s.tryToLearnNames && (stateInfo.currentType==NRC || stateInfo.currentType==NAME) && startsWithAL(n) && getLastLetter(n)==ya2) {
				nameLearner.tryToLearnNames=true;
				s.init(stateInfo.startPos);
				s.tryToLearnNames=true;
				s();
			}
		#endif
		#ifdef NONCONTEXT_LEARNING
			if (nameLearner.tryToLearnNames)
				nameLearner();
		#endif
			finish=max(s.info.finish,s.finish_pos);
			if (s.numSolutions==0/*finish==stateInfo.startPos*/) {
				finish=getLastLetter_IN_currentWord(structures->text,stateInfo.startPos);
			#ifdef REFINEMENTS
				if (s.tryToLearnNames && removeDiacritics(structures->text->mid(stateInfo.startPos,finish-stateInfo.startPos+1)).count()>=3) {
					display("{learned}");
					s.name=true;
					s.finishStem=finish;
					s.startStem=stateInfo.startPos;
					if (structures->text->at(stateInfo.startPos)==waw) {
						long newPos=stateInfo.startPos;
						skipOneLetter(structures->text,newPos);
						if (skipAL(structures->text,newPos)) {
							stateInfo.startPos=newPos;
							s.startStem=newPos;
						}
					}
					s.learnedName=true;
				}
			#endif
			#ifdef NONCONTEXT_LEARNING
				if (!structures->segmentNarrators && nameLearner.tryToLearnNames && removeDiacritics(structures->text->mid(stateInfo.startPos,finish-stateInfo.startPos+1)).count()>=3) {
					nameLearner.name=true;
					Name p(structures->text,stateInfo.startPos,finish);
					structures->learningEvaluator.addNonContextLearnedName(p);
				}
			#endif
			}
			#ifdef STATS
				current_exact=removeDiacritics(s.info->text->mid(stateInfo.startPos,finish-stateInfo.startPos+1));
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
		stateInfo.nextPos=next_positon(structures->text,finish,stateInfo.currentPunctuationInfo);
		display(structures->text->mid(stateInfo.startPos,finish-stateInfo.startPos+1)+":");
	#ifdef REFINEMENTS
		if (stop_word || s.stopword) {
			return result(STOP_WORD,stateInfo,structures,currentData);
		}
		if (phrase)
		{
			display("PHRASE ");
			//isBinOrPossessive=true; //same behaviour as Bin
			return result(NMC,stateInfo,structures,currentData);
		}
		stateInfo._3abid=s._3abid;
	#endif
		stateInfo.possessivePlace=s.possessive;
	#ifdef TRYTOLEARN
		if (!s.name && !s.nmc && !s.nrc &&!s.stopword) {
			QString word=s.getString().toString();
			if (s.tryToLearnNames && removeDiacritics(word).count()>=3) {
				bool cond=/*stateInfo.currentPunctuationInfo.has_punctuation ||*/ family;
				if (!cond) {
					hadith_stemmer n(structures->text,stateInfo.nextPos);
					n();
					if (n.is3an)
						cond=true;
				}
				if (cond) {
					display("{learned}");
					s.name=true;
					s.nmc=false;
					s.learnedName=true;
					s.startStem=s.info.start;
					s.finishStem=s.info.finish;
					if (structures->text->at(stateInfo.startPos)==waw) {
						long newPos=stateInfo.startPos;
						skipOneLetter(structures->text,newPos);
						if (skipAL(structures->text,newPos)) {
							stateInfo.startPos=newPos;
							s.startStem=newPos;
						}
					}
				}
			}
		}
	#endif
	#ifdef NONCONTEXT_LEARNING
		if (!structures->segmentNarrators && !nameLearner.name && !nameLearner.nmc && !nameLearner.nrc &&!nameLearner.stopword) {
			QString word=nameLearner.getString().toString();
			if ((stateInfo.currentPunctuationInfo.has_punctuation || family) && nameLearner.tryToLearnNames && removeDiacritics(word).count()>=3) {
				nameLearner.name=true;
				nameLearner.nmc=false;
				Name p(structures->text,nameLearner.info.start,nameLearner.info.finish);
				structures->learningEvaluator.addNonContextLearnedName(p);
			}
		}
		Name p(structures->text,nameLearner.startStem,nameLearner.finishStem);
		if (nameLearner.learnedName && (!nrcLearning || !nameLearner.has_waw || !stateInfo.nrcPreviousType)) //to avoid case of qal w...
			structures->learningEvaluator.addNonContextLearnedName(p);
		if (nrcLearning && stateInfo.nrcPreviousType && s.has_waw) //to avoid case of qal w...
			s.name=false;
		if (s.possessive)
			structures->learningEvaluator.addKnownName(p,false);
	#endif
		stateInfo.nrcPreviousType=false;
		//displayed_error<<"-";
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
			stateInfo.nrcPreviousType=true;
			//displayed_error<<"p"<<s.getString().toString();
			return result(NRC,stateInfo,structures,currentData);
		} else if (s.nmc) {
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
					if (!result(NRC,stateInfo,structures,currentData))
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
				bool isRelativeReference=s.finishStem<finish && suffixNames.contains(structures->text->mid(startSuffix,finish-startSuffix+1));
				if (isRelativeReference)
					stateInfo.nextPos=startSuffix;
				else {
					stateInfo.currentPunctuationInfo=copyPunc;
					stateInfo.nextPos=nextpos;
				}
				if (!result(NMC,stateInfo,structures,currentData))
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
					return result(NAME,stateInfo,structures,currentData);
				}
				return true;
			#else
				return result(NMC,stateInfo,currentChain);
			#endif
			}
			if (s.possessive) {
				display("Possessive ");
				return result(NMC,stateInfo,structures,currentData);
			}
			return result(NMC,stateInfo,structures,currentData);
		} else if (s.name){
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
				if (!result(NRC,stateInfo,structures,currentData))
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
		#ifdef NONCONTEXT_LEARNING
			if (!structures->segmentNarrators) {
				Name p(structures->text,s.startStem,s.finishStem);
				structures->learningEvaluator.addKnownName(p,s.learnedName);
			}
		#endif
			return result(NAME,stateInfo,structures,currentData);
		}
		else
			return result(NMC,stateInfo,structures,currentData);
	}

#endif

#ifdef NONCONTEXT_LEARNING
	bool NameLearningEvaluator::equalNames(QString * ,int start1,int end1,int start2,int end2) {
		return overLaps(start1,end1,start2,end2);
	}

	void NameLearningEvaluator::displayNameLearningStatistics() {
		double contextRecall, contextPrecision, allRecall, allPrecision;
		if (names) {
			QSet<Name>	commonContextNames =annotatedNames,
						commonNonContextNames=annotatedNames,
						nonDetectedNonContextNames=annotatedNames,
						nonDetectedContextNames=annotatedNames,
						incorrectlyDetectedNames=contextNames;
			commonContextNames.intersect(contextNames);
			commonNonContextNames.intersect(nonContextNames);
			nonDetectedNonContextNames.subtract(commonNonContextNames);
			nonDetectedContextNames.subtract(commonContextNames);
			incorrectlyDetectedNames.subtract(annotatedNames);
			QSet<Name> detectedOnlyInNonContext=commonNonContextNames;
			detectedOnlyInNonContext.subtract(commonContextNames);

			QSet<Name>::iterator itr;
			if (annotatedNames.size()>0) {
			#ifndef SUBMISSION
				itr=commonContextNames.begin();
				displayed_error<<"Contextually-Detected Correct Names:\n";
				for (;itr!=commonContextNames.end();itr++)
					displayed_error<<itr->getString()<<"\n";
				itr=detectedOnlyInNonContext.begin();
				displayed_error<<"Additional Correct Names Only Detected Un-Contexually:\n";
				for (;itr!=detectedOnlyInNonContext.end();itr++)
					displayed_error<<itr->getString()<<"\n";
				itr=nonDetectedNonContextNames.begin();
				displayed_error<<"Names not detected without Context:\n";
				for (;itr!=nonDetectedNonContextNames.end();itr++)
					displayed_error<<itr->getString()<<"\n";
				itr=nonDetectedContextNames.begin();
				displayed_error<<"Names not detected with Context:\n";
				for (;itr!=nonDetectedContextNames.end();itr++)
					displayed_error<<itr->getString()<<"\n";
				itr=incorrectlyDetectedNames.begin();
				displayed_error<<"Names Incorrectly detected:\n";
				for (;itr!=incorrectlyDetectedNames.end();itr++)
					displayed_error<<itr->getString()<<"\n";
			#endif

				contextRecall=(double)commonContextNames.size()/annotatedNames.size();
				contextPrecision=(double)commonContextNames.size()/contextNames.size();
				allRecall=(double)commonNonContextNames.size()/annotatedNames.size();
				allPrecision=(double)commonNonContextNames.size()/nonContextNames.size();
				displayed_error << "-------------------------\n"
								<< "Context Names:\n"
								<< "\trecall=\t"<<commonContextNames.size()<<"/"<<annotatedNames.size()<<"=\t"<<contextRecall<<"\n"
								<< "\tprecision=\t"<<commonContextNames.size()<<"/"<<contextNames.size()<<"=\t"<<contextPrecision<<"\n"
								<< "Non-Context Names:\n"
								<< "\trecall=\t"<<commonNonContextNames.size()<<"/"<<annotatedNames.size()<<"=\t"<<allRecall<<"\n"
								<< "\tprecision=\t"<<commonNonContextNames.size()<<"/"<<nonContextNames.size()<<"=\t"<<allPrecision<<"\n";
			}

		}
		//in both cases we report Narrators
		if(annotatedNarrators.size()>0) {
			if (!names)
				nonContextNarrators.append(contextNarrators);
			OneLevelAgreement nonContextStatistics(text,annotatedNarrators,nonContextNarrators);
			OneLevelAgreement contextStatistics(text,annotatedNarrators,contextNarrators);

			/*QSet<int> visitedTags;
			int allCommon;
			int commonContext=commonNames<NameLearningEvaluator>(text,annotatedNarrators,contextNarrators,visitedTags,allCommon,*this);
			visitedTags.clear();
			int commonNonContext=commonNames<NameLearningEvaluator>(text,annotatedNarrators,nonContextNarrators,visitedTags,allCommon,*this);
			contextRecall=(double)commonContext/annotatedNarrators.size();
			contextPrecision=(double)commonContext/contextNarrators.size();
			int commonAll=(names?commonNonContext:commonNonContext+commonContext);
			int allDetectedCount=(names?nonContextNarrators.size():nonContextNarrators.size()+contextNarrators.size());
			allRecall=(double)commonAll/annotatedNarrators.size();
			allPrecision=(double)commonAll/allDetectedCount;*/

			int totalWords=countWords(text,0,text->size()-1);
			int totalNarratorWords=countWords(text,annotatedNarrators);
			int count=1;
			QVector<int> aggregationCountList;
			for (int i=1;i<annotatedNarrators.size();i++) {
				int start=annotatedNarrators[i-1].second;
				int end=annotatedNarrators[i].first;
				PunctuationInfo punc;
				start=next_positon(text,getLastLetter_IN_currentWord(text,start),punc);
				end=getLastLetter_IN_previousWord(text,end);
				bool paraPunc;
				//qDebug()<<Name(text,start,end).getString();
				int numWords=countWords(text,start,end,&paraPunc);
				if (numWords<hadithParameters.nrc_max /*&& !paraPunc*/) {
					count++;
				} else {
					aggregationCountList.append(count);
					count=0;
				}
			}
			if (count>0) {
				aggregationCountList.append(count);
			}
			double narratorConcentration=(double)totalNarratorWords/totalWords;
			double aggregationAverage=average(aggregationCountList);

			displayed_error	<< "-------------------------\n"
							<< (names?"Sanad ":"POR ")<<"Narrators:\n";
							contextStatistics.calculateStatistics();
							contextStatistics.displayStatistics();
			displayed_error << "All Narrators:\n";
							nonContextStatistics.calculateStatistics();
							nonContextStatistics.displayStatistics();
			displayed_error << "Narrator Concentration:\n"
							<< "\tword concentration=\t"<<totalNarratorWords<<"/"<<totalWords<<"=\t"<<narratorConcentration<<"\n"
							<< "\tnarrator count=\t"<<annotatedNarrators.size()<<"\n"
							<< "Agregation Count Average:\n"
							<< "\t\t"<<"=\t"<<aggregationAverage<<"\n"
							<<"------------------------------------\n";

		}
		if ( names && annotatedNames.size()==0) {
			error << "Annotation Names File does not exist\n";
		#ifndef SUBMISSION
			QFile file(QString("%1.names").arg(fileName).toStdString().data());
			if (file.open(QIODevice::WriteOnly)) {
				QDataStream out(&file);   // we will serialize the data into the file
				out << knownNames;
				file.close();
				error << "Annotation File has been written from current known names, Correct it before use.\n";
			}
		#endif
		}
		if (annotatedNarrators.size()==0){
			error << "Annotation Narrator File does not exist\n";
		#ifndef SUBMISSION
			QFile file(QString("%1.narr").arg(fileName).toStdString().data());
			if (file.open(QIODevice::WriteOnly)) {
				QDataStream out(&file);   // we will serialize the data into the file
				if (!names)
					nonContextNarrators.append(contextNarrators);
				out<<nonContextNarrators;
				file.close();
				error << "Annotation File has been written from current known narrators, Correct it before use.\n";
			}
		#endif
		}
	}

#endif

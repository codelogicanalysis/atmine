#include "hadith.h"

#ifndef GENERAL_HADITH

#include <QFile>
#include <QRegExp>
#include <QStringList>
#include "logger/logger.h"
#include "../sql-interface/Search_by_item.h"
#include "../sarf/stemmer.h"
#include "../utilities/letters.h"

enum wordType { IKHBAR, KAWL, AAN, NAME, TERMINAL_NAME,OTHER};
QString delimiters(" :.,()");
QString a5barani,a5barana,sami3to,hadathana,hadathani,Aan,qal,yaqool,_bin,_ibin;

void hadith_initialize()
{
	a5barani.append(alef_hamza_above).append(kha2).append(ba2).append(ra2).append(noon).append(ya2);
	a5barana.append(alef_hamza_above).append(kha2).append(ba2).append(ra2).append(noon).append(alef);
	sami3to.append(seen).append(meem).append(ayn).append(ta2);
	hadathana.append(_7a2).append(dal).append(tha2).append(noon).append(alef);
	hadathani.append(_7a2).append(dal).append(tha2).append(noon).append(ya2);
	Aan.append(ayn).append(noon);
	qal.append(qaf).append(alef).append(lam);
	yaqool.append(ya2).append(qaf).append(waw).append(lam);

	delimiters="["+delimiters+fasila+"]";
}

class mystemmer: public Stemmer
{
public:
	bool name;

	mystemmer(QString word):Stemmer(word)
	{
		name=false;
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
#ifdef REDUCE_THRU_DIACRITICS
						if (stem_info.category_id==Stem->category_of_currentmatch && stem_info.raw_data==Stem->raw_data_of_currentmatch)
#else
						if (stem_info.category_id==Stem->category_of_currentmatch)
#endif
						{
							for (unsigned int i=0;i<stem_info.abstract_categories.size();i++)
									if (stem_info.abstract_categories[i] && get_abstractCategory_id(i)>=0)
									{
										if (getColumn("category","name",get_abstractCategory_id(i))=="Male Names") //Name of Person
										{
											name=true;
											return false;
										}
									}
						}
				}
				return true;
	}

};

wordType getWordType(QString word)
{
	out << word<<":";
	mystemmer s(word);
	s();
	//after adding abstract categories, we can return IKHBAR, KAWL, AAN, NAME,OTHER
	if (equal(word,a5barana) || equal(word,a5barani) || equal(word,hadathana) || equal(word,hadathani))
	{
		out <<"IKHBAR"<< " ";
		return IKHBAR;
	}
	else if (equal(word,qal) || equal(word,yaqool))
	{
		out <<"KAWL"<< " ";
		return KAWL;
	}
	else if (equal(word,Aan))
	{
		out <<"AAN"<< " ";
		return AAN;
	}
	else if (s.name)
	{
		out <<"NAME"<< " ";
		return NAME;
	}
	else
	{
		out <<"OTHER"<< " ";
		return OTHER;
	}
}

int getSanadBeginning(QStringList wordList)
{
	int listSize=wordList.size();
	for (int i=0;i<listSize;i++)
	{
		if (getWordType(wordList[i])==IKHBAR)
			return i;
	}
	return -1;
}

void buildwords()
{
	a5barani.append(alef_hamza_above).append(kha2).append(ba2).append(ra2).append(noon).append(ya2);
	a5barana.append(alef_hamza_above).append(kha2).append(ba2).append(ra2).append(noon).append(alef);
	sami3to.append(seen).append(meem).append(ayn).append(ta2);
	hadathana.append(_7a2).append(dal).append(tha2).append(noon).append(alef);
	hadathani.append(_7a2).append(dal).append(tha2).append(noon).append(ya2);
	Aan.append(ayn).append(noon);
	qal.append(qaf).append(alef).append(lam);
	yaqool.append(ya2).append(qaf).append(waw).append(lam);

	delimiters="["+delimiters+fasila+"]";
}

bool isValidTransition(int currentState,wordType currentType,int & nextState)
{

	switch(currentState)
	{
		case 1:
		if(currentType==NAME)
		{
			nextState=2;
			return TRUE;
		}
		else
			return FALSE;

		case 2:
		if(currentType==AAN)
		{
			nextState=3;
			return TRUE;
		}
		else if(currentType==KAWL)
		{
			nextState=4;
			return TRUE;
		}
		else
			return FALSE;

		case 3:
		if(currentType==NAME)
		{
			nextState=2;
			return TRUE;
		}
		else
			return FALSE;

		case 4:
		if(currentType==IKHBAR)
		{
			nextState=1;
			return TRUE;
		}
		else if(currentType==KAWL)
		{
			nextState=5;
			return TRUE;
		}
		else
			return FALSE;

		case 5:
		if(currentType==NAME)
		{
			nextState=2;
			return TRUE;
		}
				else if (currentType==TERMINAL_NAME)
		{
			nextState=6;
			return TRUE;
		}
		else
			return FALSE;
		default:
		{
						nextState=currentState;
			return TRUE;

		}
	}

}

int hadith(QString input_str)
{
		//file parsing:
		QFile input(input_str);
		if (!input.open(QIODevice::ReadWrite))
		{
				out << "File not found\n";
				return 1;
		}
		QTextStream file(&input);
		file.setCodec("utf-8");
		while (!file.atEnd())
		{
				QString line=file.readLine(0);
				if (line.isNull())
						break;
				if (line.isEmpty()) //ignore empty lines if they exist
						continue;
				QStringList wordList=line.split((QRegExp(delimiters)),QString::KeepEmptyParts);//space or enter



				int sanadBeginning=getSanadBeginning(wordList);

				int countOthersMax=5;
				int countOthers=0;
				if (sanadBeginning<0)
						return 0;

				int listSize=wordList.size()-sanadBeginning;

				int currentState=1;
				int nextState=1;
				wordType currentType;
				out<<"start of hadith";
				for (int i=sanadBeginning;i<listSize;i++)
				{
						currentType=getWordType(wordList[i]);

						if (currentType!=OTHER)
						{
								countOthers=0;
								if (isValidTransition(currentState,currentType,nextState))
								{
										//out << wordList[i]<< " ";
										currentState=nextState;
										continue;
								}
						}
						else
						{
								countOthers++;
								if (countOthers==countOthersMax)
										out<<"End of Sanad";
						}
				}
		}
		return 0;
///hhh
}

#endif

#include <QFile>
#include <QRegExp>
#include <QStringList>
#include "test.h"
//#include "../builders/functions.h"
#include "../utilities/text_handling.h"
#include "../sql-interface/Search_by_item.h"
#include "../sql-interface/sql_queries.h"
#include "../sarf/stemmer.h"
#include "../caching_structures/database_info_block.h"
#include "../utilities/diacritics.h"

enum wordType { IKHBAR, KAWL, AAN, NAME, NAME_NABI,OTHER};

//const QChar alef_hamza_above= QChar(0x0623);
//const QChar ya2=QChar(0x064A);
//const QChar waw=QChar(0x0648);
const QChar kha2=QChar(0x062E);
const QChar ba2=QChar(0x0628);
const QChar ra2=QChar(0x0628);
const QChar noon=QChar(0x0646);
//const QChar alef=QChar(0x0627);
const QChar seen=QChar(0x0633);
const QChar meem=QChar(0x0645);
const QChar ayn=QChar(0x0639);
const QChar ta2=QChar(0x062A);
const QChar qaf=QChar(0x0642);
const QChar _7a2=QChar(0x062D);
const QChar dal=QChar(0x062F);
const QChar tha2=QChar(0x062B);

QString a5barani,a5barana,sami3to,hadathana,hadathani,Aan,qal,yaqool;

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
												if (getColumn("category","name",get_abstractCategory_id(i))=="NOUN_PROP")
														{
															name=true;
															return false;
														}
						}
				}
				return true;
	}

};

wordType getWordType(QString word)
{
	mystemmer s(word);
	s();
	//after adding abstract categories, we can return IKHBAR, KAWL, AAN, NAME,OTHER
	if (equal(word,a5barana) || equal(word,a5barani) || equal(word,hadathana) || equal(word,hadathani))
		return IKHBAR;
	else if (equal(word,qal) || equal(word,yaqool))
		return KAWL;
	else if (equal(word,Aan))
		return AAN;
	else if (s.name)
		return NAME;
	else
		return OTHER;
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

}


bool isValidTransition(int previousState,wordType currentType)
{
	int nextState=-1;

	switch(previousState)
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
		else if (currentType==NAME_NABI)
		{
			nextState=6;
			return TRUE;
		}
		else
			return FALSE;
		default:
			return FALSE;
	}

}

bool first_time=true;
//starting point
int start(QString input_str, QString &output_str, QString &error_str)
{

	out.setString(&output_str);
	out.setCodec("utf-8");
	in.setString(&input_str);
	in.setCodec("utf-8");
	displayed_error.setString(&error_str);
	displayed_error.setCodec("utf-8");
/*
		if (first_time)
	{
		database_info.fill();
		first_time=false;
	}
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
		//QStringList entries=line.split(QRegExp(QString("[ \n]")),QString::KeepEmptyParts);//space or enter
*/

	/*QString word,word2;
	in >>word;
	int i1,i2;
	in >>i1>>i2;
	out<<getDiacriticword(i2,i1,word)<<"\n";*/
	//out<<equal(word,word2)<<"\n";

	QString word;
	in >>word;
	   out<<string_to_bitset(word).to_string().data()<<"     "<<bitset_to_string(string_to_bitset(word))<<"\n";
/*	Stemmer stemmer(word);
        stemmer();
*/

///hhh
/*

		QStringList wordList=line.split(" ",QString::SkipEmptyParts);

        int sanadBeginning=getSanadBeginning(wordList);

        if (sanadBeginning<0)
            return 0;

        int listSize=wordList.size()-sanadBeginning;

        int previousState=1;
        wordType currentType;

        for (int i=sanadBeginning;i<listSize;i++)
        {
            currentType=getWordType(wordList[i]);

            if (currentType!=OTHER)
            {
                if (isValidTransition(previousState,currentType))
                {
					continue;
                }
            }
        }
	}
///hhh
*/


	return 0;
}



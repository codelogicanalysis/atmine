#include <QFile>
#include <QRegExp>
#include <QStringList>
#include "test.h"
#include "../builders/functions.h"
#include "../utilities/text_handling.h"
#include "../sql-interface/Search_by_item.h"
#include "../sql-interface/sql_queries.h"
#include "../sarf/stemmer.h"
#include "../caching_structures/database_info_block.h"
#include "../utilities/diacritics.h"

enum wordType { IKHBAR, KAWL, AAN, NAME, TERMINAL_NAME,OTHER};

QString delimiters(" :.,");
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
									{

										if (getColumn("category","name",get_abstractCategory_id(i))=="NOUN_PROP" ||getColumn("category","name",get_abstractCategory_id(i))=="Name of Person")
										{
											//out<<"abcat:"<<	getColumn("category","name",get_abstractCategory_id(i))<<"\n";
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
int word_sarf_test()
{

///hhh

        QString word;
        in >>word;
        //	out<<string_to_bitset(word).to_string().data()<<"     "<<bitset_to_string(string_to_bitset(word))<<"\n";*/
        Stemmer stemmer(word);
        stemmer();
        return 0;



}
int hadith_test_case(QString input_str)
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

typedef struct stateData_{
    int count21,count22,max21,min21,min22,max22,firstNameIndex;
} stateData;

void initializeStateData(stateData & currentData)
{
currentData.count21=0;
currentData.count22=0;
currentData.max21=1000;
currentData.max22=4;
currentData.min21=4;
currentData.min22=0;
}


int getNextState(int currentState,wordType currentType,int & nextState,stateData & currentData,int index)
{

        out<<currentState<<endl;
        out<<"21:"<<currentData.count21<<" ";
        out<<"22:"<<currentData.count22<<" ";
        switch(currentState)
        {
                case 0:
                if(currentType==NAME)
                {
                    nextState=1;
                    currentData.firstNameIndex=index;
                    currentData.count21=0;
                    currentData.count22=0;
                    return 1;
                }

                return 0; //still outside

                case 1:
                if((currentType!=NAME)&&(currentState!=AAN))
                {
                    nextState=2;
                }
                return 1;

                case 2:
                if(currentType==NAME)
                {
                    nextState=1;
                    currentData.count21++;
                    if ((currentData.count21==currentData.min21)&&(currentData.count22>=currentData.min22))
                    {
                       // nextIndex=index+1;
                        return 2;
                    }

                }                
                else if (currentType==TERMINAL_NAME)//modify this for backtracking
                {
                    nextState=3;
                 //   nextIndex=index+1;
                    return 2;
                }
                else
                {
                    currentData.count22++;
                    if (currentData.count22==currentData.max22)
                        return 0;

                }
                return 1;


                default:
                return 0;
         }
    }


int hadith_test_case_general(QString input_str)
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
//        while (!file.atEnd())
  //      {
                QString wholeFile=file.readAll();//
                if (wholeFile.isNull())
                {
                        out<<"file error";
                        return 0;
                }
                if (wholeFile.isEmpty()) //ignore empty files
                {
                    out<<"empty file";
                    return 0;
                }
                QStringList wordList=wholeFile.split((QRegExp(delimiters)),QString::KeepEmptyParts);//space or enter

 //finding the start of hadith
                int listSize=wordList.size();

                int currentState=0;
                int nextState=0;
                int offset=3; //letters before first name in hadith
                wordType currentType;
                int newHadithStart=-1;


                stateData currentData;
                initializeStateData(currentData);

                int i=0;
                while (i<listSize)
                {
                        currentType=getWordType(wordList[i]);

                        if(getNextState(currentState,currentType,nextState,currentData,i)==2)

                        {
                                 newHadithStart=currentData.firstNameIndex;//-offset;
                                 out<<"start: "<<wordList[newHadithStart]<<endl;
                                 break;
                        }
                        currentState=nextState;
                        i++;
                }


                if (newHadithStart<0)
                {
                    out<<"no hadith found\n";
                    return 0;
                }


//continue probing the hadith for end of sanad
                i++;
                bool searchForStart=false;
                bool searchForEnd=true;

                int result;

                while (i<listSize)
                {

                   currentType=getWordType(wordList[i]);
                   result=getNextState(currentState,currentType,nextState,currentData,i);
                   out<<"result: "<<result<<endl;

                   if (searchForEnd)
                    {
                       if(result==0)

                        {
                            out<<"end: "<<wordList[i]<<endl;
                            searchForStart=true;
                            searchForEnd=false;

                        }
                    }

                    else if(searchForStart)
                    {

                        if(result==2)

                        {
                                 newHadithStart=currentData.firstNameIndex;//-offset;
                                 out<<"start: "<<wordList[newHadithStart]<<endl;
                                 searchForStart=false;
                                 searchForEnd=true;
                        }

                    }

                    currentState=nextState;
                    i++;
                }

                //

    //     }

return 0;
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

	if (first_time)
	{
		database_info.fill();
		first_time=false;
		buildwords();
	}

      //  hadith_test_case(input_str);
        //word_sarf_test();
                hadith_test_case_general(input_str);

	/*if (insert_rules_for_Nprop_Al()<0)
		return -1;*/


	/*QString word;
	in >>word;*/
	/*	out<<string_to_bitset(word).to_string().data()<<"     "<<bitset_to_string(string_to_bitset(word))<<"\n";
		out<<string_to_bitset(bitset_to_string(string_to_bitset(word))).to_string().data()<<"\n";*/
	/*Stemmer stemmer(word);
	stemmer();*/


	return 0;
}



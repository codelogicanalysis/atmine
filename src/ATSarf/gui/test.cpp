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
//#define HADITHDEBUG
enum wordType { IKHBAR, KAWL, AAN, NAME, TERMINAL_NAME,OTHER, NRC,NMC};
enum stateType { TEXT_S , NAME_S, NMC_S , NRC_S};
QString delimiters(" :.,");
QString a5barani,a5barana,sami3to,hadathana,hadathani,Aan,qal,yaqool,_bin,_ibin;

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

wordType getWordTypeGeneral(QString word)
{
#ifdef     HADITHDEBUG
        out << word<<":";
#endif

        mystemmer s(word);
        s();
        //after adding abstract categories, we can return IKHBAR, KAWL, AAN, NAME,OTHER
        if (equal(word,a5barana) || equal(word,a5barani) || equal(word,hadathana) || equal(word,hadathani) || equal(word,qal) || equal(word,yaqool) ||equal(word,Aan))
        {
#ifdef     HADITHDEBUG

                out <<"NRC"<< " ";
#endif
                return NRC;
        }
        else if (s.name)
        {
            if (equal(word,_bin) || equal(word,_ibin))
            {
                #ifdef     HADITHDEBUG
                out <<"NMC"<< " ";
#endif
                return NMC;
            }
            else
            {
                #ifdef     HADITHDEBUG
                out <<"NAME"<< " ";
#endif
                return NAME;
            }
        }
        else //add nisba later
        {
            #ifdef     HADITHDEBUG
                out <<"NMC"<< " ";
#endif
                return NMC;
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
        _bin.append(ba2).append(noon);
        _ibin.append(alef).append(ba2).append(noon);

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
    int firstNameIndex,nmcThreshold, narratorCount,nrcThreshold,narratorStartIndex,narratorEndIndex,nrcStartIndex,nrcEndIndex,narratorThreshold;
    QList <QString>nmcList;
    QList <QString>nrcList;
} stateData;

void initializeStateData(stateData & currentData)
{

currentData.nmcThreshold=3;
currentData.nmcList.clear();
currentData.narratorCount=0;
currentData.nrcThreshold=5;
currentData.nrcList.clear();
currentData.narratorStartIndex=0;
currentData.narratorEndIndex=0;
currentData.nrcStartIndex=0;
currentData.nrcEndIndex=0;
currentData.narratorThreshold=3;
}


bool getNextState(stateType currentState,wordType currentType,stateType & nextState,stateData & currentData,int index,QStringList wordList)
{

        //out<<currentState<<endl;
    #ifdef     HADITHDEBUG
        out<<" nmcsize: "<<currentData.nmcList.size();
        out<<" nrcsize: "<<currentData.nrcList.size();
#endif
        switch(currentState)
        {
                case TEXT_S:
                  #ifdef     HADITHDEBUG
            out<<"TEXT_S"<<endl;
#endif
                    if(currentType==NAME)
                    {
                        nextState=NAME_S;
                        currentData.firstNameIndex=index;
                        currentData.narratorStartIndex=index;
                        currentData.narratorCount=0;
                    }
                    else if (currentType==NRC)
                    {
                        nextState=NRC_S;
                    }
                    else
                    {
                        nextState=TEXT_S;
                    }

                    return TRUE;

                case NAME_S:
                    #ifdef     HADITHDEBUG
                    out<<"NAME_S"<<endl;
#endif
                    if(currentType==NMC)
                    {
                        nextState=NMC_S;
                        currentData.nmcList.clear();
                        currentData.nmcList.append(wordList[index]);

                    }
                    else if (currentType==NRC)
                    {
                        nextState=NRC_S;
                        currentData.narratorCount++;
                        #ifdef     HADITHDEBUG
                        out<<"counter"<<currentData.narratorCount<<endl;
#endif
                        currentData.nrcList.clear();
                         currentData.nrcList.append(wordList[index]);
                        currentData.narratorEndIndex=index-1;
                        currentData.nrcStartIndex=index;

                    }
                    else
                    {
                        nextState=NAME_S;
                    }

                    return TRUE;

                case NMC_S:
                    #ifdef     HADITHDEBUG
                    out<<"NMC_S"<<endl;
#endif
                    if ((currentData.nmcList.size()>currentData.nmcThreshold)&&((!(currentData.nmcList.contains(_bin))) || (!(currentData.nmcList.contains(_ibin)))))
                    {
                        nextState=TEXT_S;
                        currentData.narratorEndIndex=index-1;
                        return FALSE;
                    }
                    else if (currentType==NRC)
                    {
                        currentData.narratorCount++;
                        #ifdef     HADITHDEBUG
                        out<<"counter"<<currentData.narratorCount<<endl;
#endif

                        currentData.nrcList.clear();
                        currentData.nrcList.append(wordList[index]);

                        nextState=NRC_S;
                        currentData.narratorEndIndex=index-1;
                        currentData.nrcStartIndex=index;
                    }

                    else if(currentType==NAME)
                    {
                        nextState=NAME_S;
                    }
                    else if (currentType==NMC)
                    {
                        currentData.nmcList.append(wordList[index]);
                        nextState=NMC_S;
                    }
                    else
                    {
                        nextState=NMC_S; //maybe modify later
                    }
                    return TRUE;

                case NRC_S:
                    #ifdef     HADITHDEBUG
                    out<<"NRC_S"<<endl;
#endif

                    if (currentData.nrcList.size()>currentData.nrcThreshold)
                    {
                        nextState=TEXT_S;
                        currentData.nrcEndIndex=index-1;
                        return FALSE;
                    }
                    else if (currentType==NAME)
                    {
                        nextState=NAME_S;
                        currentData.nrcEndIndex=index-1;
                    }
                    else
                    {
                        nextState=NRC_S;
                    }
                    return TRUE;


                default:
                return TRUE;
         }
    }

int hadith_test_case_general(QString input_str)
{


    //file parsing:
		QFile input(input_str.split("\n")[0]);
		//out<<"file error:"<<input.errorString();
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
						out<<"file error:"<<input.errorString()<<"\n";
						/*#undef error
						return (input.error());*/
                }
                if (wholeFile.isEmpty()) //ignore empty files
                {
					out<<"empty file\n";
                    return 0;
                }
                QStringList wordList=wholeFile.split((QRegExp(delimiters)),QString::KeepEmptyParts);//space or enter

 //finding the start of hadith
                int listSize=wordList.size();

                stateType currentState=TEXT_S;
                stateType nextState=TEXT_S;
                int offset=3; //letters before first name in hadith
                wordType currentType;
                int newHadithStart=-1;


                stateData currentData;
                initializeStateData(currentData);

                int i=0;
                while (i<listSize)
                {
                        currentType=getWordTypeGeneral(wordList[i]);

                        if((getNextState(currentState,currentType,nextState,currentData,i,wordList)==FALSE)&& currentData.narratorCount>=currentData.narratorThreshold)

                        {
                                 newHadithStart=currentData.firstNameIndex;//-offset;
                                 out<<"new hadith start: "<<wordList[newHadithStart]<<" "<<wordList[newHadithStart+1]<<" "<<wordList[newHadithStart+2]<<" "<<wordList[newHadithStart+3]<<endl;
                                 out<<"sanad end: "<<wordList[i]<<" "<<wordList[i+1]<<" "<<wordList[i+2]<<endl; //maybe i+-1
                               //  break;
                        }
                        currentState=nextState;
                        i++;
                }


                if (newHadithStart<0)
                {
                    out<<"no hadith found\n";
                    return 0;
                }

/*
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
*/


return 0;
}
int augment()
{
	if (insert_buckwalter()<0)
		return -1;
	if (insert_rules_for_Nprop_Al())
		return -1;
	if (insert_propernames()<0)
		return -1;
	if (insert_placenames()<0)
		return -1;
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

    //   hadith_test_case_general(input_str);
      word_sarf_test();
               // hadith_test_case_general(input_str);

	/*if (insert_rules_for_Nprop_Al()<0)
=======
	//  hadith_test_case(input_str);
	//word_sarf_test();
	hadith_test_case_general(input_str);
	//augment();
        if (insert_placenames()<0)
>>>>>>> .r120
		return -1;*/
	return 0;
}



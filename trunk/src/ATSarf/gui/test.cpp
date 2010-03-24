#include <QFile>
#include <QRegExp>
#include "test.h"
//#include "../builders/functions.h"
#include "../sql-interface/sql_queries.h"
#include "../sarf/stemmer.h"
#include "../caching_structures/database_info_block.h"
#include "../utilities/diacritics.h"

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
		QStringList entries=line.split(QRegExp(QString("[ \n]")),QString::KeepEmptyParts);//space or enter
            }

	/*QString word,word2;
	in >>word;
	int i1,i2;
	in >>i1>>i2;
	out<<getDiacriticword(i2,i1,word)<<"\n";*/
	//out<<equal(word,word2)<<"\n";

	QString word;
	in >>word;
	if (first_time)
	{
		database_info.fill();
		first_time=false;
	}
	Stemmer stemmer(word);
	stemmer();

///hhh


        QStringList wordList=word.split(" ",QString::SkipEmptyParts);

        int sanadBeginning=getSanadBeginning(wordList);

        if (sanadBeginning<0)
            return 0;

        int listSize=wordList.size()-sanadBeginning;

        int previousState=1;
        int currentType;

        for (i=sanadBeginning;i<listSize;i++)
        {
            currentType=getWordType(wordListFromSanad[i]);

            if (currentType!=OTHER)
            {
                if (isValidTransition(previousState,currentType))
                {
                    continue;
                }
            }
        }

///hhh



	return 0;
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

wordState getWordType(QString word)
{
    //after adding abstract categories, we can return IKHBAR, KAWL, AAN, NAME,OTHER
    return IKHBAR

}

bool isValidTransition(int previousState,wordType currenType)
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
        else if (currenType==NAME_NABI)
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

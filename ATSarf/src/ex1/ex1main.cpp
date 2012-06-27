/**
  * @file   ex1main.cpp
  * @author Ameen Jaber
  * @brief  This file implements the main of an example to illustrate the use of the morphological analyzer. In this example,
  * we extract the words from an input file with possible verb POS tags, and return them to the user in an output file.
  */
#include <QtGui/QApplication>
#include <iostream>
#include <QFile>
#include "stemmer.h"
#include "POSVerb.h"
#include "sql_queries.h"
#include "database_info_block.h"
#include "initialize_tool.h"
#include <QColor>


using namespace std;


/**
  * This method initializes a sample of the class defined and triggers the pracket operator in it to start the tool
  */
void process(QString & input, QTextStream & outStream)
{
    QStringList list = input.split(' ', QString::SkipEmptyParts);
    for(int i=0; i<list.size(); i++)
    {
        QString * inString = &(list[i]);
        POSVerb posverb(inString, outStream);
	posverb();
    }
}

class MyProgressIFC : public EmptyProgressIFC {

public:
    virtual void report(int value)
    {
        cout<<"Progress is "<<value<<'.'<<endl;
    }

    virtual void startTaggingText(QString & text)
    {
    }

    virtual void tag(int start, int length,QColor color, bool textcolor=true)
    {
    }

    virtual void finishTaggingText()
    {
    }

    virtual void setCurrentAction(const QString & s)
    {
    }

    virtual void resetActionDisplay()
    {
    }

    virtual QString getFileName()
    {
        return "";
    }
    virtual void displayGraph(AbstractGraph *) {}
};

int verbPOSExamplewithInterface()
{
    QFile Ofile("output.txt");
    QFile Efile("error.txt");
    Ofile.open(QIODevice::WriteOnly);
    Efile.open(QIODevice::WriteOnly);
    MyProgressIFC * pIFC = new MyProgressIFC();

    bool all_set = sarfStart(&Ofile,&Efile, pIFC);

    if(!all_set)
    {
        error<<"Can't Set up Project";
    }
    else
    {
        cout<<"All Set"<<endl;
    }

    char filename[100];
    cout << "please enter a file name: " << endl;
    cin >> filename;



    QFile Ifile(filename);
    if (!Ifile.open(QIODevice::ReadOnly | QIODevice::Text)) {
       cerr << "error opening file." << endl;
       return -1;
    }


    QTextStream output(&Ofile);

    QTextStream in(&Ifile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        process(line,output);
    }

    Ofile.close();
    sarfExit();

    return 0;
}

int verbPOSExampleDefault()
{

    bool all_set = sarfStart();

    if(!all_set)
    {
        error<<"Can't Set up Project";
    }
    else
    {
        cout<<"All Set"<<endl;
    }

    char filename[100];
    cout << "please enter a file name: " << endl;
    cin >> filename;



    QFile Ifile(filename);
    if (!Ifile.open(QIODevice::ReadOnly | QIODevice::Text)) {
       cerr << "error opening file." << endl;
       return -1;
    }

    QTextStream in(&Ifile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        process(line,out);
    }
    sarfExit();
    return 0;
}

int ex1_main(int argc, char *argv[])
{
//    verbPOSExamplewithInterface();
    /// This method shows the
    verbPOSExampleDefault();
    return 0;
}

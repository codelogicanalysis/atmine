#include <QFile>
#include <QtGui/QApplication>
#include <iostream>
#include "stemmer.h"
#include "POSVerb.h"
//#include "GlossSFR.h"
#include "sql_queries.h"
#include "database_info_block.h"
#include "initialize_tool.h"
#include "MainWindow.h"


using namespace std;

void doit(QString & input, QTextStream & outStream)
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
    virtual void displayGraph(AbstractGraph * /*graph*/) {}
};

int verbPOSExamplewithInterface()
{
    QFile Ofile("output.txt");
    QFile Efile("error.txt");
    Ofile.open(QIODevice::WriteOnly);
    Efile.open(QIODevice::WriteOnly);
    MyProgressIFC * pIFC = new MyProgressIFC();

    /*
    displayed_error.setDevice(&Efile);
    out.setDevice(&Ofile);
    initialize_variables();
    */

    bool all_set = sarfStart(&Ofile,&Efile, pIFC);

    if(!all_set)
    {
        error<<"Can't Set up Project";
    }
    else
    {
        cout<<"All Set"<<endl;
    }
    /*
    start_connection(emptyPIFC);
    generate_bit_order("source",source_ids);
    generate_bit_order("category",abstract_category_ids,"abstract");
    */
    //database_info.fill(emptyPIFC);

    char filename[100];
    cout << "please enter a file name: " << endl;
    cin >> filename;



    QFile Ifile(filename);
    if (!Ifile.open(QIODevice::ReadOnly | QIODevice::Text)) {
       cerr << "error opening file." << endl;
       return -1;
    }


    QTextStream output(&Ofile); // we will serialize the data into the file

    QTextStream in(&Ifile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        doit(line,output);
    }

    Ofile.close();
    sarfExit();
     // Add code to show the output, or write it to a file
/*
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
*/
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
    /*
    start_connection(emptyPIFC);
    generate_bit_order("source",source_ids);
    generate_bit_order("category",abstract_category_ids,"abstract");
    */
    //database_info.fill(emptyPIFC);

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
        doit(line,out);
    }
    sarfExit();
    return 0;
}


int main(int argc, char *argv[])
{
    verbPOSExamplewithInterface();
    cout<<"Testing testing ..."<<endl;
    verbPOSExampleDefault();
    return 0;
}

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


int main(int argc, char *argv[])
{
    QFile Ofile("output.txt");
    QFile Efile("error.txt");
    Ofile.open(QIODevice::WriteOnly);
    Efile.open(QIODevice::WriteOnly);
    EmptyProgressIFC * emptyPIFC = new EmptyProgressIFC();

    /*
    displayed_error.setDevice(&Efile);
    out.setDevice(&Ofile);
    initialize_variables();
    */

    bool all_set = initialize_tool(&Ofile,&Efile, emptyPIFC);

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
     // Add code to show the output, or write it to a file
/*
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
*/
    return 0;   
}

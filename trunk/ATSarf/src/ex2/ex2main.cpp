#include <QtGui/QApplication>
#include <iostream>
#include <QFile>
#include "stemmer.h"
#include "GlossSFR.h"
#include "sql_queries.h"
#include "database_info_block.h"

using namespace std;


void doit(QString & input)
{
    QStringList list = input.split(' ', QString::SkipEmptyParts);
    for(int i=0; i<list.size(); i++)
    {
        GlossSFR glosssfr(&(list[i]));
        glosssfr();
    }
}


int main(int argc, char *argv[])
{
    /*
    initialize_variables();
    start_connection(0);
    generate_bit_order("source",source_ids);
    generate_bit_order("category",abstract_category_ids,"abstract");
    cout<<"hello"<<endl;
    database_info.fill(0);
    */
    char filename[100];
    cout << "please enter a file name: " << endl;
    cin >> filename;

    QFile Ifile(filename);
    if (!Ifile.open(QIODevice::ReadOnly | QIODevice::Text)) {
       cerr << "error opening file." << endl;
       return -1;
    }

    QFile Ofile("output.txt");
    Ofile.open(QIODevice::WriteOnly);
    QDataStream out(&Ofile); // we will serialize the data into the file

    QTextStream in(&Ifile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        doit(line);
    }
     // Add code to show the output, or write it to a file

/*
    QApplication a(argc, argv);
    POSVerbMainWindow w;
    w.show();
    return a.exec();
*/
    return 0;   
}

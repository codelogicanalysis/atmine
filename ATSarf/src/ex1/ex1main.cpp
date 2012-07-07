/**
  * @file   ex1main.cpp
  * @author Ameen Jaber
  * @brief  This file implements the main of an example to illustrate the use of the morphological analyzer. In this example,
  * we extract the words from an input file with possible verb POS tags, and return them to the user in an output file.
  */
#include <iostream>
#include <QFile>
#include "POSVerb.h"
#include <QColor>
#include <sarf.h>

using namespace std;


/**
  * This method runs an instance of the class defined and triggers the pracket operator in it to start the tool
  * @param input This is a string representing the input string to be processed
  */
void run_process(QString & input) {

    QStringList list = input.split(' ', QString::SkipEmptyParts);
    for(int i=0; i<list.size(); i++) {
        QString * inString = &(list[i]);
        POSVerb posverb(inString);
	posverb();
    }
}

/**
  * @class MyProgressIFC
  * @brief This class inherets from EmptyProgressIFC and implements the different functions present in it. Those functions
  * are used in order to expose the progress of the program we implemented. For detailed description, refer to the
  * documentation of the class.
  */
class MyProgressIFC : public EmptyProgressIFC {

public:
    virtual void report(int value) {
        cout<<"Progress is "<<value<<'.'<<endl;
    }

    virtual void startTaggingText(QString & text) {
    }

    virtual void tag(int start, int length,QColor color, bool textcolor=true) {
    }

    virtual void finishTaggingText() {
    }

    virtual void setCurrentAction(const QString & s) {
    }

    virtual void resetActionDisplay() {
    }

    virtual QString getFileName() {
        return "";
    }
    virtual void displayGraph(AbstractGraph *) {}
};

/**
  * This function tests the verbPOS example with an interface for the output result and error. In addition, a user
  * implementation of the progress functions is used through the class MyProgressIFC previously declared.
  * @return This function returns 0 if successful, else -1
  */
int verbPOSExamplewithInterface() {

    /*
     * The following lines define the output files in which the resulting output or error are written.
     * Also, an instance of the progress class MyProgressIFC is initialized.
     */
    QFile Ofile("output.txt");
    QFile Efile("error.txt");
    Ofile.open(QIODevice::WriteOnly);
    Efile.open(QIODevice::WriteOnly);
    MyProgressIFC * pIFC = new MyProgressIFC();

    /*
     * The previously declared files and progress instance are passed to the sarfStart function in order to initilaize the
     * tool.
     */
    Sarf srf;
    bool all_set = srf.start(&Ofile,&Efile, pIFC);

    Sarf::use(&srf);

    if(!all_set) {
        error<<"Can't Set up Project";
    }
    else {
        cout<<"All Set"<<endl;
    }

    // Take the input file name from the user and save it in a char string, which is then passed to a QFile
    char filename[100];
    cout << "please enter a file name: " << endl;
    cin >> filename;

    // The input file name is passed to a QFile which implements an interface for reading from and writing to files
    QFile Ifile(filename);
    if (!Ifile.open(QIODevice::ReadOnly | QIODevice::Text)) {
       cerr << "error opening file." << endl;
       return -1;
    }

    /*
     * The opened input file is passed to a text stream in order to read it and pass the lines to the core function to
     * run the implemented analyzer on.
     */


    QTextStream in(&Ifile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        run_process(line);
    }

    // This function is called after the processing is done in order to close the tool properly.
    srf.exit();
    //Ofile.close();

    return 0;
}

/**
  * This tests a default version of the verbPOS example where no destination is specified for the output and error by the user.
  * @return The function returns 0 if successful, else -1.
  */
int verbPOSExampleDefault() {

    /*
     * sarfStart is a function used to initialize the Sarf tool
     * It returns 1 if succesful else 0
     */
    Sarf srf;
    bool all_set = srf.start();

    Sarf::use(&srf);

    if(!all_set) {
        error<<"Can't Set up Project";
    }
    else {
        cout<<"All Set"<<endl;
    }

    // Take the input file name from the user and save it in a char string, which is then passed to a QFile
    char filename[100];
    cout << "please enter a file name: " << endl;
    cin >> filename;

    // The input file name is passed to a QFile which implements an interface for reading from and writing to files
    QFile Ifile(filename);
    if (!Ifile.open(QIODevice::ReadOnly | QIODevice::Text)) {
       cerr << "error opening file." << endl;
       return -1;
    }


    /*
     * The opened input file is passed to a text stream in order to read it and pass the lines to the core function to
     * run the implemented analyzer on.
     */
    QTextStream in(&Ifile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        run_process(line);
    }

    // This function is called after the processing is done in order to close the tool properly.
    srf.exit();
    //delete theSarf;

    return 0;
}

int ex1_main(int argc, char *argv[]) {

    //theSarf = new Sarf();
    int test;

    test = verbPOSExampleDefault();
    if(!test) {
        cout<<"The example without interface is successful\n";
    }
    else {
        cout<<"The example without interface failed\n";
    }

#if 1
    test = verbPOSExamplewithInterface();
    if(!test) {
        cout<<"The example with interface is successful\n";
    }
    else {
        cout<<"The example with interface failed\n";
    }
#endif

    return 0;
}

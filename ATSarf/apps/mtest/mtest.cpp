/**
 * @file ex0main.cpp
 * @author Ameen Jaber
 * @brief  This file implements the main of an example to illustrate the basic use of the morphological analyzer. In this example,
 * we pass an arabic text file to the program which is analyzed using the tool.
 */
#include <iostream>
#include <QFile>
#include <sarf.h>
#include <stemmer.h>
#include <myprogressifc.h>

using namespace std;


/**
 * This method runs an instance of the class defined and triggers the pracket operator in it to start the tool
 * @param input This is a string representing the input string to be processed
 */
void run_process(QString &input) {
    QStringList list = input.split(' ', QString::SkipEmptyParts);

    for (int i = 0; i < list.size(); i++) {
        QString *inString = &(list[i]);
        Stemmer stemmer(inString, 0);
        stemmer();
    }
}


/**
 * This tests a default version of the stemmer example where no destination is specified for the output and error by the user.
 * @return The function returns 0 if successful, else -1.
 */
int stemmerExampleDefault(const char *filename) {
    /*
     * sarfStart is a function used to initialize the Sarf tool
     * It returns 1 if succesful else 0
     */
    Sarf srf;
    bool all_set = srf.start();
    Sarf::use(&srf);

    if (!all_set) {
        _error << "Can't Set up Project";
    } else {
        cout << "All Set" << endl;
    }

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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Usage: mtest filename" << endl;
        return -1;
    }

    //theSarf = new Sarf();
    int test;
    test = stemmerExampleDefault(argv[1]);

    if (!test) {
        cout << "The example without interface is successful\n";
    } else {
        cout << "The example without interface failed\n";
    }

    return 0;
}

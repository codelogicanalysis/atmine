#include <iostream>
#include <QFile>
#include "sarf.h"
#include "alpha.h"
#include "gamma.h"
#include "myprogressifc.h"

#if 0
int glossSFRExampleWithInterface() {

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

    Ofile.close();
    return 0;
}
#endif

using namespace std;

int main(int argc, char *argv[]) {

    QFile Ofile("output.txt");
    QFile Efile("error.txt");
    Ofile.open(QIODevice::WriteOnly);
    Efile.open(QIODevice::WriteOnly);
    MyProgressIFC * pIFC = new MyProgressIFC();

    Sarf srf;
    bool all_set = srf.start(&Ofile,&Efile, pIFC);

    if(!all_set) {
        error<<"Can't Set up Project";
    }
    else {
        cout<<"All Set"<<endl;
    }

    Sarf::use(&srf);

    srf.exit();
    return 0;
}

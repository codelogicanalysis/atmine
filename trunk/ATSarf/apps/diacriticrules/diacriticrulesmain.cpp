#include <iostream>
#include <QFile>
#include <sarf.h>
#include "enumerator.h"
#include <myprogressifc.h>

using namespace std;

int main(int argc, char *argv[]) {


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

    Enumerator words(true);
    words();

    // This function is called after the processing is done in order to close the tool properly.
    srf.exit();
    //Ofile.close();

    return 0;
}

#include <QFile>
#include <QTextCodec>
//#include <sys/time.h>
#include "numnorm.h"
#include "sarf.h"
#include "myprogressifc.h"

using namespace std;

int main(int argc, char *argv[]) {

    /** Set encoding to UTF-8 **/

    QTextCodec::setCodecForTr( QTextCodec::codecForName( "UTF-8" ) );
    QTextCodec::setCodecForCStrings( QTextCodec::codecForName( "UTF-8" ) );

    /** Read input word from command line **/

    if(argc != 2) {
        cout<<"Input text file is not specified!\n";
        return 0;
    }

    QString fileName(argv[1]);

    /** Initialize Sarf Instance use by tool **/
    QFile iFile(fileName);
    if(!iFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        cerr << "error opening file." << endl;
        return -1;
    }
    QString text = iFile.readAll();
    if(text.isEmpty()) {
        cerr << "error no text found in input file!" << endl;
        return -1;
    }

    QFile Ofile("output.txt");
    QFile Efile("error.txt");
    Ofile.open(QIODevice::WriteOnly);
    Efile.open(QIODevice::WriteOnly);
    MyProgressIFC * pIFC = new MyProgressIFC();

    Sarf srf;
    bool all_set = srf.start(&Ofile,&Efile, pIFC);

    if(!all_set) {
        error<<"Can't Set up Project";
        return 0;
    }
    else {
        cout<<"All Set"<<endl;
    }

    Sarf::use(&srf);

    /*
    timeval tim;
    gettimeofday(&tim,NULL);
    double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
    */
    /** Run Synonymity analysis **/
    NumNorm nn(&text);
    nn();

    if(nn.extractedNumbers.count()!=0) {
        for(int i=0; i< nn.extractedNumbers.count(); i++) {
            cout << nn.extractedNumbers[i].getNumber() << '\n';
        }
    }
    /*
    gettimeofday(&tim, NULL);
    double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
    double diff = t2-t1;
    cout << "elapsed time " << diff << " s\n";
    */

    /** Close Sarf instance and exit **/
    srf.exit();
    return 0;
}

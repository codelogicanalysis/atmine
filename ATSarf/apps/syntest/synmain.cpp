#include <iostream>
#include <QFile>
#include <QTextCodec>
#include "sarf.h"
#include "ger.h"
#include "myprogressifc.h"

using namespace std;

int main(int argc, char *argv[]) {

    /** Set encoding to UTF-8 **/

    QTextCodec::setCodecForTr( QTextCodec::codecForName( "UTF-8" ) );
    QTextCodec::setCodecForCStrings( QTextCodec::codecForName( "UTF-8" ) );

    /** Read input word from command line **/

    if(argc != 3) {
        cout<<"Pass an arabic word as argument to process\n";
        return 0;
    }

    QString word(argv[1]);

    QString order_string(argv[2]);

    bool ok;
    int order = order_string.toInt(&ok);
    if(!ok) {
        cout<<"Wrong order input value\n";
        return 0;
    }

    /** Initialize Sarf Instance use by tool **/
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

    /** Run Synonymity analysis **/
    GER ger(word, 0, order);
    ger();

    /** Close Sarf instance and exit **/
    srf.exit();
    return 0;
}

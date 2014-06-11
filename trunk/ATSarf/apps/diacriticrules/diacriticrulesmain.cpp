#include <QFile>
#include <sarf.h>
#include "diacriticrules.h"
//#include "ruletest.h"
#include <myprogressifc.h>

using namespace std;

/// Used for testing
#if 0
class TempWord {
public:
    QString word;
    int start;
    int end;
};

TempWord inextWord(QString & text, int pos) {

    QSet<QChar> diacriticsSet;
    diacriticsSet.insert(shadde);
    diacriticsSet.insert(shadde);
    diacriticsSet.insert(fatha);
    diacriticsSet.insert(damma);
    diacriticsSet.insert(kasra);
    diacriticsSet.insert(sukun);
    diacriticsSet.insert(kasratayn);
    diacriticsSet.insert(dammatayn);
    diacriticsSet.insert(fathatayn);
    diacriticsSet.insert(aleft_superscript);
    diacriticsSet.insert(shadde_alef_above);
    diacriticsSet.insert(madda);

    TempWord word;
    word.word = "";
    if(pos == text.count()) {
        return word;
    }
    int next = pos;

    while((next != text.count()) && !(text.at(next).isLetter()) && !(text.at(next).isDigit()) && !(diacriticsSet.contains(text.at(next)))) {
        next++;
    }

    if(next == text.count()) {
        return word;
    }

    int end = next+1;
    if(text.at(next).isLetter() || diacriticsSet.contains(text.at(next))) {
        while((end != text.count()) && (text.at(end).isLetter() || diacriticsSet.contains(text.at(end)))) {
            end++;
        }
    }
    else {
        while((end != text.count()) && text.at(end).isDigit()) {
            end++;
        }
    }
    end = end -1;
    word.start = next;
    word.end = end;
    word.word = text.mid(next,end-next+1);
    return word;
}
#endif

int main(int argc, char *argv[]) {


    if(argc != 2) {
        cout << "Please enter a number of morphological solutions to consider:\n"
                << "-1 : Use all generatable solutions\n"
                << "N > 0 : Use N generatable solutions\n";
        return 0;
    }

    QString solutions_string(argv[1]);

    bool ok;
    long solutions = solutions_string.toLong(&ok);
    if((!ok) || (solutions<-1) || (solutions == 0)) {
        cout << "Please enter a valid number of morphological solutions to consider:\n"
                << "-1 : Use all generatable solutions\n"
                << "N > 0 : Use N generatable solutions\n";
        return 0;
    }

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

    DiacriticRules dWords(solutions, true);
    dWords();

    cout << "Filtered Items are: " << dWords.getFilteredItems() << endl;

    //This function is called after the processing is done in order to close the tool properly.
    srf.exit();
    Ofile.close();

    return 0;

    /// Testing
#if 0
    QFile file("testText.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return 0;
    }

    QString *text = new QString(file.readAll());

    int start = 0;
    int wordIndex = 1;
    int p1count = 0;
    int p2count = 0;
    int p3count = 0;
    int p4count = 0;
    int p5count = 0;
    int p6count = 0;
    int p7count = 0;
    int p1total = 0;
    int p2total = 0;
    int p3total = 0;
    int p4total = 0;
    int p5total = 0;
    int p6total = 0;
    int p7total = 0;

    while(start != text->count()) {
        TempWord word = inextWord(*text, start);
        QString noDiacriticWord = removeDiacritics(word.word);
        if(noDiacriticWord.isEmpty()) {
            break;
        }

        RuleTest rt(&noDiacriticWord, &p1count, &p2count, &p3count, &p4count, &p5count, &p6count, &p7count,
                    &p1total, &p2total, &p3total, &p4total, &p5total, &p6total, &p7total);
        rt();
        start = word.end + 1;
        wordIndex = wordIndex + 1;
    }
    wordIndex = wordIndex - 1;

    cout << "The number of words processed is: " << wordIndex << endl;
    double accuracy_p1 = ( p1count * 1.0) / p1total;
    cout << "The accuracy of the first pattern is: " << accuracy_p1 << endl;
    double accuracy_p2 = ( p2count * 1.0) / p2total;
    cout << "The accuracy of the second pattern is: " << accuracy_p2 << endl;
    double accuracy_p3 = ( p3count * 1.0) / p3total;
    cout << "The accuracy of the third pattern is: " << accuracy_p3 << endl;
    double accuracy_p4 = ( p4count * 1.0) / p4total;
    cout << "The accuracy of the fourth pattern is: " << accuracy_p4 << endl;
    double accuracy_p5 = ( p5count * 1.0) / p5total;
    cout << "The accuracy of the fifth pattern is: " << accuracy_p5 << endl;
    double accuracy_p6 = ( p6count * 1.0) / p6total;
    cout << "The accuracy of the sixth pattern is: " << accuracy_p6 << endl;
    double accuracy_p7 = ( p7count * 1.0) / p7total;
    cout << "The accuracy of the seventh pattern is: " << accuracy_p7 << endl;

    srf.exit();
    return 0;
#endif
}

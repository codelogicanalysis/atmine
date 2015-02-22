#include <QFile>
#include <sarf.h>
#include <myprogressifc.h>
#include"logger.h"
#include "diacriticguidelines.h"
#include "gafunctions.h"

int main(int argc, char *argv[]) {

    if(argc != 3) {
        cout << "Please enter a number of morphological solutions to consider:\n"
                << "-1 : Use all generatable solutions\n"
                << "N > 0 : Use N generatable solutions\n"
                <<"Also, enter the morpheme type (A-all,P-prefix,S-stem,X-suffix) you want to consider";
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

    enumeration_type enumtype;
    QString morpheme_type(argv[2]);
    if(morpheme_type == "A") {
        enumtype = ENUMALL;
    }
    else if(morpheme_type == "S") {
        enumtype = ENUMSTEM;
    }
    else {
        cout << "Please enter a valid morpheme type to consider:\n"
                << "A : all possible words with prefix-stem-suffix\n"
                << "S : all stems\n";
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

    QString fileName;

    if(morpheme_type == "A") {
        fileName.append("unvocalized_words_");
    }
    else if(morpheme_type == "S") {
        fileName.append("unvocalized_stems_");
    }

    if(solutions == -1) {
        fileName.append("full.dat");
    }
    else {
        fileName.append(QString::number(solutions));
        fileName.append(".dat");
    }
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        cout << "Building the set of all words...\n";
        if(enumtype == ENUMALL) {
            DiacriticGuidelines dg(solutions,enumtype,true);
            dg();
            //dg.saveTrie();
            dg.serializeHash();
        }
        else {
            QChar jeem = QChar(0x062C);
            QChar ghain = QChar(0x063A);
            QChar kaf = QChar(0x0643);
            QVector<QChar> letters;
            letters << hamza << alef_madda_above << alef_hamza_above << waw_hamza_above
                    << alef_hamza_below << ya2_hamza_above << alef << ba2 << ta2_marbouta
                    << ta2 << tha2 << jeem << _7a2 << kha2 << dal << thal << ra2 << zain
                    << seen << sheen << sad << dad << tah << zah << _3yn << ghain << feh
                    << qaf << kaf << lam << meem << noon << ha2 << waw << alef_maksoura
                    << ya2 << alef_wasla;

            QQueue<ATTrie::Position> stem_queue;
            ATTrie * Stem_Trie = database_info.Stem_Trie;
            ATTrie::Position pos = Stem_Trie->startWalk();
            stem_queue.enqueue(pos);

            QHash<QString, qint8> stemHash;

            while(!stem_queue.isEmpty()) {
                ATTrie::Position current_pos = stem_queue.dequeue();

                for(int i=0; i< letters.count(); i++) {
                    QChar current_letter = letters[i];
                    if (Stem_Trie->isWalkable(current_pos,current_letter)) {
                        ATTrie::Position temp_pos = Stem_Trie->clonePosition(current_pos);
                        Stem_Trie->walk(temp_pos, current_letter);
                        stem_queue.enqueue(temp_pos);

                        const StemNode * node=NULL;
                        if (Stem_Trie->isTerminal(temp_pos)) {
                                ATTrie::Position pos2 = Stem_Trie->clonePosition(temp_pos);
                                Stem_Trie->walk(pos2, '\0');
                                node = Stem_Trie->getData(pos2);
                                Stem_Trie->freePosition(pos2);
                                if (node != NULL) {
                                    stemHash.insert(removeDiacritics(node->raw_datas[0][0]),0);
                                }
                        }
                    }
                }
                Stem_Trie->freePosition(current_pos);
            }
            QFile stemFile(fileName);
            stemFile.open(QIODevice::WriteOnly);
            QDataStream stemOut(&stemFile);
            stemOut << stemHash;
            stemFile.close();
        }
    }
    else {
        QDataStream in(&file);    // read the data serialized from the file
        QHash<QString, qint8> hash;
        in >> hash;

        /* Implementing the genetic algorithm **/

//        QSet<int> wLength;
//        QHashIterator<QString, qint8> i(hash);
//        while (i.hasNext()) {
//            i.next();
//            int length = i.key().length();
//            if(!(wLength.contains(length))) {
//                cout << length << endl;
//                wLength.insert(length);
//            }
//        }

#if 0
        int tempCount = 0;
        QStringList listStemPOS;
        QStringList listWeird;
        theSarf->query.exec("SELECT POS FROM stem_category");
        while(theSarf->query.next()) {
            if(!(theSarf->query.value(0).toString().isEmpty())) {
                QString currentPOS = theSarf->query.value(0).toString();
                if(currentPOS.contains('+')) {
                    listWeird << currentPOS;
                    tempCount++;
                    continue;
                }
                QStringList stemPOS = currentPOS.split('/');
                listStemPOS << stemPOS.at(1);
            }
        }
        listStemPOS.removeDuplicates();

        QFile ofile("out.txt");
        if (!ofile.open(QIODevice::WriteOnly | QIODevice::Text))
            return 0;

        QTextStream out(&ofile);
        for(int i=0; i<listStemPOS.count(); i++) {
            out << listStemPOS[i] << '\n';
        }
        ofile.close();

        QFile wfile("weird.txt");
        if (!wfile.open(QIODevice::WriteOnly | QIODevice::Text))
            return 0;

        QTextStream outw(&wfile);
        for(int i=0; i<listWeird.count(); i++) {
            outw << listWeird[i] << '\n';
        }
        wfile.close();
#endif
//        for(int i=0; i<listStemPOS.count(); i++) {
//            if(listStemPOS.at(i).contains("+")) {
//                cout << listStemPOS.at(i).toStdString() << endl;
//            }
//        }
#if 0
        dgGeneticAlgorithm(hash,listStemPOS);
#else
        dgApriori(hash);
#endif

    }

    //This function is called after the processing is done in order to close the tool properly.
    srf.exit();
    Ofile.close();

    return 0;
}

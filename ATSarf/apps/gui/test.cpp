#include <QFile>
#include <QStringList>
#include <QString>
#include <QDateTime>
#include <QMessageBox>
#include "test.h"
#include "logger.h"
#include "hadith.h"
#include "functions.h"
#include "stemmer.h"
#include "database_info_block.h"
#include "text_handling.h"
#include "diacritics.h"
#include "timeRecognizer.h"
#include "bibleGeneology.h"
#include <sys/time.h>
#include "vocalizedCombinations.h"
#include "transliteration.h"

extern void splitRecursiveAffixes();
extern void drawAffixGraph(item_types type);
extern void listAllAffixes(item_types type);
extern int timeTagger(QString input_str);
extern int deserializeGraph(QString fileName, ATMProgressIFC *prg);
extern int mergeGraphs(QString file1, QString file2, ATMProgressIFC *prg);
extern int bibleTagger(QString input_str);
extern int hadithTagger(QString input_str);
extern int atb(QString inputString, ATMProgressIFC *prg);
extern int atb2(QString inputString, ATMProgressIFC *prg);
extern int atbDiacritic(QString inputString, ATMProgressIFC *prg);
extern void diacriticDisambiguationCount(item_types t, int numDiacritics = 1);
extern void diacriticDisambiguationCount(QString fileName, int numDiacritics,
                                         ATMProgressIFC *prg,
                                         QString reducedFile = "reducedOutput",
                                         QString allFile = "fullOutput");
extern void diacriticDisambiguationCount(QStringList &list, int numDiacritics,
                                         ATMProgressIFC *prg);
extern int mada(QString folderName, ATMProgressIFC *prg);
extern int diacriticStatistics(QString inputString, ATMProgressIFC *prg);
extern int regressionTest(QString inputString, ATMProgressIFC *prg);
extern int regressionReload(QString input, ATMProgressIFC *prg);

/**
  * This method extracts the valid data from the input string and passes it to
 * the Stemmer function
  * @author Jad Makhlouta
  * @param  input_str   This is the input string by user
  * @return Returns 0 if function is successful
  */
int word_sarf_test(QString input_str) {
    QString line = input_str.split('\n')[0]; /// Splits the input string based
    /// on new line characters and
    /// takes first entry of it
    Stemmer stemmer(&line,
                    0); /// Passes extracted string to stemmer class constructor
    stemmer(); /// Invoke the stemmer routine using () operator overloading
    return 0;
}

int augment() {
    if (insert_buckwalter() < 0) {
        return -1;
    }

    return 0;
}
/**
  * This method is called in order to extract the morphological analysis of the
 * entered word
  * @author Jad Makhlouta
  * @param  input_str   the string/word entered by the user
  * @param  ATMProgressIFC  pointer to function calling this method
  * @return returns an integer 0 if function operated normal else -1
  */
int morphology(QString input_str, ATMProgressIFC *) {
    if (word_sarf_test(input_str)) {
        return -1;
    }

    return 0;
}

int hadith(QString input_str, ATMProgressIFC *prg) {
    for (int i = 0; i < REPETITIONS; i++) {
        timeval tim;
        gettimeofday(&tim, NULL);
        double t1 = tim.tv_sec + (tim.tv_usec / 1000000.0);

        if (hadithHelper(input_str, prg)) {
            return -1;
        }

        gettimeofday(&tim, NULL);
        double t2 = tim.tv_sec + (tim.tv_usec / 1000000.0);
#if REPETITIONS < 2
        theSarf->out << "elapsed time=" << t2 - t1 << "s\n";
#else
        theSarf->out << t2 - t1 << "\n";
#endif
    }

    return 0;
}

int genealogy(QString inputString, ATMProgressIFC *prg) {
    if (genealogyHelper(inputString, prg)) {
        return -1;
    }

    return 0;
}

int biography(QString inputString, ATMProgressIFC *prg) {
    if (deserializeGraph(inputString, prg)) {
        return -1;
    }

    return 0;
}

int simple_annotation(QString inputString, ATMProgressIFC *) {
    if (timeTagger(inputString)) {
        return -1;
    }

    return 0;
}

int bible_annotation(QString inputString, ATMProgressIFC *) {
    if (bibleTagger(inputString)) {
        return -1;
    }

    return 0;
}

int hadith_annotation(QString inputString, ATMProgressIFC *) {
    if (hadithTagger(inputString)) {
        return -1;
    }

    return 0;
}

int test(QString inputString, ATMProgressIFC *prg) {
    if (regressionReload(inputString, prg)) {
        return -1;
    }

    return 0;
}

int verify(QString, ATMProgressIFC *) {
    // drawAffixGraph(PREFIX);
    // drawAffixGraph(SUFFIX);
    listAllAffixes(SUFFIX);
    listAllAffixes(PREFIX);
    return 0;
}

int breakAffix(QString, ATMProgressIFC *) {
    splitRecursiveAffixes();
    return 0;
}

int timeRecognize(QString input_str, ATMProgressIFC *prg) {
    timeval tim;
    gettimeofday(&tim, NULL);
    double t1 = tim.tv_sec + (tim.tv_usec / 1000000.0);
    int ret = timeRecognizeHelper(input_str, prg);
    gettimeofday(&tim, NULL);
    double t2 = tim.tv_sec + (tim.tv_usec / 1000000.0);
    theSarf->out << "elapsed time=" << t2 - t1 << "s\n";
    return ret;
}

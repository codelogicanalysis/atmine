#ifndef ATBEXPERIMENT_H
#define ATBEXPERIMENT_H
#include <QString>
#include <QFile>
#include <QSet>
#include <QDir>
#include <QTextStream>
#include <QRegExp>
#include <QStringList>
#include <assert.h>
#include "logger.h"
#include "stemmer.h"

class AtbStemmer: public Stemmer {
    public:
        enum Status {EQ, D_GL, D_VOC, D_ALL};
    private:
        const QStringList &voc;
        QStringList &gloss;
        const QStringList &pos;
        const QStringList &input_after;
        const QStringList &pos_after;
        bool found: 1;
        Status stat: 2;
        bool ignoreStem: 1;
        QStringList similarGlosses;
        QStringList similarVoc;

        bool correctTokenize: 1;
        bool skipTokenize: 1;
        QStringList sarfTokenization;
        QString vocalizedSolution;
        QString stemPos;

    private:
        Status updateSimilarFields(Status oldStat, Status currentStat, QString currGloss, QString currVoc, int old_pos);
        Status equal(int &index, minimal_item_info &item, Status currentStat, bool ignore = false);
    public:
        AtbStemmer(QString &input, const QStringList &aVoc, QStringList &aGloss, const QStringList &aPOS,
                   QStringList &aInputAfter, const QStringList &aPOSAfter, bool ignoreStem = false);
        virtual bool on_match();
        bool isFound() const {
            return found;
        }
        Status getStatus() const {
            return stat;
        }
        const QStringList &getMostSimilar() const {
            if (stat == D_VOC) {
                return similarVoc;
            }

            return similarGlosses;
        }
        bool isCorrectlyTokenized() const {
            return correctTokenize;
        }
        bool isSkipTokenize() const {
            return skipTokenize;
        }
        const QStringList &getTokenization() const {
            return sarfTokenization;
        }
        QString getVocalizedSolution() const {
            return vocalizedSolution;
        }
        QString getStemPOS() const {
            return stemPos;
        }
};

class AtbStemmerContextFree: public Stemmer {
        QTextStream *f_out;
        int num_solutions;
        QSet<QString> modifiedPOSList;
    public:
        AtbStemmerContextFree(QString &text, QTextStream *f_out): Stemmer(&text, 0) {
            this->f_out = f_out;
            num_solutions = 0;
        }
        bool on_match();
        int getAmbiguity() const {
            return num_solutions;
        }
        int getModifiedPosAmbiguity() const {
            return modifiedPOSList.size();
        }
};

int atb(QString inputString, ATMProgressIFC *prg);

int atb2(QString inputString, ATMProgressIFC *prg);


#endif // ATBEXPERIMENT_H

#ifndef ATBDIACRITICEXPERIMENT_H
#define ATBDIACRITICEXPERIMENT_H

#include <QString>
#include "ATMProgressIFC.h"
#include "stemmer.h"
#include "diacriticDisambiguation.h"
#include "ambiguity.h"

class AmbCombStat {
    public:
        QString voc;
        QString bestComb;
        QString worstComb;
        int bestAmbiguity;
        int worstAmbiguity;
        int countComb;
        int totalAmbiguity;
    public:
        AmbCombStat(QString voc) {
            reset(voc);
        }
        AmbCombStat(QString voc, QString bestComb, QString worstCom, int bestAmbiguity, int worstAmbiguity,
                    int countComb, int totalAmbiguity);
        void reset(QString voc);
        void augmentStats(QString comb, int ambiguity);
};

typedef QList<AmbCombStat> AmbiguityStatList;

int atbDiacritic(QString inputString, ATMProgressIFC *prg);

#endif

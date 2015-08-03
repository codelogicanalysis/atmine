#include <QFile>
#include <QStringList>
#include <QTextStream>
#include "atbDiacriticExperiment.h"
#include "vocalizedCombinations.h"
#include "transliteration.h"

void AmbCombStat::reset(QString voc) {
    this->voc = voc;
    this->bestComb = voc;
    this->worstComb = voc;
    this->bestAmbiguity = 500;
    this->worstAmbiguity = 0;
    this->countComb = 0;
    this->totalAmbiguity = 0;
}

void AmbCombStat::augmentStats(QString comb, int ambiguity) {
    if (ambiguity > worstAmbiguity) {
        worstAmbiguity = ambiguity;
        worstComb = comb;
    }

    if (ambiguity < bestAmbiguity) {
        bestAmbiguity = ambiguity;
        bestComb = comb;
    }

    totalAmbiguity += ambiguity;
    countComb++;
}

AmbCombStat::AmbCombStat(QString voc, QString bestComb, QString worstComb, int bestAmbiguity, int worstAmbiguity,
                         int countComb, int totalAmbiguity) {
    this->voc = voc;
    this->bestComb = bestComb;
    this->worstComb = worstComb;
    this->bestAmbiguity = bestAmbiguity;
    this->worstAmbiguity = worstAmbiguity;
    this->countComb = countComb;
    this->totalAmbiguity = totalAmbiguity;
}

int addRandomDiacritics(QString &voc, int num_voc) {
    QList<int> diacritics;

    for (int i = 0; i < voc.size(); i++) {
        QChar curr = voc[i];

        if (isDiacritic(curr)) {
            diacritics.append(i);
        }
    }

    for (int i = 0; i < num_voc; i++) {
        int d_size = diacritics.size();

        if (d_size == 0) {
            return i;
        }

        int index = rand() % d_size;
        diacritics.removeAt(index);
    }

    for (int i = diacritics.size() - 1; i >= 0; i--) {
        voc.remove(diacritics[i], 1);
    }

    return num_voc;
}

void getStatDiacriticAssignmentHelper(QString s, AmbiguityStatList &stats) {
    AmbiguityStemmer stemmer(s);
    stemmer();

    for (int amb = 0; amb < ambiguitySize; amb++) {
        int ambiguity = stemmer.getAmbiguity((Ambiguity)amb);
        stats[amb].augmentStats(s, ambiguity);
    }
}

void getStatDiacriticAssignment(QString voc, int numDiacritics, AmbiguityStatList &stats) {
    stats.clear();

    for (int i = 0; i < ambiguitySize; i++) {
        stats.append(AmbCombStat(voc));
    }

    VocalizedCombinationsGenerator c(voc, numDiacritics);

    if (c.isUnderVocalized()) {
#ifdef MAX_FOR_UNDERVOCALIZED
        QString s = voc;
        getStatDiacriticAssignmentHelper(s, stats);
#endif
    } else {
        for (c.begin(); !c.isFinished(); ++c) {
            QString s = c.getString();
            getStatDiacriticAssignmentHelper(s, stats);
        }
    }
}

#define Has_Tanween (partial_voc.contains(fathatayn) && c==1)

int atbDiacritic(QString inputString, ATMProgressIFC *prg) {
    //TODO: apply affect for tanween in equality if present force it
    QFile diacritics_file(inputString);

    if (!diacritics_file.open(QIODevice::ReadWrite)) {
        theSarf->out << "Diacritics File not found\n";
        return 1;
    }

    QString line;
    int total = 0;
    const int maxDiacritics = 6;
    int no_voc_total[ambiguitySize] = {0};
    int voc_total[ambiguitySize] = {0};
    int voc_ambiguity[maxDiacritics][ambiguitySize] = {{0}};
    int no_voc_ambiguity[maxDiacritics][ambiguitySize] = {{0}};
    int total_count[maxDiacritics][ambiguitySize] = {{0}};
    int full_vocalized[ambiguitySize] = {0};
    double other_ambiguity[maxDiacritics][ambiguitySize] = {{0}};
    int other_best_ambiguity[maxDiacritics][ambiguitySize] = {{0}};
    int other_worst_ambiguity[maxDiacritics][ambiguitySize] = {{0}};
    int other_count[maxDiacritics][ambiguitySize] = {{0}};
    int partial_best[maxDiacritics][ambiguitySize] = {{0}};
    int partial_worst[maxDiacritics][ambiguitySize] = {{0}};
    double partial_average[maxDiacritics][ambiguitySize] = {{0}};
    int partial_best_total[ambiguitySize] = {0};
    int partial_worst_total[ambiguitySize] = {0};
    double partial_average_total[ambiguitySize] = {0};
    int correctly_detected[ambiguitySize] = {0}, correctly_detectedNoDiacritics[ambiguitySize] = {0};
    int countReduced = 0, countEquivalent = 0, countEquivalentTanween = 0, countReducedTanween = 0, countTanween = 0;
    QTextStream file(&diacritics_file);
    int filePos = 0;
    long fileSize = file.readAll().size();
    file.seek(0);

    while (!file.atEnd()) {
        line = file.readLine(0);
        filePos += line.size() + 1;

        if (line.startsWith("Diacritics") || line.startsWith("ERROR") || line.isEmpty()) {
            continue;
        }

        total++;
        QStringList entries = line.split("\t", QString::KeepEmptyParts);
        QString voc = entries[0];
        QString partial_voc = entries[1];
        QString no_voc = removeDiacritics(partial_voc);
        int c = entries[2].toInt();
        QString ignore;
        ignore = (entries.size() == 4 ? entries[3] : "");

        if (!equal(voc, no_voc)) {
            continue;
        }

        AmbiguityStemmer stemmer(partial_voc);
        stemmer();
        AmbiguityStemmer stemmer2(no_voc);
        stemmer2();
        AmbiguityStemmer stemmer3(voc);
        stemmer3();

        for (int amb = 0; amb < ambiguitySize; amb++) {
            Ambiguity ambiguity = (Ambiguity)amb;
            int voc_amb = stemmer.getAmbiguity(ambiguity);
            int no_voc_amb = stemmer2.getAmbiguity(ambiguity);
            int full_amb = stemmer3.getAmbiguity(ambiguity);
            no_voc_total[amb] += no_voc_amb;
            voc_total[amb] += voc_amb;
            no_voc_ambiguity[c][amb] += no_voc_amb;
            voc_ambiguity[c][amb] += voc_amb;
            total_count[c][amb]++;

            if (!partial_voc.contains(fathatayn) && c == 1) {
                no_voc_ambiguity[0][amb] += no_voc_amb;
                voc_ambiguity[0][amb] += voc_amb;
                total_count[0][amb]++;
            }

            full_vocalized[amb] += full_amb;

            if (ambiguity == All_Ambiguity) {
                QTextStream *print;

                if (no_voc_amb != voc_amb) {
                    print = &(theSarf->out);
                    countReduced++;

                    if (Has_Tanween) {
                        countReducedTanween++;
                    }
                } else {
                    print = &(theSarf->displayed_error);
                }

                (void)print;
            }

            if (equal(no_voc, voc)) {
                correctly_detectedNoDiacritics[amb]++;
            } else if (ambiguity == All_Ambiguity) {
                theSarf->displayed_error << no_voc << "\t" << voc << "\n";
            }

            if (ignore == "i") {
                correctly_detected[amb]++;
                assert(!equal(partial_voc, voc, true));
            }

            if (equal(partial_voc, voc, true)) {
                correctly_detected[amb]++;

                if (ambiguity == All_Ambiguity) {
                    for (int i = 0; i < maxDiacritics; i++) {
                        AmbiguityStatList d;
                        getStatDiacriticAssignment(voc, i, d);

                        for (int amb = 0; amb < ambiguitySize; amb++) {
                            if (d[amb].countComb > 0) {
                                double average = ((double)d[amb].totalAmbiguity) / d[amb].countComb;
                                int best = d[amb].bestAmbiguity;
                                int worst = d[amb].worstAmbiguity;
                                other_ambiguity[i][amb] += average;
                                other_best_ambiguity[i][amb] += best;
                                other_worst_ambiguity[i][amb] += worst;
                                other_count[i][amb]++;

                                if (c == i) {
                                    partial_average[i][amb] += average;
                                    partial_best[i][amb] += best;
                                    partial_worst[i][amb] += worst;
                                    partial_average_total[amb] += average;
                                    partial_best_total[amb] += best;
                                    partial_worst_total[amb] += worst;

                                    if (!partial_voc.contains(fathatayn) && c == 1) {
                                        partial_average[0][amb] += average;
                                        partial_best[0][amb] += best;
                                        partial_worst[0][amb] += worst;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            countEquivalent++;

            if (Has_Tanween) {
                countEquivalentTanween++;
            }
        }

        if (Has_Tanween) {
            countTanween++;
        }

        if (prg != NULL) {
            prg->report(((double)filePos) / fileSize * 100 + 0.5);
        }
    }

    for (int amb = 0; amb < ambiguitySize; amb++) {
        theSarf->displayed_error << interpret((Ambiguity)amb) << ":\n";
        theSarf->displayed_error    << "\tRecall=\t" << correctly_detected[amb] << "/" << correctly_detectedNoDiacritics[amb] <<
                                    "=\t" << correctly_detected[amb] / ((double)correctly_detectedNoDiacritics[amb]) << "\n"
                                    << "\tPrecision=\t" << correctly_detected[amb] << "/" << voc_total[amb] << "=\t" << correctly_detected[amb] / ((
                                                double)voc_total[amb]) << "\n\n";
        theSarf->displayed_error    << "\tRecall (no)=\t" << correctly_detectedNoDiacritics[amb] << "/" <<
                                    correctly_detectedNoDiacritics[amb] << "=\t" << 1.0 << "\n"
                                    << "\tPrecision (no)=\t" << correctly_detectedNoDiacritics[amb] << "/" << no_voc_total[amb] << "=\t" <<
                                    correctly_detectedNoDiacritics[amb] / ((double)no_voc_total[amb]) << "\n\n";
    }

    theSarf->displayed_error << "\nEquivalent Factor:\t" << countEquivalent << "/" << total << "=\t" << ((
                                 double)countEquivalent) / total << "\n";
    theSarf->displayed_error << "Reduction Factor:\t" << countReduced << "/" << countEquivalent << "=\t" << ((
                                 double)countReduced) / countEquivalent << "\n";
    theSarf->displayed_error << "\nEquivalent Tanween Factor:\t" << countEquivalentTanween << "/" << countTanween << "=\t"
                             << ((double)countEquivalentTanween) / countTanween << "\n";
    theSarf->displayed_error << "Reduction Tanween Factor:\t" << countReducedTanween << "/" << countEquivalentTanween <<
                             "=\t" << ((double)countReducedTanween) / countEquivalentTanween << "\n";
    int countNonReduced = countReduced - countReducedTanween;
    int countNonEquivalent = countEquivalent - countEquivalentTanween;
    theSarf->displayed_error << "Reduction Non-Tanween Factor:\t" << countNonReduced << "/" << countNonEquivalent << "=\t"
                             << ((double)countNonReduced) / countNonEquivalent << "\n";
    return 0;
}

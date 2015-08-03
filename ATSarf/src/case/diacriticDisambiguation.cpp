#include <assert.h>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include "Quadruplet.h"
#include "Triplet.h"
#include "transliteration.h"
#include "stemmer.h"
#include "morphemes.h"
#include "diacriticDisambiguation.h"
#include "vocalizedCombinations.h"

typedef QSet<VocCombIndexListPair> CombSet;
//typedef Quadruplet<QString,QString, QString,MorphemeDiacritics> TextNposNvocNmorphDiaQuadruplet;
typedef Triplet<QString, QString, MorphemeDiacritics> TextNposNmorphDiaTriplet;

void DiacriticDisambiguationBase::reset() {
    for (int i = 0; i < ambiguitySize; i++) {
        total[i] = 0;
        left[i] = 0;
        leftBranching[i] = 0;
        totalBranching[i] = 0;
        countAmbiguity[i] = 0;
        bestLeft[i] = 0;
        worstLeft[i] = 0;
        countReduced[i] = 0;
        reducingCombinations[i] = 0;
        totalCombinations[i] = 0;
    }

    currId = -1;
    countWithoutDiacritics = 0;
}

DiacriticDisambiguationBase::DiacriticDisambiguationBase(bool mapBased, bool suppressOutput, int diacriticsCount) {
    this->mapBased = mapBased;
    this->suppressOutput = suppressOutput;
    this->diacriticsCount = diacriticsCount;
    reset();
}

void DiacriticDisambiguationBase::store(long id, QString entry, AmbiguitySolution &s) {
    //assert(equal(s.voc,entry));
    if (!equal(s.voc, entry)) {
        qDebug() << "Conflict:\t" << s.voc << entry;
        return;
    }

    if (mapBased) {
        EntryAmbiguitySolutionList &l = solutionMap[id];
        l.second.append(s);

        if (l.first.isEmpty()) {
            l.first = entry;
        }

        /*else
          assert(entry==l.first);*/
    } else {
        if (currId != id) {
            if (currId >= 0) {
                analyze();
            }

            currId = id;
            currEntry = entry;
        }

        currSol.append(s);
    }
}

void DiacriticDisambiguationBase::store(long id, QString entry, QString raw_data, QString description, QString POS,
                                        Morphemes morphemes) {
    AmbiguitySolution s(raw_data, description, POS, "", morphemes);
    store(id, entry, s);
}

void DiacriticDisambiguationBase::store(long id, QString entry, QString raw_data, QString description, QString POS) {
    AmbiguitySolution s(raw_data, description, "", POS);
    store(id, entry, s);
}

DiacriticDisambiguationBase::~DiacriticDisambiguationBase() {
    analyze();

    for (int amb = 0; amb < ambiguitySize; amb++) {
        theSarf->displayed_error    << interpret((Ambiguity)amb) << ":" << "\t"
                                    << (double)(((long double)total[amb]) / countAmbiguity[amb]) << " -->\t"
                                    << (double)(left[amb] / countAmbiguity[amb]) << " =>\t"
                                    << (long long)left[amb] << "/" << total[amb] << " =\t"
                                    << (double)(left[amb] / total[amb]) << ",\t"
                                    << (long long)leftBranching[amb] << "/" << totalBranching[amb] << " =\t"
                                    << (double)((long double)leftBranching[amb] / totalBranching[amb]) << "\t"
                                    << "(" << bestLeft[amb] << "/" << total[amb] << "="
                                    << (double)(((long double)bestLeft[amb]) / total[amb]) << ","
                                    << worstLeft[amb] << "/" << total[amb] << "="
                                    << (double)(((long double)worstLeft[amb]) / total[amb]) << ")\t"
                                    << (double)((long double)countReduced[amb]) / countAmbiguity[amb] << "\t"
                                    << (double)((long double)reducingCombinations[amb]) / totalCombinations[amb] << "\n";
    }

    theSarf->displayed_error    << "\nUndiacritized:\t" << countWithoutDiacritics << "/" << countAmbiguity[All_Ambiguity] <<
                                "=\t"
                                << (double)((long double)countWithoutDiacritics) / countAmbiguity[All_Ambiguity] << "\n";
}

void DiacriticDisambiguationBase::analyze() {
    if (mapBased) {
        for (Map::iterator itr = solutionMap.begin(); itr != solutionMap.end(); itr++) {
            QString currAffix = itr->first;
            AmbiguitySolutionList currSol = itr->second;
            analyzeOne(currAffix, currSol);
        }

        solutionMap.clear();
    } else {
        analyzeOne(currEntry, currSol);
        currId = -1;
        currEntry = "";
        currSol.clear();
    }
}

void DiacriticDisambiguationBase::printDiacriticDisplay(Diacritics d, QTextStream *o) {
    if (!d.isSelfConsistent()) {
        (*o) << "~";
    } else if (d.hasShadde()) {
        (*o) << (int)SHADDA;
    } else {
        Diacritic dia = d.getMainDiacritic();

        if (dia != UNDEFINED_DIACRITICS) {
            (*o) << (int)dia;
        }
    }
}

void DiacriticDisambiguationBase::printDiacritics(QString entry, int pos, QChar c,
                                                  QTextStream *o) {  //for one diacritic
    (*o) << entry.size() << "\t" << diacriticsCount;
    Diacritics dia(c);
    (*o) << "\t" << pos << "\t";
    printDiacriticDisplay(dia, o);
}

void DiacriticDisambiguationBase::printDiacritics(QString unvoc, const QList<Diacritics> &d, AmbiguitySolution sol,
                                                  QTextStream *o) {  //for multiple diacritcs
    (*o) << sol.voc << "\t" << d.size() << "\t" << diacriticsCount;

    for (int i = 0; i < d.size(); i++) {
        const Diacritics &dia = d[i];

        if (!dia.isEmpty()) {
            (*o) << "\t" << i << "\t";
            printDiacriticDisplay(dia, o);
            int relPos, morphSize;
            MorphemeType t = sol.getMorphemeTypeAtPosition(i, dia, relPos, morphSize);
            QChar letterBefore = unvoc[i];
            (*o) << "\t" << (int)t
                 << "\t" << relPos
                 << "\t" << getRelativePos(relPos, morphSize)
                 << "\t" << letterBefore
                 << "\t" << isLongVowel(letterBefore)
                 << "\t" << isShamsi(letterBefore)
                 << "\t" << sol.stemPOS;
        }
    }
}


void insertIntoCombSet(QSet<TextNposNmorphDiaTriplet> &alreadyProceesed, CombSet &allPossibleComb,
                       VocalizedCombination  &comb, int index, AmbiguitySolution &sol) {
    QString s = comb.getString();

    if (comb.hasSelfInconsistency()) {
        qDebug() << "Found inconsistency in generated lexicon entries:\t" << s << "\tpos: " << sol.voc;
        return;
    }

    MorphemeDiacritics mD = sol.getMorphemeDiacriticSummary(comb);
    QString pos = sol.stemPOS;
    //QString voc=sol.voc;
    TextNposNmorphDiaTriplet p(s, pos/*,voc*/, mD);

    if (alreadyProceesed.contains(p)) {
        return;
    }

    alreadyProceesed.insert(p);
    CombSet::iterator itr = allPossibleComb.find(comb);

    if (itr == allPossibleComb.end()) {
        VocCombIndexListPair p(comb);
        p.indicies.insert(index);
        allPossibleComb.insert(p);
    } else {
        VocCombIndexListPair &p = (VocCombIndexListPair &)(
                                      *itr); //safe since changing p by adding index does not disrupt its position in set
        p.indicies.insert(index);
    }
}


void DiacriticDisambiguationBase::analyzeOne(QString currEntry, const AmbiguitySolutionList &currSol) {
    AmbiguitySolutionList currSolutions[ambiguitySize];

    for (int amb = 0; amb < ambiguitySize; amb++) {
        if ((Ambiguity)amb != All_Ambiguity) {
            currSolutions[amb] = getAmbiguityUnique(currSol, (Ambiguity)amb);
        } else {
            currSolutions[amb] = currSol;
        }
    }

    int sub_total[ambiguitySize] = {0}, sub_left[ambiguitySize] = {0};
    int best_sub_Left[ambiguitySize] = {0}, worst_sub_Left[ambiguitySize] = {0};

    for (int amb = 0; amb < ambiguitySize; amb++) {
        best_sub_Left[amb] = currSolutions[amb].size();
    }

    {
        CombSet allPossibleComb;
        QSet<TextNposNmorphDiaTriplet> alreadyProceesed;

        for (int j = 0; j < currSolutions[All_Ambiguity].size(); j++) { //All_Ambiguity contains all solutions
            AmbiguitySolution &sol = currSolutions[All_Ambiguity][j];
            QString voc = sol.voc;

            if (diacriticsCount > 0) {
                VocalizedCombinationsGenerator v(voc, diacriticsCount);

                if (v.isUnderVocalized()) {
                    VocalizedCombination c = VocalizedCombination::deduceCombination(voc);
                    insertIntoCombSet(alreadyProceesed, allPossibleComb, c, j, sol);
                } else {
                    for (v.begin(); !v.isFinished(); ++v) {
                        VocalizedCombination c = v.getCombination();
                        insertIntoCombSet(alreadyProceesed, allPossibleComb, c, j, sol);
                    }
                }
            } else { //i.e. all diacritics
                VocalizedCombination c = VocalizedCombination::deduceCombination(voc);
                insertIntoCombSet(alreadyProceesed, allPossibleComb, c, j, sol);
            }
        }

        CombSet::iterator itr = allPossibleComb.begin();

        for (; itr != allPossibleComb.end(); itr++) {
            VocCombIndexListPair &combIndexList = (VocCombIndexListPair &)(*itr);
            VocalizedCombination &c = combIndexList.comb;
            QString s = c.getString();
            const QList<Diacritics> &d = c.getDiacritics();
            //const DiacriticsPositionsList & diaPos=c.getShortList();
            int numDia = c.getNumDiacritics();

            if (numDia == 0) {
                continue;
            }

            double valid_ratio[ambiguitySize];

            for (int amb = 0; amb < ambiguitySize; amb++) {
                int valid_count = 0;

                for (int j = 0; j < currSolutions[amb].size(); j++) {
                    if (equal(s, currSolutions[amb][j].voc, true)) {
                        valid_count++;
                    }
                }

                valid_ratio[amb] = ((double)valid_count) / (currSolutions[amb].size());
                sub_left[amb] += valid_count;
                worst_sub_Left[amb] = max(worst_sub_Left[amb], valid_count);
                best_sub_Left[amb] = min(best_sub_Left[amb], valid_count);
                sub_total[amb] += currSolutions[amb].size();

                if (valid_ratio[amb] < 1) {
                    reducingCombinations[amb]++;
                }

                totalCombinations[amb]++;
            }

#ifndef REDUCED_EQ_AMBIGUIOUS
            bool reduced = valid_ratio[All_Ambiguity] < 1;
#else
            bool reduced = currSolutions[Vocalization].size() > 1;
#endif
            bool display = ((numDia == diacriticsCount || diacriticsCount == -1) && !suppressOutput && reduced);

            for (int i = 0; i < 2; i++) {
                QTextStream *o = (i == 0 ? & (theSarf->out) : &hadith_out);

                if (display || i == 1) { //always display for hadith_out
                    int total = combIndexList.indicies.size();

                    for (QSet<int>::iterator itr = combIndexList.indicies.begin(); itr != combIndexList.indicies.end(); ++itr) {
                        int index = *itr;
                        AmbiguitySolution &sol =
                            currSolutions[All_Ambiguity][index]; //make instead of indicies in general to indicies to uniques ones out of All_Ambiguity
                        (*o) << s << "\t";
                        printDiacritics(currEntry, d, sol, o);
                        int vocLeft = currSolutions[Vocalization].size();
                        (*o) << "\t" << vocLeft << "\t" << (1.0 / ((double)total));

                        for (int amb = 0; amb < ambiguitySize; amb++) {
                            (*o) << "\t" << valid_ratio[amb];
                        }

                        (*o) << "\n";
                    }
                }
            }
        }

        if (allPossibleComb.size() == 0) {
            return;
        }
    }

    for (int amb = 0; amb < ambiguitySize; amb++) {
        if (sub_total[amb] == 0) {
            if (amb == (int)All_Ambiguity && !currEntry.isEmpty()) {
                countWithoutDiacritics++;
            }
        } else {
            /*if (diacriticsCount<0) {
              qDebug()<<sub_total[amb]<<" vs "<<currSolutions[amb].size();
              assert(sub_total[amb]==currSolutions[amb].size());
              }*/
            left[amb] += ((double)sub_left[amb]) / sub_total[amb] * currSolutions[amb].size();
            worstLeft[amb] += worst_sub_Left[amb];
            bestLeft[amb] += best_sub_Left[amb];
            leftBranching[amb] += sub_left[amb];
            totalBranching[amb] += sub_total[amb];
            total[amb] += currSolutions[amb].size();
        }

        assert(totalBranching[amb] > 0);

        if (currSolutions[amb].size() != 0) {
            countAmbiguity[amb]++;

            if (sub_left[amb] < sub_total[amb]) {
                countReduced[amb]++;
            }
        }
    }
}

void AffixDisambiguation::visit(node *n, QString affix, QString raw_data, long /*category_id*/, QString description,
                                QString POS) {
    store((long)n, affix, raw_data, description, POS);
}

void StemDisambiguation::operator()() {
    for (int i = 0; i < stemNodes->size(); i++) {
        StemNode &n = (*stemNodes)[i];
        long stem_id = n.stem_id;
        StemNode::CategoryVector &cats = n.category_ids;

        for (int c = 0; c < cats.size(); c++) {
            long cat_id = cats[c];
            //QString cat=database_info.comp_rules->getCategoryName(cat_id);
            StemNode::RawDatasEntry &raws = n.raw_datas[c];

            for (int r = 0; r < raws.size(); r++) {
                QString raw = raws[r];
                QString stem = removeDiacritics(raw); //bc currently n.key left empty
                ItemEntryKey entry(stem_id, cat_id, raw);
                ItemCatRaw2AbsDescPosMapItr itr = map->find(entry);

                while (itr != map->end() && itr.key() == entry) {
                    //dbitvec d=itr.value().first;
                    QString pos = itr.value().third;
                    long desc_id = itr.value().second;
                    QString desc;

                    if (desc_id >= 0) {
                        desc = (*database_info.descriptions)[desc_id];
                    }

                    store(stem_id, stem, raw, desc, pos);
                    itr++;
                }
            }
        }
    }
}

void DisambiguationStemmer::store(QString entry, AmbiguitySolution &s) {
    storage.store(id, entry, s);
}

FullListDisambiguation::FullListDisambiguation(QStringList &inputList, ATMProgressIFC *prg,
                                               int numDiacritcs): DiacriticDisambiguationBase(false, true, numDiacritcs), list(inputList) {
    this->prg = prg;
}

void FullListDisambiguation::operator()() {
    QString unvoc;
    int count = 0;
    long size = list.size();

    foreach (unvoc, list) {
        DisambiguationStemmer s(count, unvoc, *this);
        s();
        count++;
        prg->report((((double)count) / size) * 100 + 0.5);
    }
}

FullListDisambiguation::~FullListDisambiguation() {
    analyze();
}

FullFileDisambiguation::FullFileDisambiguation(QString inputFileName, ATMProgressIFC *prg, int numDiacritcs,
                                               QString reducedFileName, QString allFileName): DiacriticDisambiguationBase(false, true, numDiacritcs) {
    this->inputFileName = inputFileName;
    this->reducedFileName = reducedFileName;
    this->allFileName = allFileName;
    this->prg = prg;

    if (!allFileName.isEmpty() || !reducedFileName.isEmpty()) {
        suppressOutput = false;
    }
}

void FullFileDisambiguation::operator()() {
    if (!reducedFileName.isEmpty()) {
        QFile::remove(reducedFileName);
        reducedFile.setFileName(reducedFileName);
        assert(reducedFile.open(QFile::ReadWrite));
        oldDevice = theSarf->out.device();
        theSarf->out.setDevice(&reducedFile);
        theSarf->out.setCodec("utf-8");
        theSarf->out <<
                     "partial_voc\tfull_voc\tsize\tnumDia\tposDia\tdia\tmorph\tmorphAbs\tMorphRel\tletter\tvowel\tshamsi\tPOS\tVocLeft\tVOC_A\tGloss_A\tPOS_A\tToken_A\tStem_A\tAll_A\n";
    }

    if (!allFileName.isEmpty()) {
        QFile::remove(allFileName);
        allFile.setFileName(allFileName);
        assert(allFile.open(QFile::ReadWrite));
        oldDeviceAll = hadith_out.device();
        hadith_out.setDevice(&allFile);
        hadith_out.setCodec("utf-8");
        hadith_out <<
                   "partial_voc\tfull_voc\tsize\tnumDia\tposDia\tdia\tmorph\tmorphRel\tMorphAbs\tletter\tvowel\tshamsi\tPOS\tVocLeft\tVOC_A\tGloss_A\tPOS_A\tToken_A\tStem_A\tAll_A\n";
    }

    QFile input(inputFileName);

    if (!input.open(QIODevice::ReadOnly)) {
        _error << "File not found\n";
        return;
    }

    QTextStream file(&input);
    //file.setCodec("utf-8");
    long count = 0;
    long size = input.size();
    long pos = 0;

    while (!file.atEnd()) {
        QString line = file.readLine(0);

        if (line == "") {
            continue;
        }

        QString unvoc = Buckwalter::convertFrom(line);
        DisambiguationStemmer s(count, unvoc, *this);
        s();
        //analyze();
        count++;
        pos += line.size() + 1; //to account for '\n'
        prg->report((((double)pos) / size) * 100 + 0.5);
    }

    input.close();
}

FullFileDisambiguation::~FullFileDisambiguation() {
    analyze();

    if (reducedFile.isOpen()) {
        reducedFile.close();
    }

    theSarf->out.setDevice(oldDevice);

    if (allFile.isOpen()) {
        allFile.close();
    }

    hadith_out.setDevice(oldDeviceAll);
}


void diacriticDisambiguationCount(item_types t, int numDiacritics = 1) {
    if (t == STEM) {
        StemDisambiguation d(numDiacritics);
        d();
    } else {
        AffixDisambiguation d(t, numDiacritics);
        d();
    }
}

void diacriticDisambiguationCount(QString fileName, int numDiacritics, ATMProgressIFC *prg,
                                  QString reducedFile = "reducedOutput", QString allFile = "fullOutput") {
    FullFileDisambiguation d(fileName, prg, numDiacritics, reducedFile, allFile);
    d();
}

void diacriticDisambiguationCount(QStringList &list, int numDiacritics, ATMProgressIFC *prg) {
    FullListDisambiguation d(list, prg, numDiacritics);
    d();
}


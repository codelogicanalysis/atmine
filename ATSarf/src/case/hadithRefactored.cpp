#if 1
#include "hadith.h"

#ifdef GENERAL_HADITH
#include <QTextBrowser>
#include <assert.h>

#include "ATMProgressIFC.h"
#include "Math_functions.h"
#include "graph.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "hadithCommon.h"

#define display(c)

typedef int (*functionUsingChains_t)(ChainsContainer &, ATMProgressIFC *, QString);

class HadithSegmentor {
    private:
        QString fileName;
        StateData currentData;
        QString *text;
        //long current_pos;


        int segmentHelper(QString *text, int start, int end, functionUsingChains_t functionUsingChains, ATMProgressIFC *prg,
                          bool segmentNarrators) {
            QFile chainOutput(chainDataStreamFileName);
            chainOutput.remove();

            if (!chainOutput.open(QIODevice::ReadWrite)) {
                return 1;
            }

            QDataStream chainOut(&chainOutput);

            if (text == NULL) {
                return -1;
            }

            long text_size = min(text->size(), end + 1);
#ifdef COUNT_AVERAGE_SOLUTIONS
            total_solutions = 0;
            stemmings = 0;
#endif
            long  newHadithStart = -1;
            currentData.initialize();
#ifdef CHAIN_BUILDING
            HadithData *currentChain = new HadithData(text, true, NULL, fileName);
            currentChain->initialize(text);
            currentChain->segmentNarrators = segmentNarrators;
            display(QString("\ninit0\n"));
#else
            chainData *currentChain = NULL;
#endif
            long  sanadEnd;
            int hadith_Counter = 1;
            StateInfo stateInfo;
            stateInfo.resetCurrentWordInfo();
            stateInfo.currentState = TEXT_S;
            stateInfo.nextState = TEXT_S;
            stateInfo.lastEndPos = start;
            stateInfo.startPos = start;
            stateInfo.nrcPreviousType = false;
            stateInfo.processedStructure = INITIALIZE;
            stateInfo.previousPunctuationInfo.fullstop = true;

            while (stateInfo.startPos < text_size && isDelimiter(text->at(stateInfo.startPos))) {
                stateInfo.startPos++;
            }

#ifdef PROGRESSBAR
            prg->setCurrentAction("Parsing Hadith");
#endif

            for (; stateInfo.startPos < text_size;) {
                if ((proceedInStateMachine(stateInfo, currentChain, currentData) == false)) {
                    assert(currentChain->narrator == NULL);

                    if (currentData.narratorCount >= hadithParameters.narr_min) {
                        if (!segmentNarrators) {
                            sanadEnd = currentData.narratorEndIndex;
#ifdef DISPLAY_HADITH_OVERVIEW
                            newHadithStart = currentData.mainStructureStartIndex;
                            //long end=text->indexOf(QRegExp(delimiters),sanadEnd);//sanadEnd is first letter of last word in sanad
                            //long end=stateInfo.endPos;
                            theSarf->out << "\n" << hadith_Counter << " new hadith start: " << text->mid(newHadithStart, display_letters) << endl;
                            theSarf->out << "sanad end: " << text->mid(sanadEnd - display_letters + 1, display_letters) << endl << endl;
                        }

#ifdef CHAIN_BUILDING
                        currentChain->chain->serialize(chainOut);
                        //currentChain->chain->serialize(displayed_error);
#endif
#endif
                        hadith_Counter++;
                    }
                }

                stateInfo.currentState = stateInfo.nextState;
                stateInfo.startPos = stateInfo.nextPos;
                stateInfo.lastEndPos = stateInfo.endPos;
                stateInfo.previousPunctuationInfo = stateInfo.currentPunctuationInfo;

                if (stateInfo.number) {
                    stateInfo.previousPunctuationInfo.fullstop = true;
                    stateInfo.previousPunctuationInfo.has_punctuation = true;
                }

                if (stateInfo.previousPunctuationInfo.has_punctuation) {
                    stateInfo.previousPunctuationInfo.fullstop = true;
                }

#ifdef PROGRESSBAR
                prg->report((double)stateInfo.startPos / text_size * 100 + 0.5);

                if (stateInfo.startPos == text_size - 1) {
                    break;
                }

#endif
            }

#ifdef NONCONTEXT_LEARNING

            if (!currentChain->segmentNarrators) {
                currentChain->learningEvaluator.displayNameLearningStatistics();
                currentChain->learningEvaluator.resetLearnedNames();
            }

#endif
            prg->report(100);
#if defined(DISPLAY_HADITH_OVERVIEW)

            if (!segmentNarrators && newHadithStart < 0) {
                theSarf->out << "no hadith found\n";
                chainOutput.close();
                return 2;
            }

            chainOutput.close();
#endif
#ifdef CHAIN_BUILDING //just for testing deserialize
            QFile f("hadith_chains.txt");

            if (!f.open(QIODevice::WriteOnly)) {
                return 1;
            }

            QTextStream file_hadith(&f);
            file_hadith.setCodec("utf-8");

            if (!chainOutput.open(QIODevice::ReadWrite)) {
                return 1;
            }

            QDataStream tester(&chainOutput);
            int tester_Counter = 1;
#ifdef TEST_NARRATOR_GRAPH
            ChainsContainer chains;
            chains.clear();
#endif
#if defined(TAG_HADITH)
            prg->startTaggingText(*text);
#endif

            while (!tester.atEnd()) {
                Chain *s = new Chain(text);
                s->deserialize(tester);
#ifdef TEST_NARRATOR_GRAPH
                chains.append(s);
#endif
#if defined(TAG_HADITH)

                for (int j = 0; j < s->m_chain.size(); j++) {
                    ChainPrim *curr_struct = s->m_chain[j];

                    if (curr_struct->isNarrator()) {
                        Narrator *n = (Narrator *)curr_struct;

                        if (n->m_narrator.size() == 0) {
                            theSarf->out << "found a problem an empty narrator in (" << tester_Counter << "," << j << ")\n";
                            continue;
                        }

                        prg->tag(curr_struct->getStart(), curr_struct->getLength(), Qt::darkYellow, false);

                        for (int i = 0; i < n->m_narrator.size(); i++) {
                            NarratorPrim *nar_struct = n->m_narrator[i];

                            if (nar_struct->isNamePrim()) {
                                if (((NamePrim *)nar_struct)->learnedName) {
                                    prg->tag(nar_struct->getStart(), nar_struct->getLength(), Qt::blue, true);
                                    //error<<nar_struct->getString()<<"\n";
                                } else {
                                    prg->tag(nar_struct->getStart(), nar_struct->getLength(), Qt::white, true);
                                }
                            } else if (((NameConnectorPrim *)nar_struct)->isFamilyConnector()) {
                                prg->tag(nar_struct->getStart(), nar_struct->getLength(), Qt::darkRed, true);
                            } else if (((NameConnectorPrim *)nar_struct)->isPossessive()) {
                                prg->tag(nar_struct->getStart(), nar_struct->getLength(), Qt::darkMagenta, true);
                            }
                        }
                    } else {
                        prg->tag(curr_struct->getStart(), curr_struct->getLength(), Qt::gray, false);
                    }
                }

#else
                hadith_out << tester_Counter << " ";
                s->serialize(hadith_out);
#endif
                tester_Counter++;
                s->serialize(file_hadith);
            }

            chainOutput.close();
            f.close();
#ifdef TEST_NARRATOR_GRAPH
            (*functionUsingChains)(chains, prg, fileName);
#endif
#endif
#ifndef TAG_HADITH
#if 0
            prg->startTaggingText(*hadith_out.string()); //we will not tag but this will force a text to be written there
#endif
#else
            prg->finishTaggingText();
#endif
#endif
#ifdef COUNT_AVERAGE_SOLUTIONS
            displayed_error << (double)(total_solutions / (long double)stemmings) << "\n"
                            << stemmings << "\n";
#endif
            //delete text;
            /*if (currentChain!=NULL)
              delete currentChain;*/
            return 0;
        }

    public:
        int segment(QString input_str, int (*functionUsingChains)(ChainsContainer &, ATMProgressIFC *, QString),
                    ATMProgressIFC *prg)  {
            fileName = input_str;
            QFile input(input_str);

            if (!input.open(QIODevice::ReadOnly)) {
                theSarf->out << "File not found\n";
                return 1;
            }

            QTextStream file(&input);
            file.setCodec("utf-8");
            text = new QString(file.readAll());

            if (text->isNull()) {
                theSarf->out << "file error:" << input.errorString() << "\n";
                return 1;
            }

            if (text->isEmpty()) {//ignore empty files
                theSarf->out << "empty file\n";
                return 0;
            }

            return segmentHelper(text, 0, text->size() - 1, functionUsingChains, prg, false);
        }
        int segment(QString *text, int (*functionUsingChains)(ChainsContainer &, ATMProgressIFC *, QString),
                    ATMProgressIFC *prg, bool segmentNarrators = false)  {
            fileName = "";
            return segmentHelper(text, 0, text->size() - 1, functionUsingChains, prg, segmentNarrators);
        }
        int segment(QString *text, int start, int end, int (*functionUsingChains)(ChainsContainer &, ATMProgressIFC *, QString),
                    ATMProgressIFC *prg, bool segmentNarrators = false)  {
            fileName = "";
            return segmentHelper(text, start, end, functionUsingChains, prg, segmentNarrators);
        }
};

#ifdef IMAN_CODE
class adjective_stemmer: public Stemmer {
    public:
        bool adj ;
        long finish_pos;

        adjective_stemmer(QString *word, int start): Stemmer(word, start/*,false*/) {
            adj = false;
            finish_pos = start;
        }
        bool on_match() {
            for (unsigned int i = 0; i < stem_info->abstract_categories.length(); i++)
                if (stem_info->abstract_categories[i] && get_abstractCategory_id(i) >= 0) {
                    if (getColumn("category", "name", get_abstractCategory_id(i)) == "ADJ") {
                        adj = true;
                        finish_pos = info.finish;
                        return false;
                    }
                }

            return true;
        }
};

int adjective_detector(QString input_str) {
    QFile input(input_str);

    if (!input.open(QIODevice::ReadWrite)) {
        out << "File not found\n";
        return 1;
    }

    QTextStream file(&input);
    file.setCodec("utf-8");
    text = new QString(file.readAll());

    if (text->isNull()) {
        out << "file error:" << input.errorString() << "\n";
        return 1;
    }

    if (text->isEmpty()) { //ignore empty files
        out << "empty file\n";
        return 0;
    }

    long text_size = text->size();

    while (current_pos < text->length() && delimiters.contains(text->at(current_pos))) {
        current_pos++;
    }

    for (; current_pos < text_size;) {
        adjective_stemmer s(text, current_pos);
        s();
        long finish = max(s.info.finish, s.finish_pos);

        if (s.adj) {
            out << text->mid(current_pos, finish - current_pos + 1) + ":ADJECTIVE\n";
        }

        current_pos = next_positon(finish);; //here current_pos is changed
    }

    return 0;
}
#endif

int hadithHelper(QString input_str, ATMProgressIFC *prg) {
    input_str = input_str.split("\n")[0];
#ifdef IMAN_CODE
    return adjective_detector(input_str);
#endif
    HadithSegmentor s;
    s.segment(input_str, &test_GraphFunctionalities, prg);
    return 0;
}

int segmentNarrators(QString *text, int start, int end, int (*functionUsingChains)(ChainsContainer &, ATMProgressIFC *,
                     QString), ATMProgressIFC *prg) {
    int narr_min = hadithParameters.narr_min;
    hadithParameters.narr_min = 0;
    //int size=text->size();
    QString addedText = QString(" stop ").repeated(hadithParameters.nmc_max +/*2*/7);
    text->insert(end + 1, addedText);
    //qDebug()<<*text;
    HadithSegmentor s;
    s.segment(text, start, end + addedText.size() + 1, functionUsingChains, prg, true);
    hadithParameters.narr_min = narr_min;
    text->remove(end + 1, addedText.size());
    //qDebug()<<*text;
    return 0;
}


int segmentNarrators(QString *text, int (*functionUsingChains)(ChainsContainer &, ATMProgressIFC *, QString),
                     ATMProgressIFC *prg) {
    return segmentNarrators(text, 0, text->size() - 1, functionUsingChains, prg);
}



#endif

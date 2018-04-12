#ifndef NARRATORDETECTOR_H
#define NARRATORDETECTOR_H

#include "hadith.h"
#include "hadithCommon.h"
#include "narrator_abstraction.h"

class NarratorGraph;

typedef QList<Biography *> BiographyList;

int biographyHelper(QString input_str, ATMProgressIFC *prg);
BiographyList *getBiographies(QString input_str, NarratorGraph *graph, ATMProgressIFC *prg, int nodeId = -1);

#endif

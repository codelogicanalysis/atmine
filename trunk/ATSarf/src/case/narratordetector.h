#ifndef NARRATORDETECTOR_H
#define NARRATORDETECTOR_H

#include "hadith.h"
#include "hadithCommon.h"
#include "narrator_abstraction.h"

class NarratorGraph;

typedef QList<Biography*> BiographyList;

int biographyHelper(QString input_str,ATMProgressIFC *prg);
#ifdef TEST_BIOGRAPHIES
BiographyList * getBiographies(QString input_str,NarratorGraph* graph,ATMProgressIFC *prg);
#endif

#endif // NARRATORDETECTOR_H

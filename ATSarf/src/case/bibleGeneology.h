#ifndef BIBLEGENEOLOGY_H
#define BIBLEGENEOLOGY_H

#include <QString>
#include "common.h"
#include "ATMProgressIFC.h"

#define GET_WAW
#define TRUST_OLD
#define GENEOLOGYDEBUG

class GeneologyParameters {
public:
	unsigned int theta_0:10;
	unsigned int N_min:10;
	unsigned int unused:12;
	GeneologyParameters() {
		theta_0=30;
		N_min=5;
	}
};

extern GeneologyParameters geneologyParameters;

int genealogyHelper(QString input_str,ATMProgressIFC *prgs);
void geneology_initialize();

#endif // BIBLEGENEOLOGY_H

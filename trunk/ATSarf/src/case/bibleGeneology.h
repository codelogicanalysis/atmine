#ifndef BIBLEGENEOLOGY_H
#define BIBLEGENEOLOGY_H

#include <QString>
#include "common.h"
#include "ATMProgressIFC.h"

#define GET_WAW
//#define TRUST_OLD
#define GENEOLOGYDEBUG
//#define DISPLAY_INDIVIDUAL
#define REDUCE_AFFIX_SEARCH

class GeneologyParameters {
public:
	unsigned int theta_0:10;
	unsigned int N_min:8;
	unsigned int C_max:8;
	unsigned int radius:8;
	GeneologyParameters() {
		theta_0=35;
		N_min=3;
		C_max=3;
		radius=3;
	}
};

extern GeneologyParameters geneologyParameters;

int genealogyHelper(QString input_str,ATMProgressIFC *prgs);
void geneology_initialize();

#endif // BIBLEGENEOLOGY_H

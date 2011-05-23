#ifndef TIMERECOGNIZER_H
#define TIMERECOGNIZER_H

#include "ATMProgressIFC.h"

//#define TIMEDEBUG
#define GET_AFFIXES_ALSO

class TimeParameters
{
public:
	unsigned int num_before_absolute:5;
	unsigned int num_after_absolute:5;
	unsigned int unused_int:22;
	TimeParameters() {
		num_before_absolute=1;
		num_after_absolute =2;
	}
};

void time_initialize();

int timeRecognizeHelper(QString input_str,ATMProgressIFC *prg);

#endif // TIMERECOGNIZER_H

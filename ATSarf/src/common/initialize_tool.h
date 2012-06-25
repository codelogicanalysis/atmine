#ifndef INITIALIZE_TOOL_H
#define INITIALIZE_TOOL_H

#include <QFile>
#include "ATMProgressIFC.h"

bool sarfStart(QFile * _out=NULL, QFile * _displayed_error=NULL, ATMProgressIFC * pIFC=NULL);  // must be called for tool general initialization

bool sarfExit();

#endif // INITIALIZE_TOOL_H

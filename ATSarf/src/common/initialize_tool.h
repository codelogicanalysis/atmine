#ifndef INITIALIZE_TOOL_H
#define INITIALIZE_TOOL_H

#include <QFile>
#include "ATMProgressIFC.h"

bool initialize_tool(QFile * _out, QFile * _displayed_error, ATMProgressIFC * pIFC);  // must be called for tool general initialization
#endif // INITIALIZE_TOOL_H

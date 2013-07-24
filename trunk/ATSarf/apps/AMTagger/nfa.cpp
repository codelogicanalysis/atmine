#include "nfa.h"

NFA::NFA(QString name)
{
    this->name = name;
    start = "";
    accept = "";
    last = "";
    i=0;
    actionStack = NULL;
}

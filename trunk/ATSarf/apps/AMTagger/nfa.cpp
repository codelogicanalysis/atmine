#include "nfa.h"

NFA::NFA(MSF* formula)
{
    this->formula = formula;
    start = "";
    accept = "";
    last = "";
    i=0;
    actionStack = NULL;
}

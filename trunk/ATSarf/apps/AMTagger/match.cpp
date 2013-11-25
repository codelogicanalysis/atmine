#include "match.h"

Match::Match(Operation op, Match *parent)
{
    this->parent = parent;
    this->op = op;
    this->msf = NULL;
}

Match::~Match() {

}

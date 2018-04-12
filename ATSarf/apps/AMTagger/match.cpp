#include "match.h"

Match::Match(Operation op, Match *parent, int id, QString sourceText)
{
    this->parent = parent;
    this->op = op;
    this->msf = NULL;
    this->id = id;
    this->sourceText = sourceText;
}

Match::~Match() {

}

#include "unaryf.h"

UNARYF::UNARYF(QString name, Operation op, int limit) : MSF(name) {
    this->op = op;
    this->limit = limit;
}

void UNARYF::addMSF(MSF * msf) {
    vector.append(msf);
}

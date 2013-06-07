#include "binaryf.h"

BINARYF::BINARYF(QString name, Operation op): MSF(name) {
    this->op = op;
}

void BINARYF::addLeftMSF(MSF * msf) {
    lvector.append(msf);
}

void BINARYF::addRightMSF(MSF * msf) {
    rvector.append(msf);
}

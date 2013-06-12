#include "mbf.h"

MBF::MBF(QString name, MSF* parent, QString bf) : MSF(name,parent)
{
    this->bf = bf;
}

bool MBF::isMBF() {
    return true;
}

bool MBF::isUnary() {
    return false;
}

bool MBF::isBinary() {
    return false;
}

bool MBF::isFormula() {
    return false;
}

bool MBF::isSequential() {
    return false;
}

QString MBF::print() {
    return bf;
}

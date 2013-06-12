#include "binaryf.h"

BINARYF::BINARYF(QString name, MSF* parent, Operation op): MSF(name,parent) {
    this->op = op;
}

void BINARYF::setLeftMSF(MSF * msf) {
    leftMSF = msf;
}

void BINARYF::setRightMSF(MSF * msf) {
    rightMSF = msf;
}

bool BINARYF::isMBF() {
    return false;
}

bool BINARYF::isUnary() {
    return false;
}

bool BINARYF::isBinary() {
    return true;
}

bool BINARYF::isFormula() {
    return false;
}

bool BINARYF::isSequential() {
    return false;
}

QString BINARYF::print() {
    QString value = "(";
    value.append(leftMSF->print());
    value.append(")");
    if(op == AND) {
        value.append("&");
    }
    else {
        value.append("|");
    }
    value.append("(");
    value.append(rightMSF->print());
    value.append(")");
    return value;
}

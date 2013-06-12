#include "unaryf.h"

UNARYF::UNARYF(QString name, MSF* parent, Operation op, int limit) : MSF(name,parent) {
    this->op = op;
    this->limit = limit;
}

void UNARYF::setMSF(MSF * msf) {
    this->msf = msf;
}

bool UNARYF::isMBF() {
    return false;
}

bool UNARYF::isUnary() {
    return true;
}

bool UNARYF::isBinary() {
    return false;
}

bool UNARYF::isFormula() {
    return false;
}

bool UNARYF::isSequential() {
    return false;
}

QString UNARYF::print() {
    QString value = "(";
    value.append(msf->print());
    value.append(")");
    if(op == STAR) {
        value.append("*");
    }
    else if(op == PLUS) {
        value.append("+");
    }
    else if(op == UPTO) {
        value.append("^");
        value.append(QString::number(limit));
    }
    else {
        value.append("?");
    }
    return value;
}

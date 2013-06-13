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

void UNARYF::buildTree(QTreeWidgetItem* parent) {
    QStringList data;
    QString opText = "";
    if(op == STAR) {
        opText = "*";
    }
    else if(op == PLUS) {
        opText = "+";
    }
    else if(op == UPTO) {
        opText = "^";
        opText.append(QString::number(limit));
    }
    else if(op == KUESTION) {
        opText = "?";
    }
    data << name << QString() << opText;
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, data);
    msf->buildTree(item);
}

void UNARYF::buildTree(QTreeWidget* parent) {
    QStringList data;
    QString opText = "";
    if(op == STAR) {
        opText = "*";
    }
    else if(op == PLUS) {
        opText = "+";
    }
    else if(op == UPTO) {
        opText = "^";
        opText.append(QString::number(limit));
    }
    else if(op == KUESTION) {
        opText = "?";
    }
    data << name << QString() << opText;
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, data);
    msf->buildTree(item);
}

QVariantMap UNARYF::getJSON() {
    QVariantMap uMap;
    uMap.insert("name", name);
    uMap.insert("type","unary");
    uMap.insert("parent", parent->name);
    QString opText = "";
    if(op == STAR) {
        opText = "*";
    }
    else if(op == PLUS) {
        opText = "+";
    }
    else if(op == UPTO) {
        opText = "^";
        opText.append(QString::number(limit));
    }
    else if(op == KUESTION) {
        opText = "?";
    }
    uMap.insert("op",opText);
    uMap.insert("limit",limit);
    uMap.insert("MSF",msf->getJSON());
    return uMap;
}

UNARYF::~UNARYF() {
    delete msf;
}

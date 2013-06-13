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

void BINARYF::buildTree(QTreeWidgetItem* parent) {
    QStringList data;
    QString opText = "";
    if(op == OR) {
        opText = "|";
    }
    else if(op == AND) {
        opText = "&";
    }

    data << name << QString() << opText;
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, data);
    leftMSF->buildTree(item);
    rightMSF->buildTree(item);
}

void BINARYF::buildTree(QTreeWidget* parent) {
    QStringList data;
    QString opText = "";
    if(op == OR) {
        opText = "|";
    }
    else if(op == AND) {
        opText = "&";
    }

    data << name << QString() << opText;
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, data);
    leftMSF->buildTree(item);
    rightMSF->buildTree(item);
}

QVariantMap BINARYF::getJSON() {
    QVariantMap bMap;
    bMap.insert("name",name);
    bMap.insert("type","binary");
    bMap.insert("parent", parent->name);
    QString opText = "";
    if(op == OR) {
        opText = "|";
    }
    else if(op == AND) {
        opText = "&";
    }
    bMap.insert("op",opText);
    bMap.insert("leftMSF",leftMSF->getJSON());
    bMap.insert("rightMSF",rightMSF->getJSON());
    return bMap;
}

BINARYF::~BINARYF() {
    delete leftMSF;
    delete rightMSF;
}

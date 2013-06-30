#include "binaryf.h"

BINARYF::BINARYF(QString name, MSF* parent, Operation op): MSF(name,parent) {
    this->op = op;
    leftMSF = NULL;
    rightMSF = NULL;
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

bool BINARYF::buildNFA(NFA *nfa) {

    if(nfa->start.isEmpty()) {
        QString state1 = "q";
        state1.append(QString::number(nfa->i));
        nfa->start = state1;
        nfa->last = state1;
        (nfa->i)++;
    }

    QString currentLast = "q";
    currentLast.append(QString::number(nfa->i));
    (nfa->i)++;

    QString currentStart = nfa->last;

    if(op == OR) {

        if(!(leftMSF->buildNFA(nfa))) {
            return false;
        }
        nfa->transitions.insert(nfa->last + '|' + "epsilon", currentLast);
        nfa->last = currentStart;

        if(!(rightMSF->buildNFA(nfa))) {
            return false;
        }
        nfa->transitions.insert(nfa->last + '|' + "epsilon", currentLast);
        nfa->last = currentLast;
        nfa->accept = currentLast;
        return true;
    }
    else if(op == AND) {

        QString andAccept = "q";
        andAccept.append(QString::number(nfa->i));
        (nfa->i)++;
        nfa->andAccept.append(andAccept);

        if(!(leftMSF->buildNFA(nfa))) {
            return false;
        }
        nfa->transitions.insert(nfa->last + '|' + "epsilon", andAccept);
        nfa->last = currentStart;

        if(!(rightMSF->buildNFA(nfa))) {
            return false;
        }
        nfa->transitions.insert(nfa->last + '|' + "epsilon", andAccept);
        nfa->last = currentLast;
        nfa->accept = currentLast;

        nfa->transitions.insert(currentStart + '|' + "*AND*", currentLast);
        return true;
    }
    return false;
}

bool BINARYF::removeSelfFromMap(QMap<QString, MSF*> &map) {
    if(!(leftMSF->removeSelfFromMap(map))) {
        return false;
    }
    if(!(rightMSF->removeSelfFromMap(map))) {
        return false;
    }
    int count = map.remove(name);
    if(count > 0) {
        return true;
    }
    else {
        return false;
    }
}

BINARYF::~BINARYF() {
    delete leftMSF;
    delete rightMSF;
}

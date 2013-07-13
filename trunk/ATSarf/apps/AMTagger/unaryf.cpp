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

QString UNARYF::printwithNames() {
    QString value = name;
    value.append("=((");
    value.append(msf->printwithNames());
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
    value.append(')');
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
    uMap.insert("init", init);
    uMap.insert("actions",actions);
    uMap.insert("after", after);
    uMap.insert("returns", returns);
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

bool UNARYF::buildNFA(NFA *nfa) {

    if(op == PLUS) {
        if(!(msf->buildNFA(nfa))) {
            return false;
        }
    }

    //if(nfa->start.isEmpty()) {
    QString state1 = "q";
    state1.append(QString::number(nfa->i));
    if(nfa->start.isEmpty()) {
        nfa->start = state1;
    }
    else {
        nfa->transitions.insert(nfa->last + "|epsilon", state1);
    }
    //nfa->start = state1;
    nfa->last = state1;
    (nfa->i)++;
    //}
    /// Need to save this to connect to last state after addition of MSF
    QString currentStart = nfa->last;
    QString nextState = "q";
    nextState.append(QString::number(nfa->i));

    if(!(msf->buildNFA(nfa))) {
        return false;
    }

    QString currentLast = "q";
    currentLast.append(QString::number(nfa->i));
    (nfa->i)++;
    nfa->transitions.insert(currentStart + '|' + "epsilon",currentLast);
    nfa->transitions.insert(nfa->last + '|' + "epsilon", currentLast);

    if(op == KUESTION) {
        nfa->last = currentLast;
        nfa->accept = currentLast;
        return true;
    }

    if(op == PLUS || op == STAR) {
        nfa->transitions.insert(nfa->last + '|' + "epsilon", nextState);
        nfa->last = currentLast;
        nfa->accept = currentLast;
        return true;
    }

    nfa->last = currentStart;
    int newLimit = limit - 1;
    for(int i=0; i<newLimit; i++) {
        for(int j=0; j<i; j++) {
            if(!(msf->buildNFA(nfa))) {
                return false;
            }
        }
        nfa->transitions.insert(nfa->last + '|' + "epsilon", currentLast);
        nfa->last = currentStart;
    }
    nfa->last = currentLast;
    nfa->accept = currentLast;
    return true;
}

bool UNARYF::removeSelfFromMap(QMap<QString, MSF*> &map) {
    if(!(msf->removeSelfFromMap(map))) {
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

QStringList UNARYF::getMSFNames() {
    QStringList list(name);
    list += msf->getMSFNames();
    return list;
}

UNARYF::~UNARYF() {
    delete msf;
}

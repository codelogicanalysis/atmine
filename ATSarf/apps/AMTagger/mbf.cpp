#include "mbf.h"
#include "global.h"

MBF::MBF(QString name, MSF* parent, QString bf, bool isF) : MSF(name,parent)
{
    this->bf = bf;
    if(isF) {
        this->isF = true;
    }
    else {
        this->isF = false;
    }
}

bool MBF::isMBF() {
    return !isF;
}

bool MBF::isUnary() {
    return false;
}

bool MBF::isBinary() {
    return false;
}

bool MBF::isFormula() {
    return isF;
}

bool MBF::isSequential() {
    return false;
}

QString MBF::print() {
    return bf;
}

QString MBF::printwithNames() {
    QString value = name;
    value.append('=');
    value.append(bf);
    return value;
}

void MBF::buildTree(QTreeWidgetItem* parent) {
    QStringList data;
    data << name << bf << QString();
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, data);
}

void MBF::buildTree(QTreeWidget* parent) {
    QStringList data;
    data << name << bf << QString();
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, data);
}

QVariantMap MBF::getJSON() {
    QVariantMap mbfMap;
    mbfMap.insert("name", name);
    mbfMap.insert("type","mbf");
    mbfMap.insert("init", init);
    mbfMap.insert("actions",actions);
    mbfMap.insert("after", after);
    mbfMap.insert("returns", returns);
    mbfMap.insert("parent", parent->name);
    mbfMap.insert("MBF", bf);
    mbfMap.insert("isFormula",isF);
    return mbfMap;
}

bool MBF::buildNFA(NFA *nfa) {
    // O -> O under bf
    QString state1 = "q";
    state1.append(QString::number(nfa->i));
    (nfa->i)++;
    QString state2 = "q";
    state2.append(QString::number(nfa->i));
    (nfa->i)++;
    nfa->transitions.insert(state1 + '|' + bf,state2);
    if(nfa->start.isEmpty()) {
        nfa->start = state1;
    }
    else {
        nfa->transitions.insert(nfa->last + '|' + "epsilon",state1);
    }
    nfa->last = state2;
    nfa->accept = state2;
    return true;
}

bool MBF::removeSelfFromMap(QMap<QString, MSF*> &map) {
    int count = map.remove(name);
    if(count > 0) {
        return true;
    }
    else {
        return false;
    }
}

QStringList MBF::getMSFNames() {
    return QStringList(name);
}

MBF::~MBF() {
    if(isF) {
        for(int i=0; i<_atagger->tempMSFVector->count(); i++) {
            if(_atagger->tempMSFVector->at(i)->name == bf) {
                _atagger->tempMSFVector->at(i)->usedCount = _atagger->tempMSFVector->at(i)->usedCount -1;
                break;
            }
        }
    }
}

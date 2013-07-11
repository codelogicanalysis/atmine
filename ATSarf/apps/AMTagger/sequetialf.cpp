#include "sequetialf.h"

SequentialF::SequentialF(QString name, MSF* parent): MSF(name, parent)
{
}

void SequentialF::addMSF(MSF *msf) {
    vector.append(msf);
}

bool SequentialF::isMBF() {
    return false;
}

bool SequentialF::isUnary() {
    return false;
}

bool SequentialF::isBinary() {
    return false;
}

bool SequentialF::isFormula() {
    return false;
}

bool SequentialF::isSequential() {
    return true;
}

QString SequentialF::print() {
    QString value;
    for(int i=0; i<vector.count(); i++) {
        value.append(vector.at(i)->print());
        value.append(" ");
    }
    value.chop(1);
    return value;
}

QString SequentialF::printwithNames() {
    if(parent != NULL) {
        return QString();
    }

    QString value = name;
    value.append("=(");
    for(int i=0; i<vector.count(); i++) {
        value.append(vector.at(i)->printwithNames());
        value.append(" ");
    }
    value.chop(1);
    value.append(')');
    return value;
}

void SequentialF::buildTree(QTreeWidgetItem* parent) {
    QStringList data;
    data << name << QString() << "()";
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, data);
    for(int i=0; i<vector.count(); i++) {
        vector.at(i)->buildTree(item);
    }
}

void SequentialF::buildTree(QTreeWidget* parent) {

    QStringList data;
    data << name << QString() << "()";
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, data);
    for(int i=0; i<vector.count(); i++) {
        vector.at(i)->buildTree(item);
    }
}

QVariantMap SequentialF::getJSON() {
    QVariantMap sMap;
    sMap.insert("name", name);
    sMap.insert("type","sequential");
    sMap.insert("init", init);
    sMap.insert("actions",actions);
    sMap.insert("after", after);
    sMap.insert("returns", returns);
    sMap.insert("parent",parent->name);
    QVariantList sList;
    for(int i=0; i<vector.count();i++) {
        sList << vector.at(i)->getJSON();
    }
    sMap.insert("MSFs",sList);
    return sMap;
}

bool SequentialF::buildNFA(NFA *nfa) {
    for(int i=0; i< vector.count(); i++) {
        if(!(vector.at(i)->buildNFA(nfa))) {
            return false;
        }
    }
    nfa->accept = nfa->last;
    return true;
}

bool SequentialF::removeSelfFromMap(QMap<QString, MSF*> &map) {
    for(int j=0; j<vector.count(); j++) {
        if(!(vector.at(j)->removeSelfFromMap(map))) {
            return false;
        }
    }
    int count = map.remove(name);
    if(count > 0) {
        return true;
    }
    else {
        return false;
    }
}

QStringList SequentialF::getMSFNames() {
    QStringList list;
    list.append(name);
    for(int i=0; i<vector.count(); i++) {
        QStringList tempList = vector.at(i)->getMSFNames();
        list += tempList;
    }
    return list;
}

SequentialF::~SequentialF() {
    for(int i=0; i<vector.count(); i++) {
        delete vector.at(i);
    }
}

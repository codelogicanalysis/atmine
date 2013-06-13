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
    sMap.insert("parent",parent->name);
    QVariantList sList;
    for(int i=0; i<vector.count();i++) {
        sList << vector.at(i)->getJSON();
    }
    sMap.insert("MSFs",sList);
    return sMap;
}

SequentialF::~SequentialF() {
    for(int i=0; i<vector.count(); i++) {
        delete vector.at(i);
    }
}

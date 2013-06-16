#include "msformula.h"
#include "global.h"

MSFormula::MSFormula(QString name, MSF* parent): MSF(name, parent)
{
    i=0;
    usedCount = 0;
}

bool MSFormula::addMSF(QString parent, MSF * msf, int left) {
    if(parent == this->name) {
        map.insert(msf->name,msf);
        vector.append(msf);
        if(msf->isFormula()) {
            MBF* mbf = (MBF*)msf;
            for(int j=0; j<_atagger->msfVector->count(); j++) {
                if(_atagger->msfVector->at(j)->name == mbf->bf) {
                    _atagger->msfVector->at(j)->usedCount++;
                    break;
                }
            }
        }
        i++;
        return true;
    }

    if(map.find(parent) != map.end()) {
        map.insert(msf->name,msf);
        MSF* parentF = map.value(parent);
        if(parentF->isUnary()) {
            UNARYF* pf = (UNARYF*)parentF;
            pf->setMSF(msf);
            //delete pf;
        }
        else if(parentF->isBinary()) {
            BINARYF* pf = (BINARYF*)parentF;
            if(left == 0) {
                pf->setRightMSF(msf);
            }
            else {
                pf->setLeftMSF(msf);
            }
            //delete pf;
        }
        else if(parentF->isSequential()) {
            SequentialF* sf = (SequentialF*)parentF;
            sf->addMSF(msf);
        }
        else {
            return false;
        }
        i++;
        return true;
    }
    return false;
}

bool MSFormula::removeMSF(QString parent, QString msfName) {

    MSF* msf = map.value(msfName);
    delete msf;
    if(name == parent) {
        if(!(map.remove(msfName) == 1)) {
            return false;
        }
        for(int j=0; j< vector.count(); j++) {
            if(vector.at(j)->name == msfName) {
                vector.remove(j);
                break;
            }
        }
        return true;
    }

    if(map.find(parent) != map.end()) {
        SequentialF* sf = (SequentialF*)(map.value(parent));
        delete msf;
        if(!(map.remove(msfName) == 1)) {
            return false;
        }
        for(int j=0; j< sf->vector.count(); j++) {
            if(sf->vector.at(j)->name == msfName) {
                sf->vector.remove(j);
                break;
            }
        }
        return true;
    }
    return false;
}

bool MSFormula::updateMSF(QString parent, QString child, UNARYF *msf) {
    if(this->name == parent) {
        map.insert(msf->name, msf);
        MSF* childMSF;
        int j;
        for(j=0; j< vector.count(); j++) {
            if(vector.at(j)->name == child) {
                childMSF = vector.at(j);
                break;
            }
        }
        msf->msf = childMSF;
        vector.replace(j, msf);
        i++;
        return true;
    }

    MSF* childMSF = map.value(child);
    if(map.find(parent) != map.end()) {
        map.insert(msf->name, msf);
        MSF* parentF = map.value(parent);
        if(parentF->isBinary()) {
            /// Parent is binary formula

            BINARYF* parentMSF = (BINARYF*)parentF;
            msf->msf = childMSF;
            if(childMSF->name == parentMSF->leftMSF->name) {
                parentMSF->leftMSF = msf;
            }
            else {
                parentMSF->rightMSF = msf;
            }
        }
        else if(parentF->isSequential()) {
            /// Parent is sequential formula

            SequentialF* sf = (SequentialF*)parentF;
            int j;
            for(j=0; j<sf->vector.count(); j++) {
                if(sf->vector.at(j)->name == child) {
                    childMSF = vector.at(j);
                    break;
                }
            }
            msf->msf = childMSF;
            sf->vector.replace(j,msf);
        }
        else if(parentF->isFormula()) {
            /// Parent is a formula
        }
        else {
            return false;
        }
        i++;
        return true;
    }
    return false;
}

bool MSFormula::updateMSF(QString parent, QString fchild, QString schild, BINARYF* msf) {
    if(this->name == parent) {
        map.insert(msf->name, msf);
        MSF* childMSF1;
        MSF* childMSF2;
        int m,n;
        for(int j=0; j< vector.count(); j++) {
            if(vector.at(j)->name == fchild) {
                childMSF1 = vector.at(j);
                m = j;
            }
            else if(vector.at(j)->name == schild) {
                childMSF2 = vector.at(j);
                n = j;
            }
        }
        msf->setLeftMSF(childMSF1);
        msf->setRightMSF(childMSF2);
        vector.replace(m, msf);
        vector.remove(n);
        i++;
        return true;
    }

    MSF* childMSF1 = map.value(fchild);
    MSF* childMSF2 = map.value(schild);

    if(map.find(parent) != map.end()) {
        map.insert(msf->name, msf);
        MSF* parentF = map.value(parent);

        if(parentF->isSequential()) {
            /// Parent is sequential formula

            SequentialF* sf = (SequentialF*)parentF;
            int m,n;
            for(int j=0; j<sf->vector.count(); j++) {
                if(sf->vector.at(j)->name == fchild) {
                    m = j;
                }
                else if(sf->vector.at(j)->name == schild) {
                    n = j;
                }
            }
            msf->setLeftMSF(childMSF1);
            msf->setRightMSF(childMSF2);
            sf->vector.replace(m,msf);
            sf->vector.remove(n);
        }
        else {
            return false;
        }
        i++;
        return true;
    }
    return false;
}

bool MSFormula::updateMSF(QString parent, QVector<QString> *children, SequentialF *msf) {

    if(parent == this->name) {
        map.insert(msf->name, msf);
        MSF* fchild = map.value(children->at(0));

        for(int m=0; m<children->count(); m++) {
            for(int n=0; n<vector.count(); n++) {
                MSF* child = vector.at(n);
                if(children->at(m) == vector.at(n)->name) {
                    if(m == 0) {
                        msf->addMSF(fchild);
                        vector.replace(n,msf);
                    }
                    else {
                        msf->addMSF(child);
                        vector.remove(n);
                    }
                    break;
                }
            }
        }
        i++;
        return true;
    }

    if(map.find(parent) != map.end()) {
        map.insert(msf->name, msf);

        /// The parent should be a sequential formula
        if(map.value(parent)->isSequential()) {

            SequentialF* parentF = (SequentialF*)(map.value(parent));
            MSF* fchild = map.value(children->at(0));

            for(int m=0; m<children->count(); m++) {
                for(int n=0; n<parentF->vector.count(); n++) {
                    MSF* child = parentF->vector.at(n);
                    if(children->at(m) == parentF->vector.at(n)->name) {
                        if(m == 0) {
                            msf->addMSF(fchild);
                            parentF->vector.replace(n,msf);
                        }
                        else {
                            msf->addMSF(child);
                            parentF->vector.remove(n);
                        }
                        break;
                    }
                }
            }
            i++;
            return true;
        }
    }
    return false;
}

bool MSFormula::isMBF() {
    return false;
}

bool MSFormula::isUnary() {
    return false;
}

bool MSFormula::isBinary() {
    return false;
}

bool MSFormula::isFormula() {
    return true;
}

bool MSFormula::isSequential() {
    return false;
}

QString MSFormula::print() {
    if(parent != NULL) {
        return name;
    }

    QString value = "";
    if(vector.count() == 0) {
        return value;
    }
    for(int i=0; i<vector.count(); i++) {
        value.append(vector.at(i)->print());
        value.append(" ");
    }
    value.chop(1);
    return value;
}

void MSFormula::buildTree(QTreeWidget* parent) {
    for(int j=0; j<vector.count(); j++) {
        vector.at(j)->buildTree(parent);
    }
}

void MSFormula::buildTree(QTreeWidgetItem* parent) {
    for(int j=0; j<vector.count(); j++) {
        vector.at(j)->buildTree(parent);
    }
}

QVariantMap MSFormula::getJSON() {
    QVariantMap msfMap;
    msfMap.insert("name", name);
    msfMap.insert("type","msformula");
    msfMap.insert("i",i);
    QVariantList msfList;
    for(int j=0; j<vector.count(); j++) {
        msfList << vector.at(j)->getJSON();
    }
    msfMap.insert("MSFs",msfList);
    return msfMap;
}

bool MSFormula::buildNFA(NFA *nfa) {
    for(int j=0; j<vector.count(); j++) {
        if(!(vector.at(j)->buildNFA(nfa))) {
            return false;
        }
    }
    return true;
}

MSFormula::~MSFormula() {
    for(int j=0; j<vector.count(); j++) {
        delete vector.at(j);
    }
}

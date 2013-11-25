#include "msformula.h"
#include "global.h"

MSFormula::MSFormula(QString name, MSF* parent): MSF(name, parent)
{
    i=0;
    usedCount = 0;
}

bool MSFormula::addMSF(MSF * msf) {
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

bool MSFormula::removeMSF(QString parent, QString msfName) {

    MSF* msf = map.value(msfName);
    if(!(msf->removeSelfFromMap(map))) {
        return false;
    }
    map.insert(msfName, msf);
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
        delete msf;
        return true;
    }

    if(map.find(parent) != map.end()) {
        SequentialF* sf = (SequentialF*)(map.value(parent));
        if(!(map.remove(msfName) == 1)) {
            return false;
        }
        for(int j=0; j< sf->vector.count(); j++) {
            if(sf->vector.at(j)->name == msfName) {
                sf->vector.remove(j);
                break;
            }
        }
        delete msf;
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
        childMSF->parent = msf;
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
            childMSF->parent = msf;
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
                    break;
                }
            }
            msf->msf = childMSF;
            childMSF->parent = msf;
            sf->vector.replace(j,msf);
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
        childMSF1->parent = msf;
        msf->setRightMSF(childMSF2);
        childMSF2->parent = msf;
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
            childMSF1->parent = msf;
            msf->setRightMSF(childMSF2);
            childMSF2->parent = msf;
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
                        fchild->parent = msf;
                        vector.replace(n,msf);
                    }
                    else {
                        msf->addMSF(child);
                        child->parent = msf;
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
                            fchild->parent = msf;
                            parentF->vector.replace(n,msf);
                        }
                        else {
                            msf->addMSF(child);
                            child->parent = msf;
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
    for(int j=0; j<vector.count(); j++) {
        value.append(vector.at(j)->print());
        value.append(" ");
    }
    value.chop(1);
    return value;
}

QString MSFormula::printwithNames() {
    if(parent != NULL) {
        return QString();
    }

    if(vector.count() == 0) {
        return QString();
    }
    else if(vector.count() == 1) {
        return vector.at(0)->printwithNames();
    }

    QString value;
    for(int j=0; j<vector.count(); j++) {
        value.append(vector.at(j)->printwithNames());
        value.append(" ");
    }
    value.chop(1);
    value.append(')');
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

bool MSFormula::buildActionFile(QString &actionsData, QMultiMap<QString, QPair<QString,QString> > *functionParametersMap) {
    actionsData.append(includes + "\n\n");
    actionsData.append(members + "\n\n");

    for(int j=0; j<vector.count(); j++) {
        if(!(vector.at(j)->buildActionFile(actionsData, functionParametersMap))) {
            return false;
        }
    }
    return true;
}

QVariantMap MSFormula::getJSON() {
    QVariantMap msfMap;
    msfMap.insert("name", name);
    msfMap.insert("init", init);
    //msfMap.insert("after", after);
    msfMap.insert("actions", actions);
    msfMap.insert("returns", returns);
    msfMap.insert("description", description);
    msfMap.insert("fgcolor", fgcolor);
    msfMap.insert("bgcolor", bgcolor);
    msfMap.insert("type","msformula");
    msfMap.insert("i",i);
    msfMap.insert("usedCount", usedCount);
    msfMap.insert("includes",includes);
    msfMap.insert("members",members);
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
    QString state1 = "q";
    state1.append(QString::number(nfa->i));
    (nfa->i)++;
    if(nfa->start.isEmpty()) {
        nfa->start = state1;
    }
    else {
        nfa->transitions.insert(nfa->last + '|' + "epsilon",state1);
    }
    nfa->last = state1;
    nfa->accept = state1;
    return true;
}

bool MSFormula::removeSelfFromMap(QMap<QString, MSF*> &map) {
    for(int j=0; j<vector.count(); j++) {
        if(!(vector.at(j)->removeSelfFromMap(map))) {
            return false;
        }
    }
    return true;
}

QStringList MSFormula::getMSFNames() {
    QStringList list;
    for(int j=0; j<vector.count(); j++) {
        list += vector.at(j)->getMSFNames();
    }
    return list;
}

MSFormula::~MSFormula() {
    for(int j=0; j<vector.count(); j++) {
        delete vector.at(j);
    }
}

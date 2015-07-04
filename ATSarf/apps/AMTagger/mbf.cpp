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
    //QTreeWidgetItem* item = new QTreeWidgetItem(parent, data);
}

void MBF::buildTree(QTreeWidget* parent) {
    QStringList data;
    data << name << bf << QString();
    //QTreeWidgetItem* item = new QTreeWidgetItem(parent, data);
}

bool MBF::buildActionFile(QString &actionsData, QMultiMap<QString, QPair<QString,QString> > *functionParametersMap) {
    /// Adding function for preMatch actions
    QString tempInit = init;
    actionsData.append("extern \"C\" void " + name + "_preMatch(");
    if(!(tempInit.isEmpty())) {
        QSet<QString> param;
        while(true) {
            int dollarIndex = tempInit.indexOf("$", 0);
            if(dollarIndex == -1) {
                break;
            }

            QString msfName = tempInit.mid(dollarIndex+1).section('.',0,0);

            int afterDotPosition = tempInit.indexOf('.', dollarIndex) +1;
            QRegExp sep("[^a-zA-Z]");
            QString attribute = tempInit.mid(afterDotPosition).section(sep, 0, 0);
            tempInit = tempInit.remove(dollarIndex, 1);
            tempInit = tempInit.replace(afterDotPosition-2, 1, '_');

            if(param.contains(msfName + '|' + attribute)) {
                continue;
            }
            param.insert(msfName + '|' + attribute);
            if(attribute.compare("text") == 0) {
                functionParametersMap->insert(name + "_preMatch", QPair<QString,QString>(msfName,"text"));
                actionsData.append("QString " + msfName + "_text, ");
            }
            else if(attribute.compare("number") == 0) {
                functionParametersMap->insert(name + "_preMatch", QPair<QString,QString>(msfName,"number"));
                actionsData.append("int " + msfName + "_number, ");
            }
            else if(attribute.compare("position") == 0) {
                functionParametersMap->insert(name + "_preMatch", QPair<QString,QString>(msfName,"position"));
                actionsData.append("int " + msfName + "_position, ");
            }

            else if(attribute.compare("length") == 0) {
                functionParametersMap->insert(name + "_preMatch", QPair<QString,QString>(msfName,"length"));
                actionsData.append("int " + msfName + "_length, ");
            }
            else if(attribute.compare("matches") == 0) {
                functionParametersMap->insert(name + "_preMatch", QPair<QString,QString>(msfName,"matches"));
                actionsData.append("vector<Match>& " + msfName + "_matches, ");
            }
            else {
                return false;
            }
        }
        if(param.count() != 0) {
            actionsData.chop(2);
        }
    }
    actionsData.append(") {\n" + tempInit + "\n}\n\n");

    /// Adding function for onMatch actions
    QString tempMatch = actions;
    if(returns.isEmpty()) {
        actionsData.append("extern \"C\" void " + name + "_onMatch(");
    }
    else {
        actionsData.append("extern \"C\" " + returns + ' ' + name + "_onMatch(");
    }
    if(!(tempMatch.isEmpty())) {
        QSet<QString> param;
        while(true) {
            int dollarIndex = tempMatch.indexOf("$", 0);
            if(dollarIndex == -1) {
                break;
            }

            QString msfName = tempMatch.mid(dollarIndex+1).section('.',0,0);

            int afterDotPosition = tempMatch.indexOf('.', dollarIndex) +1;
            QRegExp sep("[^a-zA-Z]");
            QString attribute = tempMatch.mid(afterDotPosition).section(sep, 0, 0);
            tempMatch = tempMatch.remove(dollarIndex, 1);
            tempMatch = tempMatch.replace(afterDotPosition-2, 1, '_');

            if(param.contains(msfName + '|' + attribute)) {
                continue;
            }
            param.insert(msfName + '|' + attribute);

            if(attribute.compare("text") == 0) {
                functionParametersMap->insert(name + "_onMatch", QPair<QString,QString>(msfName,"text"));
                actionsData.append("QString " + msfName + "_text, ");
            }
            else if(attribute.compare("number") == 0) {
                functionParametersMap->insert(name + "_onMatch", QPair<QString,QString>(msfName,"number"));
                actionsData.append("int " + msfName + "_number, ");
            }
            else if(attribute.compare("position") == 0) {
                functionParametersMap->insert(name + "_onMatch", QPair<QString,QString>(msfName,"position"));
                actionsData.append("int " + msfName + "_position, ");
            }

            else if(attribute.compare("length") == 0) {
                functionParametersMap->insert(name + "_onMatch", QPair<QString,QString>(msfName,"length"));
                actionsData.append("int " + msfName + "_length, ");
            }
            else if(attribute.compare("matches") == 0) {
                functionParametersMap->insert(name + "_onMatch", QPair<QString,QString>(msfName,"matches"));
                actionsData.append("vector<Match>& " + msfName + "_matches, ");
            }
            else {
                return false;
            }
        }
        if(param.count() != 0) {
            actionsData.chop(2);
        }
    }
    actionsData.append(") {\n" + tempMatch + "\n}\n\n");
    return true;
}

QVariantMap MBF::getJSON() {
    QVariantMap mbfMap;
    mbfMap.insert("name", name);
    mbfMap.insert("type","mbf");
    mbfMap.insert("init", init);
    mbfMap.insert("actions",actions);
    //mbfMap.insert("after", after);
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
    nfa->stateTOmsfMap.insert(state1, QPair<MSF*,QString>(this,"pre"));
    nfa->stateTOmsfMap.insert(state2, QPair<MSF*,QString>(this,"on"));

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

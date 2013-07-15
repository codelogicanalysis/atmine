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

bool UNARYF::buildActionFile(QString &actionsData, QMultiMap<QString, QString> *functionParametersMap) {
    if(!(msf->buildActionFile(actionsData, functionParametersMap))) {
        return false;
    }

    /// Adding function for preMatch actions
    QString tempInit = init;
    actionsData.append("void " + name + "_preMatch(");
    if(!(tempInit.isEmpty())) {
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

            if(attribute.compare("text") == 0) {
                functionParametersMap->insert(name + "_preMatch", msfName + "|text");
                actionsData.append("QString " + msfName + "_text, ");
            }
            else if(attribute.compare("number") == 0) {
                functionParametersMap->insert(name + "_preMatch", msfName + "|number");
                actionsData.append("int " + msfName + "_number, ");
            }
            else if(attribute.compare("position") == 0) {
                functionParametersMap->insert(name + "_preMatch", msfName + "|position");
                actionsData.append("int " + msfName + "_position, ");
            }

            else if(attribute.compare("length") == 0) {
                functionParametersMap->insert(name + "_preMatch", msfName + "|length");
                actionsData.append("int " + msfName + "_length, ");
            }
            else {
                return false;
            }
        }
        actionsData.chop(2);
    }
    actionsData.append(") {\n" + tempInit + "\n}\n\n");

    /// Adding function for onMatch actions
    QString tempMatch = actions;
    actionsData.append("void " + name + "_onMatch(");
    if(!(tempMatch.isEmpty())) {
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
            if(attribute.compare("text") == 0) {
                functionParametersMap->insert(name + "_onMatch", msfName + "|text");
                actionsData.append("QString " + msfName + "_text, ");
            }
            else if(attribute.compare("number") == 0) {
                functionParametersMap->insert(name + "_onMatch", msfName + "|number");
                actionsData.append("int " + msfName + "_number, ");
            }
            else if(attribute.compare("position") == 0) {
                functionParametersMap->insert(name + "_onMatch", msfName + "|position");
                actionsData.append("int " + msfName + "_position, ");
            }

            else if(attribute.compare("length") == 0) {
                functionParametersMap->insert(name + "_onMatch", msfName + "|length");
                actionsData.append("int " + msfName + "_length, ");
            }
            else {
                return false;
            }
        }
        actionsData.chop(2);
    }
    actionsData.append(") {\n" + tempMatch + "\n}\n\n");

    QString tempAfter = after;
    actionsData.append("void " + name + "_postMatch(");
    if(!(tempAfter.isEmpty())) {
        while(true) {
            int dollarIndex = tempAfter.indexOf("$", 0);
            if(dollarIndex == -1) {
                break;
            }

            QString msfName = tempAfter.mid(dollarIndex+1).section('.',0,0);

            int afterDotPosition = tempAfter.indexOf('.', dollarIndex) +1;
            QRegExp sep("[^a-zA-Z]");
            QString attribute = tempAfter.mid(afterDotPosition).section(sep, 0, 0);
            tempAfter = tempAfter.remove(dollarIndex, 1);
            tempAfter = tempAfter.replace(afterDotPosition-2, 1, '_');
            if(attribute.compare("text") == 0) {
                functionParametersMap->insert(name + "_postMatch", msfName + "|text");
                actionsData.append("QString " + msfName + "_text, ");
            }
            else if(attribute.compare("number") == 0) {
                functionParametersMap->insert(name + "_postMatch", msfName + "|number");
                actionsData.append("int " + msfName + "_number, ");
            }
            else if(attribute.compare("position") == 0) {
                functionParametersMap->insert(name + "_postMatch", msfName + "|position");
                actionsData.append("int " + msfName + "_position, ");
            }

            else if(attribute.compare("length") == 0) {
                functionParametersMap->insert(name + "_postMatch", msfName + "|length");
                actionsData.append("int " + msfName + "_length, ");
            }
            else {
                return false;
            }
        }
        actionsData.chop(2);
    }
    actionsData.append(") {\n" + tempAfter + "\n}\n\n");

    return true;
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

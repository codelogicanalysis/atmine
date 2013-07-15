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

QString BINARYF::printwithNames() {
    QString value = name;
    value.append("=((");
    value.append(leftMSF->printwithNames());
    value.append(")");
    if(op == AND) {
        value.append("&");
    }
    else {
        value.append("|");
    }
    value.append("(");
    value.append(rightMSF->printwithNames());
    value.append("))");
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

bool BINARYF::buildActionFile(QString &actionsData, QMultiMap<QString, QString> *functionParametersMap) {
    if(!(leftMSF->buildActionFile(actionsData, functionParametersMap))) {
        return false;
    }

    if(!(rightMSF->buildActionFile(actionsData, functionParametersMap))) {
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

QVariantMap BINARYF::getJSON() {
    QVariantMap bMap;
    bMap.insert("name",name);
    bMap.insert("type","binary");
    bMap.insert("init", init);
    bMap.insert("actions",actions);
    bMap.insert("after", after);
    bMap.insert("returns", returns);
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

    //if(nfa->start.isEmpty()) {
    QString state1 = "q";
    state1.append(QString::number(nfa->i));
    if(nfa->start.isEmpty()) {
        nfa->start = state1;
    }
    else {
        nfa->transitions.insert(nfa->last + "|epsilon", state1);
    }
    nfa->last = state1;
    (nfa->i)++;
    //}

    QString currentStart = nfa->last;

    if(op == OR) {

        if(!(leftMSF->buildNFA(nfa))) {
            return false;
        }
        QString leftlast = nfa->last;
        nfa->last = currentStart;

        if(!(rightMSF->buildNFA(nfa))) {
            return false;
        }

        QString currentLast = "q";
        currentLast.append(QString::number(nfa->i));
        (nfa->i)++;

        nfa->transitions.insert(leftlast + '|' + "epsilon", currentLast);
        nfa->transitions.insert(nfa->last + '|' + "epsilon", currentLast);

        nfa->last = currentLast;
        nfa->accept = currentLast;
        return true;
    }
    else if(op == AND) {

        if(!(leftMSF->buildNFA(nfa))) {
            return false;
        }
        QString leftlast = nfa->last;
        nfa->last = currentStart;

        if(!(rightMSF->buildNFA(nfa))) {
            return false;
        }

        QString currentLast = "q";
        currentLast.append(QString::number(nfa->i));
        (nfa->i)++;

        QString andAccept = "q";
        andAccept.append(QString::number(nfa->i));
        (nfa->i)++;
        nfa->andAccept.append(andAccept);

        nfa->transitions.insert(leftlast + '|' + "epsilon", andAccept);
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

QStringList BINARYF::getMSFNames() {
    QStringList list(name);
    list += leftMSF->getMSFNames();
    list += rightMSF->getMSFNames();
    return list;
}

BINARYF::~BINARYF() {
    delete leftMSF;
    delete rightMSF;
}

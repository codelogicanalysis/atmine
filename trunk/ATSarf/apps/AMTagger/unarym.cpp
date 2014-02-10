#include "unarym.h"

UnaryM::UnaryM(Operation op, Match *parent, int limit): Match(op,parent)
{
    this->limit = limit;
}

bool UnaryM::setMatch(Match *match) {
    matches.prepend(match);
    return true;
}

bool UnaryM::isUnaryM() {
    return true;
}

bool UnaryM::isBinaryM() {
    return false;
}

bool UnaryM::isSequentialM() {
    return false;
}

bool UnaryM::isKeyM() {
    return false;
}

bool UnaryM::isMERFTag() {
    return false;
}

int UnaryM::getPOS() {
    if(matches.isEmpty()) {
        return -1;
    }
    return matches.at(0)->getPOS();
}

int UnaryM::getLength() {
    if(matches.isEmpty()) {
        return 0;
    }
    int start = matches.at(0)->getPOS();
    int end = matches.last()->getPOS() + matches.last()->getLength();
    return end - start;
}

QString UnaryM::getText() {
    QString text;
    for(int i=0; i<matches.count(); i++) {
        text.append(matches.at(i)->getText());
        text.append(' ');
    }
    text.chop(1);
    return text;
}

int UnaryM::getMatchCount() {
    if(matches.isEmpty()) {
        return 0;
    }
    int count = 0;
    for(int i=0; i<matches.count(); i++) {
        count += matches.at(i)->getMatchCount();
    }
    return count;
}

void UnaryM::buildMatchTree(Agraph_t* G,Agnode_t* node,Agedge_t* edge,QMap<Agnode_t *,Agnode_t *>* parentNodeMap,QTreeWidgetItem* parentItem, int& id) {
    QStringList data;
    /// Used for the case of UPTO only
    QString limitS;
    if(op == STAR) {
        data << "Operation" << "*";
    }
    else if(op == PLUS) {
        data << "Operation" << "+";
    }
    else if(op == UPTO) {
        limitS = QString::number(limit);
        limitS.prepend('^');
        data << "Operation" << limitS;
    }
    else {
        data << "Operation" << "?";
    }
    QTreeWidgetItem* newItem = new QTreeWidgetItem(parentItem,data);
    parentItem = newItem;

    if(node == NULL) {
        stringstream strs;
        strs << id;
        string temp_str = strs.str();
        char* nodeID = strdup(temp_str.c_str());
        Agnode_t * newNode = agnode(G,nodeID, 1);
        agset(G,const_cast<char *>("root"),nodeID);
        id = id+1;
        if(op == STAR) {
            agset(newNode,const_cast<char *>("label"),const_cast<char *>("*"));
        }
        else if(op == PLUS) {
            agset(newNode,const_cast<char *>("label"),const_cast<char *>("+"));
        }
        else if(op == UPTO) {
            char * writable = strdup(limitS.toStdString().c_str());
            agset(newNode,const_cast<char *>("label"),writable);
            free(writable);
        }
        else {
            agset(newNode,const_cast<char *>("label"),const_cast<char *>("?"));
        }
        parentNodeMap->insert(newNode,NULL);
        node = newNode;
    }
    else {
        stringstream strs;
        strs << id;
        string temp_str = strs.str();
        char* nodeID = strdup(temp_str.c_str());
        Agnode_t * newNode = agnode(G,nodeID, 1);
        id = id+1;
        if(op == STAR) {
            agset(newNode,const_cast<char *>("label"),const_cast<char *>("*"));
        }
        else if(op == PLUS) {
            agset(newNode,const_cast<char *>("label"),const_cast<char *>("+"));
        }
        else if(op == UPTO) {
            char * writable = strdup(limitS.toStdString().c_str());
            agset(newNode,const_cast<char *>("label"),writable);
            free(writable);
        }
        else {
            agset(newNode,const_cast<char *>("label"),const_cast<char *>("?"));
        }
        edge = agedge(G, node, newNode, 0, 1);
        parentNodeMap->insert(newNode, node);
        node = newNode;
    }

    for(int i=0; i<matches.count(); i++) {
        matches.at(i)->buildMatchTree(G,node,edge,parentNodeMap,parentItem,id);
    }
}

void UnaryM::executeActions(NFA* nfa) {
    MSFormula* formula = (MSFormula*)(nfa->formula);

    /** pre match **/
    QString preMatch = msf->name;
    preMatch.append("_preMatch();\n");
    formula->actionData.append(preMatch);
    /** Done **/

    for(int i=0;i<matches.count();i++) {
        matches.at(i)->executeActions(nfa);
    }

    /** on match **/
    QString onMatch = msf->name;
    onMatch.append("_onMatch");
    QMultiMap<QString,QPair<QString,QString> > *functionParametersMap = &(formula->functionParametersMap);
    QList<QPair<QString,QString> > params = functionParametersMap->values(onMatch);
    onMatch.append('(');

    for(int j=0; j<params.count(); j++) {
        QString msfName = params.at(j).first;
        QString field = params.at(j).second;

        QString sarfMatches;
        QString paramValue = getParam(msfName,field,&sarfMatches);
        if(field.compare("text") == 0) {
            onMatch.append('\"' + paramValue + "\",");
        }
        else if(field.compare("position") == 0) {
            onMatch.append(paramValue + ',');
        }
        else if(field.compare("length") == 0) {
            onMatch.append(paramValue + ',');
        }
        else if(field.compare("number") == 0) {
            onMatch.append(paramValue + ',');
        }
        else if(field.compare("matches") == 0) {
            onMatch.append(paramValue + ',');
            formula->actionData.append(sarfMatches);
        }
    }
    if(params.count() == 0) {
        onMatch.append(");\n");
    }
    else {
        onMatch.chop(1);
        onMatch.append(");\n");
    }
    formula->actionData.append(onMatch);
    /** Done **/
}

QString UnaryM::getParam(QString msfName,QString param, QString* sarfMatches) {
    if(msf->name == msfName) {
        if(param  == "text") {
            return getText();
        }
        else if(param == "position") {
            return QString::number(getPOS());
        }
        else if(param == "number") {
            QString text = getText();
            NumNorm nn(&text);
            nn();
            int number = NULL;
            if(nn.extractedNumbers.count()!=0) {
                number = nn.extractedNumbers[0].getNumber();
                return QString::number(number);
            }
            else {
                return "NULL";
            }
        }
        else if(param == "length") {
            return QString::number(getLength());
        }
        else {
            return "NULL";
        }
    }
    else {
        for(int i=matches.count()-1; i>=0; i--) {
            QString paramValue = matches.at(i)->getParam(msfName,param);
            if(!(paramValue.isEmpty())) {
                return paramValue;
            }
        }
        return "";
    }
    return "";
}

QVariantMap UnaryM::getJSON() {
    QVariantMap unaryMap;
    unaryMap.insert("type","unary");
    unaryMap.insert("msf",msf->name);
    unaryMap.insert("op",op);
    unaryMap.insert("limit",limit);
    QVariantList matchList;
    for(int i=matches.count()-1; i>=0; i--) {
        matchList << matches[i]->getJSON();
    }
    unaryMap.insert("matches",matchList);
    return unaryMap;
}

void UnaryM::constructRelation(Relation* relation, QVector<Match*>& entity1, QVector<Match*>& entity2, QVector<Match*>& edge) {

    if(relation->entity1->name == msf->name) {
        entity1.append(this);
    }
    if(relation->entity2->name == msf->name) {
        entity2.append(this);
    }
    if(relation->edge != NULL && relation->edge->name == msf->name) {
        edge.append(this);
    }

    for(int i=0; i<matches.count(); i++) {
        matches.at(i)->constructRelation(relation,entity1,entity2,edge);
    }
}

UnaryM::~UnaryM() {
    for(int i=0; i<matches.count(); i++) {
        delete (matches.at(i));
    }
    matches.clear();
}

#include "sequentialm.h"

SequentialM::SequentialM(Match* parent, int id): Match(SEQUENCE,parent,id)
{
}

bool SequentialM::setMatch(Match *match) {
    matches.prepend(match);
    return true;
}

bool SequentialM::isUnaryM() {
    return false;
}

bool SequentialM::isBinaryM() {
    return false;
}

bool SequentialM::isSequentialM() {
    return true;
}

bool SequentialM::isKeyM() {
    return false;
}

bool SequentialM::isMERFTag() {
    return false;
}

int SequentialM::getPOS() {
    if(matches.isEmpty()) {
        return -1;
    }
    return matches.at(0)->getPOS();
}

int SequentialM::getLength() {
    if(matches.isEmpty()) {
        return 0;
    }
    int start = matches.at(0)->getPOS();
    int end = matches.last()->getPOS() + matches.last()->getLength();
    return end - start;
}

QString SequentialM::getText() {
    QString text;
    for(int i=0; i<matches.count(); i++) {
        text.append(matches.at(i)->getText());
        text.append(' ');
    }
    text.chop(1);
    return text;
}

int SequentialM::getMatchCount() {
    if(matches.isEmpty()) {
        return 0;
    }
    int count = 0;
    for(int i=0; i<matches.count(); i++) {
        count += matches.at(i)->getMatchCount();
    }
    return count;
}

void SequentialM::buildMatchTree(Agraph_t* G,Agnode_t* node,Agedge_t* edge,QMap<Agnode_t *,Agnode_t *>* parentNodeMap,QTreeWidgetItem* parentItem, int& id) {
    if(msf == NULL && matches.count() == 1) {
        matches.at(0)->buildMatchTree(G,node,edge,parentNodeMap,parentItem,id);
        return;
    }

    QStringList data;
    data << "Operation" << "()";

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
        agset(newNode,const_cast<char *>("label"),const_cast<char *>("()"));
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
        agset(newNode,const_cast<char *>("label"),const_cast<char *>("()"));
        edge = agedge(G, node, newNode, 0, 1);
        parentNodeMap->insert(newNode, node);
        node = newNode;
    }

    for(int i=0; i<matches.count(); i++) {
        matches.at(i)->buildMatchTree(G,node,edge,parentNodeMap,parentItem,id);
    }
}

void SequentialM::executeActions(NFA* nfa) {
    if(msf == NULL) {
        for(int i=0;i<matches.count();i++) {
            matches.at(i)->executeActions(nfa);
        }
        return;
    }

    MSFormula* formula = static_cast<MSFormula *>(nfa->formula);

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

QString SequentialM::getParam(QString msfName, QString param, QString* /*sarfMatches*/) {
    if(msf == NULL) {
        for(int i=0; i<matches.count(); i++) {
            QString paramValue = matches.at(i)->getParam(msfName,param);
            if(!(paramValue.isEmpty())) {
                return paramValue;
            }
        }
        return "";
    }

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
            if(nn.extractedNumbers.count()!=0) {
                int number = nn.extractedNumbers[0].getNumber();
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
        for(int i=0; i<matches.count(); i++) {
            QString paramValue = matches.at(i)->getParam(msfName,param);
            if(!(paramValue.isEmpty())) {
                return paramValue;
            }
        }
        return "";
    }
    return "";
}

QVariantMap SequentialM::getJSON() {
    QVariantMap seqMap;
    seqMap.insert("type","sequential");
    seqMap.insert("id",id);
    if(msf == NULL) {
        seqMap.insert("mreid","_NULL_");
    }
    else {
        seqMap.insert("mreid",msf->name);
    }
    QVariantList matchList;
    for(int i=matches.count()-1; i>=0; i--) {
        matchList << matches[i]->getJSON();
    }
    seqMap.insert("matches",matchList);
    return seqMap;
}

void SequentialM::constructRelation(Relation* relation, QVector<Match*>& entity1, QVector<Match*>& entity2, QVector<Match*>& edge) {

    if(msf != NULL) {
        if(relation->entity1->name == msf->name) {
            entity1.append(this);
        }
        if(relation->entity2->name == msf->name) {
            entity2.append(this);
        }
        if(relation->edge != NULL && relation->edge->name == msf->name) {
            edge.append(this);
        }
    }

    for(int i=0; i<matches.count(); i++) {
        matches.at(i)->constructRelation(relation,entity1,entity2,edge);
    }
}

SequentialM::~SequentialM() {
    for(int i=0; i<matches.count(); i++) {
        delete (matches.at(i));
    }
    matches.clear();
}

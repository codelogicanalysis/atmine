#include "binarym.h"

BinaryM::BinaryM(Operation op, Match *parent, int id): Match(op,parent,id)
{
    leftMatch = NULL;
    rightMatch = NULL;
}

bool BinaryM::setMatch(Match *match) {
    if(leftMatch == NULL) {
        leftMatch = match;
    }
    else {
        rightMatch = match;
    }
    return true;
}

bool BinaryM::isUnaryM() {
    return false;
}

bool BinaryM::isBinaryM() {
    return true;
}

bool BinaryM::isSequentialM() {
    return false;
}

bool BinaryM::isKeyM() {
    return false;
}

bool BinaryM::isMERFTag() {
    return false;
}

int BinaryM::getPOS() {
    if(leftMatch == NULL) {
        return -1;
    }
    return leftMatch->getPOS();
}

int BinaryM::getLength() {
    if(leftMatch == NULL) {
        return 0;
    }
    return leftMatch->getLength();
}

QString BinaryM::getText() {
    return leftMatch->getText();
}

int BinaryM::getMatchCount() {
    if(leftMatch == NULL) {
        return 0;
    }
    return leftMatch->getMatchCount();
}

void BinaryM::buildMatchTree(Agraph_t* G,Agnode_t* node,Agedge_t* edge,QMap<Agnode_t *,Agnode_t *>* parentNodeMap,QTreeWidgetItem* parentItem, int& id) {
    //Agnode_t* oldNode = node;
    //QTreeWidgetItem* oldParentItem = parentItem;

    QStringList data;
    if(op == OR) {
        data << "Operation" << "|";
    }
    else {
        data << "Operation" << "&";
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
        if(op == OR) {
            agset(newNode,const_cast<char *>("label"),const_cast<char *>("|"));
        }
        else {
            agset(newNode,const_cast<char *>("label"),const_cast<char *>("&"));
        }
        parentNodeMap->insert(newNode,NULL);
        node = newNode;
        //oldNode = newNode;
    }
    else {
        stringstream strs;
        strs << id;
        string temp_str = strs.str();
        char* nodeID = strdup(temp_str.c_str());
        Agnode_t * newNode = agnode(G,nodeID, 1);
        id = id+1;
        if(op == OR) {
            agset(newNode,const_cast<char *>("label"),const_cast<char *>("|"));
        }
        else {
            agset(newNode,const_cast<char *>("label"),const_cast<char *>("&"));
        }
        edge = agedge(G, node, newNode, 0, 1);
        parentNodeMap->insert(newNode, node);
        node = newNode;
    }

    leftMatch->buildMatchTree(G,node,edge,parentNodeMap,parentItem,id);
    if(rightMatch != NULL) {
        rightMatch->buildMatchTree(G,node,edge,parentNodeMap,parentItem,id);
    }
}

void BinaryM::executeActions(NFA* nfa) {
    MSFormula* formula = (MSFormula*)(nfa->formula);

    /** pre match **/
    QString preMatch = msf->name;
    preMatch.append("_preMatch();\n");
    formula->actionData.append(preMatch);
    /** Done **/

    leftMatch->executeActions(nfa);
    if(rightMatch != NULL && op == AND) {
        rightMatch->executeActions(nfa);
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

QString BinaryM::getParam(QString msfName,QString param, QString* sarfMatches) {
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
        if(op == OR) {
            return leftMatch->getParam(msfName,param);
        }
        else {
            QString lParamValue = leftMatch->getParam(msfName,param);
            if(!(lParamValue.isEmpty())) {
                return lParamValue;
            }
            return rightMatch->getParam(msfName,param);
        }
    }
    return "";
}

QVariantMap BinaryM::getJSON() {
    QVariantMap binaryMap;
    binaryMap.insert("id",id);
    binaryMap.insert("type","binary");
    binaryMap.insert("msf",msf->name);
    binaryMap.insert("op",op);
    binaryMap.insert("leftMatch",leftMatch->getJSON());
    if(rightMatch != NULL) {
        binaryMap.insert("rightMatch",rightMatch->getJSON());
    }
    return binaryMap;
}

void BinaryM::constructRelation(Relation* relation, QVector<Match*>& entity1, QVector<Match*>& entity2, QVector<Match*>& edge) {

    if(relation->entity1->name == msf->name) {
        entity1.append(this);
    }
    if(relation->entity2->name == msf->name) {
        entity2.append(this);
    }
    if(relation->edge != NULL && relation->edge->name == msf->name) {
        edge.append(this);
    }

    leftMatch->constructRelation(relation,entity1,entity2,edge);
    if(rightMatch != NULL) {
        rightMatch->constructRelation(relation,entity1,entity2,edge);
    }
}

BinaryM::~BinaryM() {
    delete leftMatch;
    delete rightMatch;
    leftMatch = NULL;
    rightMatch = NULL;
}

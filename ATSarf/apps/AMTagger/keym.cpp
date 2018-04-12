#include "keym.h"
#include "amfiller.h"

KeyM::KeyM(Match *parent, QString key, int pos, int length, int id): Match(KEY,parent,id)
{
    this->parent = parent;
    this->key = key;
    this->pos = pos;
    this->length = length;
}

bool KeyM::setMatch(Match * /*match*/) {
    return true;
}

bool KeyM::isUnaryM() {
    return false;
}

bool KeyM::isBinaryM() {
    return false;
}

bool KeyM::isSequentialM() {
    return false;
}

bool KeyM::isKeyM() {
    return true;
}

bool KeyM::isMERFTag() {
    return false;
}

int KeyM::getPOS() {
    return pos;
}

int KeyM::getLength() {
    return length;
}

QString KeyM::getText() {
    return word;
}

int KeyM::getMatchCount() {
    return 1;
}

void KeyM::buildMatchTree(Agraph_t* G,Agnode_t* node, Agedge_t* /*edge*/, QMap<Agnode_t *,Agnode_t *>* parentNodeMap, QTreeWidgetItem* /*parentItem*/, int& id) {
    QStringList data;
    data << key << word;
    //QTreeWidgetItem* newItem = new QTreeWidgetItem(parentItem,data);

    QString text = key + '\n' + word;
    char * writable = strdup(text.toStdString().c_str());
    if(node == NULL) {
        stringstream strs;
        strs << id;
        string temp_str = strs.str();
        char* nodeID = strdup(temp_str.c_str());
        Agnode_t * newNode = agnode(G,nodeID, 1);
        agset(G,const_cast<char *>("root"),nodeID);
        id = id+1;
        agset(newNode,const_cast<char *>("label"),writable);
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
        agset(newNode,const_cast<char *>("label"),writable);
        //edge = agedge(G, node, newNode, 0, 1);
        agedge(G, node, newNode, 0, 1);
        parentNodeMap->insert(newNode, node);

    }
    free(writable);
}

void KeyM::executeActions(NFA* nfa) {
    MSFormula* formula = static_cast<MSFormula *>(nfa->formula);

    /** pre match **/
    QString preMatch = msf->name;
    preMatch.append("_preMatch();\n");
    formula->actionData.append(preMatch);
    /** Done **/

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

QString KeyM::getParam(QString msfName,QString param, QString* sarfMatches) {
    if(msf->name == msfName) {
        if(param  == "text") {
            return word;
        }
        else if(param == "position") {
            return QString::number(pos);
        }
        else if(param == "number") {
            NumNorm nn(&word);
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
            return QString::number(length);
        }
        else if(param == "matches") {
            QString mvName = msfName + '_' + param;
            AMFiller filler(word,sarfMatches,mvName);
            filler();
            QString rValue;
            rValue.append('&' + mvName);
            return rValue;
        }
    }
    return "";
}

QVariantMap KeyM::getJSON() {
    QVariantMap keyMap;
    keyMap.insert("type","key");
    keyMap.insert("key",key);
    keyMap.insert("pos",pos);
    keyMap.insert("length",length);
    keyMap.insert("text",word);
    keyMap.insert("mreid",msf->name);
    keyMap.insert("id",id);
    return keyMap;
}

void KeyM::constructRelation(Relation* relation, QVector<Match*>& entity1, QVector<Match*>& entity2, QVector<Match*>& edge) {

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

KeyM::~KeyM() {
}

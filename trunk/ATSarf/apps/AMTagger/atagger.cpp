#include "atagger.h"
#include <QVariant>
#include <qjson/serializer.h>
#include <QFile>
#include <QTextStream>
#include <QFile>
#include <dlfcn.h>
#include <stdlib.h>

ATagger * _atagger = NULL;

ATagger::ATagger() {
    tagTypeVector = new QVector<TagType*>();
    compareToTagTypeVector = new QVector<TagType*>();
    msfVector = new QVector<MSFormula*>();
    nfaVector = new QVector<NFA*>();
}

ATagger::~ATagger() {
    tagVector.clear();
    for(int i=0; i<tagTypeVector->size(); i++) {
        delete (*tagTypeVector)[i];
    }
    delete tagTypeVector;
    for(int i=0; i<compareToTagTypeVector->size(); i++) {
        delete (*compareToTagTypeVector)[i];
    }
    delete compareToTagTypeVector;
    for(int i=0; i<msfVector->count(); i++) {
        delete (*msfVector)[i];
    }
    delete msfVector;
    for(int i=0; i<nfaVector->count(); i++) {
        delete (*nfaVector)[i];
    }
    delete nfaVector;
}

bool ATagger::insertTag(QString type, int pos, int length, Source source, Dest dest) {

    Tag tag(type,pos,length,source);
    if(dest == original) {
        if(!(tagVector.contains(tag))) {
            tagVector.append(tag);
        }
        else {
            return false;
        }
    }
    else {
        if(!(compareToTagVector.contains(tag))) {
            compareToTagVector.append(tag);
        }
        else {
            return false;
        }
    }
    return true;
}

bool ATagger::insertTagType(QString tag, QString desc, int id, QString fgcolor, QString bgcolor, int font, bool underline, bool bold, bool italic, Source source, Dest dest) {

    TagType* tt = new TagType(tag,desc,id,fgcolor,bgcolor,font,underline,bold,italic,source);
    if(dest == original) {
        tagTypeVector->append(tt);
    }
    else {
        compareToTagTypeVector->append(tt);
    }
    return true;
}

bool ATagger::insertSarfTagType(QString tag, QVector<Quadruple< QString , QString , QString , QString > > tags, QString desc, int id, QString fgcolor, QString bgcolor, int font, bool underline, bool bold, bool italic, Source source, Dest dest) {
    SarfTagType* stt = new SarfTagType(tag,tags,desc,id,fgcolor,bgcolor,font,underline,bold,italic,source);
    if(dest == original) {
        tagTypeVector->append(stt);
    }
    else {
        compareToTagTypeVector->append(stt);
    }
    return true;
}

bool ATagger::buildNFA() {
    for(int i=0; i<msfVector->count(); i++) {
        NFA* nfa = new NFA(msfVector->at(i)->name);
        nfaVector->append(nfa);
        if(!(msfVector->at(i)->buildNFA(nfa))) {
            return false;
        }
    }
    return true;
}

bool ATagger::buildActionFile() {
    for(int i=0; i<msfVector->count(); i++) {
        MSFormula *formula = msfVector->at(i);
        if(!(formula->buildActionFile(formula->actionData, &(formula->functionParametersMap)))) {
            return false;
        }
    }

    /*
    for(int i=0; i<msfVector->count(); i++) {
        MSFormula *formula = msfVector->at(i);
        QFile file(formula->name + ".txt");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;

        QTextStream out(&file);
        out << formula->actionData;
        file.close();
    }
    */

    return true;
}

bool ATagger::executeActions() {

    /** Add function calls of each formula to a function named msfName() **/
    for(int i=0; i< nfaVector->count(); i++) {

        NFA* nfa = nfaVector->at(i);
        for(int j=0; j< msfVector->count(); j++) {

            MSFormula* formula = msfVector->at(i);
            if(nfa->name == formula->name) {

                formula->actionData.append("extern \"C\" void " + formula->name + "_actions() {\n");
                while(!(nfa->actionStack->isEmpty())) {
                    formula->actionData.append(nfa->actionStack->pop() + ";\n");
                }
                formula->actionData.append("}\n");
            }
        }

    }

    /** write action functions in .cpp output files, compile shared library, and call them **/
    for(int i=0; i< msfVector->count(); i++) {
        MSFormula* formula = msfVector->at(i);

        QString msfName = formula->name;
        QString cppFile = msfName + ".cpp";
        QFile f(cppFile);
        f.open(QIODevice::ReadWrite);
        QTextStream out(&f);
        out << formula->actionData;

        f.close();

        // create library
        QString command = "/usr/bin/g++ -fPIC -shared " + msfName + ".cpp -o lib" + msfName + ".so";
        system (command.toStdString().c_str());

        // load library
        QString sharedLib = "./lib" + msfName + ".so";
        void * fLib = dlopen ( sharedLib.toStdString().c_str(), RTLD_LAZY );
        if ( !fLib ) {
            cerr << "Cannot open library: " << dlerror() << '\n';
            return false;
        }

        if ( fLib ) {
            QString actionFunction = msfName + "_actions";
            void (*fn)() = (void (*)()) dlsym ( fLib, actionFunction.toStdString().c_str());
            if (fn) {
                // use function
                fn();
            }
            dlclose(fLib);
        }
    }

    return true;
}

bool ATagger::runSimulator() {

    /// Build NFA
    if(!buildNFA()) {
        return false;
    }

    if(!buildActionFile()) {
        return false;
    }

    /// Simulate NFAs referring to all the MSFs built
    simulationVector.clear();
    for(int i=0; i<nfaVector->count(); i++) {
        int index = 0;
        /// Simulate current MSF starting from a set of tokens referring to a single word
        for(int j=index; j<tagVector.count(); j++) {
            QStack<QString> *tempStack;
            QVector<Tag*>* tags = simulateNFA(nfaVector->at(i), tempStack, nfaVector->at(i)->start, index);
            if(tags != NULL) {
                int pos = tags->first()->pos;
                int length = tags->last()->pos - tags->first()->pos + tags->last()->length;
                MERFTag tag(nfaVector->at(i)->name, pos, length);
                tag.tags = tags;
                simulationVector.append(tag);

                /// Skip the tokens of the words in the match
                for(; j<tagVector.count(); j++) {
                    if((tagVector.at(j).pos) > (tags->last()->pos)) {
                        index = j;
                        break;
                    }
                }
                /// Join stacks
                if(nfaVector->at(i)->actionStack == NULL) {
                    nfaVector->at(i)->actionStack = tempStack;
                }
                else {
                    for(int k=0; k< nfaVector->at(i)->actionStack->count(); k++) {
                        tempStack->push(nfaVector->at(i)->actionStack->at(k));
                        //nfaVector->at(i)->actionStack->pop_back();
                    }
                    delete (nfaVector->at(i)->actionStack);
                    nfaVector->at(i)->actionStack = tempStack;
                }
            }
            else {
                while((j<tagVector.count()) && (tagVector.at(index).pos == tagVector.at(j).pos)) {
                    j++;
                }
                index = j;
            }
        }
    }

    executeActions();
    return true;
}

bool lessThan(const QString &s1, const QString &s2)
{
    QString actionType1 = s1.section('|',1,1);
    QString actionType2 = s2.section('|',1,1);
    if((actionType1 == actionType2) && (actionType1 == "pre")) {
        return s1 < s2;
    }
    else if((actionType1 == actionType2) && (actionType1 == "on")) {
        return s1 > s2;
    }
    else if(actionType1 == "on"){
        return true;
    }
    else {
        return false;
    }
}

bool ATagger::refineFunctions(NFA* nfa, QList<QString> &function, int index) {
    for(int i=0; i<function.count(); i++) {
        function[i] = function[i].replace('|','_');
        function[i] = function[i].append("Match(");
        if(function[i].contains("pre")) {
            function[i] = function[i].append(')');
            continue;
        }
        if(index != -1) {
            QMultiMap<QString,QString> *functionParametersMap;
            for(int j=0; j< msfVector->count(); j++) {
                if(msfVector->at(j)->name == nfa->name) {
                    functionParametersMap = &(msfVector->at(j)->functionParametersMap);
                    break;
                }
            }
            QString key = function[i];
            key.chop(1);
            QList<QString> params = functionParametersMap->values(key);
            for(int j=0; j<params.count(); j++) {
                QString msfName = params.at(j).section('|',0,0);
                QString field = params.at(j).section('|',1,1);
                QString msfName2 = function[i].section('_',0,0);
                if(!(msfName.compare(msfName2) == 0)) {
                    return false;
                }

                if(field.compare("text") == 0) {
                    QString paramText = text.mid(tagVector.at(index).pos, tagVector.at(index).length);
                    function[i].append('\"' + paramText + "\",");
                }
                else if(field.compare("position") == 0) {
                    function[i].append(tagVector.at(index).pos + ',');
                }
                else if(field.compare("length") == 0) {
                    function[i].append(tagVector.at(index).length + ',');
                }
                else if(field.compare("number") == 0) {
                    QString possibleNum = text.mid(tagVector.at(index).pos, tagVector.at(index).length);
                    NumNorm nn(&possibleNum);
                    nn();
                    int number = NULL;
                    if(nn.extractedNumbers.count()!=0) {
                        number = nn.extractedNumbers[0].getNumber();
                        function[i].append(QString::number(number) + ',');
                    }
                    else {
                        function[i].append("-1,");
                    }
                }
                else {
                    return false;
                }
            }
            if(params.count() == 0) {
                function[i].append(')');
            }
            else {
                function[i].chop(1);
                function[i].append(')');
            }
        }
        else {
            function[i].append(')');
        }
    }
    return true;
}

QVector<Tag*>* ATagger::simulateNFA(NFA* nfa, QStack<QString> *&actionStack, QString state, int tagIndex) {

    if((state == nfa->accept) || (nfa->andAccept.contains(state))) {
        /// check if the accept state stands for any function calls and push them to stack
        QList<QString> functionCalls = nfa->stateTOmsfMap.values(state);
        if(!(functionCalls.isEmpty())) {
            qSort(functionCalls.begin(), functionCalls.end(), lessThan);
        }
        refineFunctions(nfa,functionCalls);
        actionStack = new QStack<QString>();
        for(int i=0; i< functionCalls.count(); i++) {
            actionStack->push(functionCalls.at(i));
        }
        /// initialize a tag vector and return it
        QVector<Tag*> *tags = new QVector<Tag*>();
        return tags;
    }
    /** checked when transition found from MBF before MSF**/
    bool done = false;

    /// Use a vector of vectors to collect correct solutions and choose longest
    QVector<QVector<Tag*>*> tags;
    /// Use a vector of stacks to collect functions calls
    QVector<QStack<QString> *> stacks;

    QVector<int> tokens;
    for(int i=tagIndex; (i<tagVector.count()) && (tagVector.at(i).pos == tagVector.at(tagIndex).pos); i++) {
        tokens.append(i);
    }

    int nextIndex;
    if(!(tokens.isEmpty())) {
        nextIndex = (tokens.last()) + 1;
    }

    for(int i=0; i<tokens.count(); i++) {
        QList<QString> nstates = nfa->transitions.values(state + '|' + tagVector.at(tokens.at(i)).type);
        for(int j = 0; j < nstates.size(); j++) {
            done = true;
            QStack<QString> *tempStack;
            QVector<Tag*>* temp = simulateNFA(nfa, tempStack, nstates.at(j), nextIndex);
            if(temp != NULL) {
                Tag* t = new Tag(tagVector.at(tokens.at(i)).type, tagVector.at(tokens.at(i)).pos, tagVector.at(tokens.at(i)).length, sarf);
                temp->prepend(t);
                tags.append(temp);
                /// construct action stack for this match
                //QString tempState = "q";
                //int stateNumber = state.mid(1).toInt() +1;
                //tempState.append(QString::number(stateNumber));
                QList<QString> functionCalls = nfa->stateTOmsfMap.values(state);
                if(!(functionCalls.isEmpty())) {
                    qSort(functionCalls.begin(), functionCalls.end(), lessThan);
                }
                refineFunctions(nfa,functionCalls, tokens.at(i));
                for(int i=0; i< functionCalls.count(); i++) {
                    tempStack->push(functionCalls.at(i));
                }
                stacks.append(tempStack);
            }
        }
    }

    QList<QString> nstates =nfa->transitions.values(state + '|' + "epsilon");
    for(int j = 0; j < nstates.size(); j++) {
        done = true;
        QStack<QString> *tempStack;
        QVector<Tag*>* temp = simulateNFA(nfa, tempStack, nstates.at(j), tagIndex);
        if(temp != NULL) {
            tags.append(temp);
            /// construct action stack for this match
            QList<QString> functionCalls = nfa->stateTOmsfMap.values(state);
            if(!(functionCalls.isEmpty())) {
                qSort(functionCalls.begin(), functionCalls.end(), lessThan);
            }
            refineFunctions(nfa,functionCalls);
            for(int i=0; i< functionCalls.count(); i++) {
                tempStack->push(functionCalls.at(i));
            }
            stacks.append(tempStack);
        }
    }

    QString formula;
    /** check for formula transition **/
    if(!done) {
        for(int i=0; i<nfaVector->count(); i++) {
            QList<QString> nstates = nfa->transitions.values(state + '|' + nfaVector->at(i)->name);
            for(int j = 0; j < nstates.size(); j++) {
                formula = nfaVector->at(i)->name;
                QStack<QString> *tempStack;
                QVector<Tag*>* temp = simulateNFA(nfaVector->at(i), tempStack, nfaVector->at(i)->start, tagIndex);
                if(temp != NULL) {
                    tags.append(temp);
                    /// construct action stack for this match
                    QList<QString> functionCalls = nfa->stateTOmsfMap.values(state);
                    if(!(functionCalls.isEmpty())) {
                        qSort(functionCalls.begin(), functionCalls.end(), lessThan);
                    }
                    refineFunctions(nfa,functionCalls);
                    for(int i=0; i< functionCalls.count(); i++) {
                        tempStack->push(functionCalls.at(i));
                    }
                    stacks.append(tempStack);
                }
                break;
            }
        }
    }

    if(tags.count() != 0) {

        /** check if we have AND formula **/
        if(nfa->transitions.contains(state + "|*AND*")) {

            if((tags.count() == 2) && (tags.at(0)->count() == tags.at(1)->count())) {
                /// We have two matches referring to the two AND paths and both of same length

                int wordSkip = tags.at(0)->count();
                int index = tagIndex;
                for(int i=0; i<wordSkip; i++) {
                    int j=index;
                    while((j<tagVector.count()) && (tagVector.at(j).pos == tagVector.at(index).pos)) {
                        j++;
                    }
                    index = j;
                }

                QList<QString> nstates =nfa->transitions.values(state + "|*AND*");
                for(int j = 0; j < nstates.size(); j++) {
                    QStack<QString> *tempStack;
                    QVector<Tag*>* temp = simulateNFA(nfa, tempStack, nstates.at(j), index);
                    if(temp != NULL) {
                        tags.append(temp);
                        /// construct action stack for this match
                        QList<QString> functionCalls = nfa->stateTOmsfMap.values(state);
                        if(!(functionCalls.isEmpty())) {
                            qSort(functionCalls.begin(), functionCalls.end(), lessThan);
                        }
                        refineFunctions(nfa,functionCalls);
                        /// Add two branch stacks to current
                        for(int i=0; i< 2; i++) {
                            for(int k=0; k<stacks.at(i)->count(); k++) {
                                tempStack->push(stacks.at(i)->at(k));
                                //stacks.at(i)->pop_back();
                            }
                        }
                        /// Add AND actions including on match and successive ones
                        for(int i=0; i< functionCalls.count(); i++) {
                            tempStack->push(functionCalls.at(i));
                        }
                        delete (stacks.at(1));
                        stacks.remove(1);
                        delete (stacks.at(0));
                        stacks.remove(0);
                        stacks.append(tempStack);
                    }
                    else {
                        return NULL;
                    }
                }

                /** add AND tags to consequent match **/
                for(int i=2; i< tags.count(); i++) {
                    for(int j = (tags.at(1)->count()-1); j>=0 ; j--) {
                        tags.at(i)->prepend(tags.at(1)->at(j));
                    }
                }

                tags.remove(1);
                for(int i=0; i< tags.at(0)->count(); i++) {
                    delete tags.at(0)->at(i);
                }
                tags.remove(0);
            }
            else {
                return NULL;
            }
        }

        /** Check if we have a MSF use **/
        if(!(formula.isEmpty())) {
            if(!(tags.isEmpty())) {
                /// We have a match referring to the MSF path

                int wordSkip = tags.at(0)->count();
                int index = tagIndex;
                for(int i=0; i<wordSkip; i++) {
                    int j=index;
                    while((j<tagVector.count()) && (tagVector.at(j).pos == tagVector.at(index).pos)) {
                        j++;
                    }
                    index = j;
                }

                QList<QString> nstates =nfa->transitions.values(state + '|' + formula);
                for(int j = 0; j < nstates.size(); j++) {
                    QStack<QString> *tempStack;
                    QVector<Tag*>* temp = simulateNFA(nfa, tempStack, nstates.at(j), index);
                    if(temp != NULL) {
                        tags.append(temp);
                        /// construct action stack for this match
                        QList<QString> functionCalls = nfa->stateTOmsfMap.values(state);
                        if(!(functionCalls.isEmpty())) {
                            qSort(functionCalls.begin(), functionCalls.end(), lessThan);
                        }
                        refineFunctions(nfa,functionCalls);
                        /// Add formula stack to current
                        for(int i=0; i< stacks.count(); i++) {
                            for(int k=0; k<stacks.at(i)->count(); k++) {
                                tempStack->push(stacks.at(i)->at(k));
                                //stacks.at(i)->pop_back();
                            }
                        }
                        /// Add current node actions
                        for(int i=0; i< functionCalls.count(); i++) {
                            tempStack->push(functionCalls.at(i));
                        }
                        delete (stacks.at(0));
                        stacks.remove(0);
                        stacks.append(tempStack);
                    }
                    else {
                        return NULL;
                    }
                }

                /** add AND tags to consequent match **/
                for(int i=1; i< tags.count(); i++) {
                    for(int j = (tags.at(0)->count()-1); j>=0 ; j--) {
                        tags.at(i)->prepend(tags.at(0)->at(j));
                    }
                }

                for(int i=0; i< tags.at(0)->count(); i++) {
                    delete tags.at(0)->at(i);
                }
                tags.remove(0);
            }
            else {
                return NULL;
            }
        }

        /** get longest match and return it **/
        int maxCount = -1;
        int maxIndex;
        for(int i=0; i< tags.count(); i++) {
            if(tags.at(i)->count() > maxCount) {
                maxIndex = i;
                maxCount = tags.at(i)->count();
            }
        }

        /// Delete smaller matches
        for(int i=0; i< tags.count(); i++) {
            if(i != maxIndex) {
                for(int j=0; j< tags.at(i)->count(); j++) {
                    delete tags.at(i)->at(j);
                }
                delete (stacks.at(i));
                //stacks.remove(i);
            }
        }
        actionStack = stacks.at(maxIndex);
        return tags.at(maxIndex);
    }

    actionStack = NULL;
    return NULL;
}

QByteArray ATagger::dataInJsonFormat(Data _data) {
    QByteArray json;

    if(_data == sarfTTV) {
        QVariantMap sarftagtypedata;
        QVariantList sarftagtypeset;
        for( int i=0; i < _atagger->tagTypeVector->count(); i++) {
            QVariantMap data;
            SarfTagType* tagtype = (SarfTagType*)(tagTypeVector->at(i));
            data.insert("Tag",tagtype->tag);
            data.insert("Description",tagtype->description);
            data.insert("id",_atagger->tagTypeVector->count());
            data.insert("foreground_color",tagtype->fgcolor);
            data.insert("background_color",tagtype->bgcolor);
            data.insert("font",tagtype->font);
            data.insert("underline",tagtype->underline);
            data.insert("bold",tagtype->bold);
            data.insert("italic",tagtype->italic);

            QVariantList tagsarray;
            for(int j=0; j < tagtype->tags.count(); j++) {
                QVariantMap tagsdata;
                const Quadruple< QString , QString , QString , QString > * quad = &(tagtype->tags.at(j));
                tagsdata.insert(quad->first,quad->second);
                tagsdata.insert("Negation",quad->third);
                tagsdata.insert("Relation",quad->fourth);
                tagsarray << tagsdata;
            }
            data.insert("Features",tagsarray);

            sarftagtypeset << data;
        }
        sarftagtypedata.insert("TagTypeSet",sarftagtypeset);

        /** Convert MSFs to JSON **/

        if(!(_atagger->msfVector->isEmpty())) {

            QVariantList msfsList;
            for(int i=0; i<_atagger->msfVector->count(); i++) {
                msfsList << _atagger->msfVector->at(i)->getJSON();
            }
            sarftagtypedata.insert("MSFs", msfsList);
        }

        QJson::Serializer serializer;
        json = serializer.serialize(sarftagtypedata);
    }
    else if(_data == tagTV) {
        /** Convert TagType to QJSON **/

        QVariantMap tagtypedata;
        QVariantList tagtypeset;
        for(int i=0; i < _atagger->tagTypeVector->count(); i++) {
            QVariantMap data;
            data.insert("Tag",(_atagger->tagTypeVector->at(i))->tag);
            data.insert("Description",(_atagger->tagTypeVector->at(i))->description);
            data.insert("id",_atagger->tagTypeVector->count());
            data.insert("foreground_color",(_atagger->tagTypeVector->at(i))->fgcolor);
            data.insert("background_color",(_atagger->tagTypeVector->at(i))->bgcolor);
            data.insert("font",(_atagger->tagTypeVector->at(i))->font);
            data.insert("underline",(_atagger->tagTypeVector->at(i))->underline);
            data.insert("bold",(_atagger->tagTypeVector->at(i))->bold);
            data.insert("italic",(_atagger->tagTypeVector->at(i))->italic);

            tagtypeset << data;
        }
        tagtypedata.insert("TagTypeSet",tagtypeset);
        QJson::Serializer serializer;
        json = serializer.serialize(tagtypedata);
    }
    else if(_data == tagV || _data == sarfTV) {

        /** Convert Tag to JSON **/

        QVariantMap tagdata;
        tagdata.insert("file",_atagger->textFile);
        tagdata.insert("TagTypeFile",_atagger->tagtypeFile);
        tagdata.insert("textchecksum", _atagger->text.count());
        QVariantList tagset;
        for(int i=0; i<_atagger->tagVector.count(); i++) {
            QVariantMap data;
            data.insert("type",(_atagger->tagVector.at(i)).type);
            data.insert("pos",(_atagger->tagVector.at(i)).pos);
            data.insert("length",(_atagger->tagVector.at(i)).length);
            data.insert("source",(_atagger->tagVector.at(i)).source);
            tagset << data;
        }
        tagdata.insert("TagArray",tagset);
        QJson::Serializer serializer;
        json = serializer.serialize(tagdata);
    }
#if 0
    else if(_data == sarfMSF) {
        /** Convert MSFs to JSON **/

        QVariantMap msfsMap;
        msfsMap.insert("mbffile",_atagger->tagtypeFile);
        QVariantList msfsList;
        for(int i=0; i<_atagger->msfVector->count(); i++) {
            msfsList << _atagger->msfVector->at(i)->getJSON();
        }
        msfsMap.insert("MSFs",msfsList);
        QJson::Serializer serializer;
        json = serializer.serialize(msfsMap);
    }
#endif

    return json;
}

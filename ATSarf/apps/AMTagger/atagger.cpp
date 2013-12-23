#include "atagger.h"
#include <QVariant>
#include <qjson/serializer.h>
#include <QFile>
#include <QTextStream>
#include <dlfcn.h>
#include <stdlib.h>

ATagger * _atagger = NULL;

ATagger::ATagger() {
    tagTypeVector = new QVector<TagType*>();
    compareToTagTypeVector = new QVector<TagType*>();
    msfVector = new QVector<MSFormula*>();
    nfaVector = new QVector<NFA*>();
    isTagMBF = true;
}

ATagger::~ATagger() {
    tagHash.clear();
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

bool ATagger::insertTag(const TagType* type, int pos, int length, int wordIndex, Source source, Dest dest) {

    Tag tag(type,pos,length,wordIndex,source);
    if(dest == original) {
        if(!(tagHash.contains(wordIndex,tag))) {
            tagHash.insert(wordIndex,tag);
        }
        else {
            return false;
        }
    }
    else {
        if(!(compareToTagHash.contains(wordIndex,tag))) {
            compareToTagHash.insert(wordIndex,tag);
        }
        else {
            return false;
        }
    }
    return true;
}

bool ATagger::insertTagType(QString tag, QString desc, QString fgcolor, QString bgcolor, int font, bool underline, bool bold, bool italic, Source source, Dest dest) {

    TagType* tt = new TagType(tag,desc,fgcolor,bgcolor,font,underline,bold,italic,source);
    if(dest == original) {
        tagTypeVector->append(tt);
    }
    else {
        compareToTagTypeVector->append(tt);
    }
    return true;
}

bool ATagger::insertSarfTagType(QString tag, QVector<Quadruple< QString , QString , QString , QString > > tags, QString desc, QString fgcolor, QString bgcolor, int font, bool underline, bool bold, bool italic, Source source, Dest dest) {
    SarfTagType* stt = new SarfTagType(tag,tags,desc,fgcolor,bgcolor,font,underline,bold,italic,source);
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
        NFA* nfa = new NFA(msfVector->at(i));
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

void ATagger::executeActions(NFA* nfa, int index) {

    /** Add function calls of each formula to a function named msfName() **/
    MSFormula* formula = (MSFormula*)nfa->formula;
    formula->actionData.append("extern \"C\" void " + formula->name + "_actions() {\n");
    for(int i=index; i<simulationVector.count(); i++) {
        MERFTag* mTag = (MERFTag*)(simulationVector.at(i));
        MSFormula* _formula = mTag->formula;
        if(_formula->name == formula->name) {
            mTag->match->executeActions(nfa);
        }
    }
    formula->actionData.append("}\n");

    /** write action functions in .cpp output files, compile shared library, and call them **/
    QString msfName = formula->name;
    QString path = tagFile.left(tagFile.lastIndexOf('/')+1);
    QString cppFile = path + msfName + ".cpp";
    QFile f(cppFile);
    f.open(QIODevice::ReadWrite|QIODevice::Truncate|QIODevice::Text);
    QTextStream out(&f);
    out.flush();
    out << formula->actionData;

    f.close();

    // create library
    QString command = "/usr/bin/g++ -fPIC -shared " + cppFile + " -o " + path + "lib" + msfName + ".so";
    system (command.toStdString().c_str());

    // load library
    QString sharedLib = path + "lib" + msfName + ".so";
    void * fLib = dlopen ( sharedLib.toStdString().c_str(), RTLD_LAZY );
    if ( !fLib ) {
        cerr << "Cannot open library: " << dlerror() << '\n';
        return;
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

void ATagger::constructRelations(int index) {

    for(int i=index; i<simulationVector.count(); i++) {
        MERFTag* merftag = (MERFTag*)(simulationVector[i]);
        MSFormula* formula = (MSFormula*)(merftag->formula);
        for(int j=0; j<formula->relationVector.count(); j++) {
            Relation* relation = formula->relationVector[j];
            Match* entity1 = NULL;
            Match* entity2 = NULL;
            Match* edge = NULL;
            if(merftag->match->constructRelation(relation,entity1,entity2,edge)) {
                RelationM* relM = new RelationM(relation,entity1,entity2,edge);
                merftag->relationMatchVector.append(relM);
            }
        }
    }
}

void ATagger::drawNFA() {
    for(int i=0; i< _atagger->nfaVector->count(); i++) {
        NFA* nfa = _atagger->nfaVector->at(i);

        QString path = tagFile.left(tagFile.lastIndexOf('/')+1);
        QString cppFile = path + nfa->formula->name + ".dot";
        QFile f(cppFile);
        f.open(QIODevice::WriteOnly);
        QTextStream out(&f);
        out << "digraph " << nfa->formula->name << " {\n";

        QMapIterator<QString, QString> iterator(nfa->transitions);
        while (iterator.hasNext()) {
             iterator.next();
             QString key = iterator.key();
             QString value = iterator.value();

             QString sourceState = key.section('|',0,0);
             QString transitionKey = key.section('|',1,1);
             out << sourceState << " -> " << value;

             if(transitionKey.compare("epsilon") == 0) {
                 out << " [label=<&#949;>];\n";
             }
             else {
                 out << " [label=" << transitionKey << "];\n";
             }
         }
        out << '}';
        f.close();
    }
}

bool ATagger::runSimulator() {

    /// Build NFA
    if(!buildNFA()) {
        return false;
    }

    drawNFA();

    if(!buildActionFile()) {
        return false;
    }

    /// Simulate NFAs referring to all the MSFs built
    simulationVector.clear();
    for(int i=0; i<nfaVector->count(); i++) {
        int lastCount = simulationVector.count();
        /// Simulate current MSF starting from a set of tokens referring to a single word
        for(int j=1; j<=wordCount; j++) {
            Match* match = simulateNFA(nfaVector->at(i), nfaVector->at(i)->start, j);
            if(match != NULL && match->getLength() > 0) {
                MERFTag* merftag = (MERFTag*)(match->parent);
                merftag->pos = merftag->getPOS();
                merftag->length = merftag->getLength();
                simulationVector.append(merftag);

                /// Skip the words in the match
                int matchCount = merftag->getMatchCount();
                j = j + matchCount -1;
            }
            else {
                if(match != NULL) {
                    delete match;
                }
            }
        }
        /** Execute Actions **/
        if(lastCount != simulationVector.count()) {
            executeActions(nfaVector->at(i), lastCount);
            constructRelations(lastCount);
        }
        /** Done **/
    }

    /** Clean data in MSF vector **/
    for(int i=0; i<msfVector->count();i++) {
        msfVector->at(i)->actionData.clear();
        msfVector->at(i)->functionParametersMap.clear();
    }
    return true;
}

Match* ATagger::simulateNFA(NFA* nfa, QString state, int wordIndex) {
    if((state == nfa->accept) || (nfa->andAccept.contains(state))) {
        /// initialize a MERFTag and return it
        MERFTag* merftag = new MERFTag((MSFormula*)(nfa->formula),sarf);
        SequentialM* seq = new SequentialM(merftag);
        merftag->setMatch(seq);
        seq->msf = NULL;
        return seq;
    }

    /** checked when transition found from MBF before MSF**/
    bool done = false;

    /// Use a vector of matches to collect correct solutions and choose longest
    QVector<Match*> matches;

    /// Get the tags for word at wordIndex
    QList<Tag> tokens;
    if(tagHash.contains(wordIndex)) {
        tokens = tagHash.values(wordIndex);
    }
    else {
        /// Word is tagged by NONE
        int pos = wordIndexMap.key(wordIndex);
        Word word = nextWord(text, pos);
        int length = word.end-word.start+1;
        Tag t(NULL,pos,length,wordIndex,sarf);
        tokens.append(t);
    }

    for(int i=0; i<tokens.count(); i++) {
        QList<QString> nstates;
        if(tokens.at(i).tagtype == NULL) {
            nstates = nfa->transitions.values(state + "|NONE");
        }
        else {
            nstates = nfa->transitions.values(state + '|' + tokens.at(i).tagtype->name);
        }
        for(int j = 0; j < nstates.size(); j++) {
            done = true;
            int nextWordIndex = wordIndex;
            if(_atagger->isStatementEndSet.contains(wordIndex)) {
                nextWordIndex = (_atagger->wordCount) + 1;
            }
            else {
                nextWordIndex++;
            }
            Match* temp = simulateNFA(nfa, nstates.at(j), nextWordIndex);
            if(temp != NULL) {
                /** Update Match **/
                KeyM* keyMatch = (KeyM*)temp;
                if(tokens.at(i).tagtype == NULL) {
                    keyMatch->key = "NONE";
                }
                else {
                    keyMatch->key = tokens.at(i).tagtype->name;
                }
                keyMatch->length = tokens.at(i).length;
                keyMatch->pos = tokens.at(i).pos;
                keyMatch->msf = nfa->stateTOmsfMap.value(state).first;
                keyMatch->word = text.mid(tokens.at(i).pos,tokens.at(i).length);
                temp = temp->parent;
                /** Done **/
                matches.append(temp);
            }
        }
    }

    QList<QString> nstates =nfa->transitions.values(state + '|' + "epsilon");
    for(int j = nstates.size()-1; j >=0 ; j--) {
        done = true;
        Match* temp = simulateNFA(nfa,nstates.at(j),wordIndex);
        if(temp != NULL) {
            /** Update Match **/
            if(nfa->stateTOmsfMap.contains(state)) {
                QPair<MSF*,QString> pair = nfa->stateTOmsfMap.value(state);
                MSF* msf = pair.first;
                if(pair.second.compare("pre") == 0) {
                    /** Check for case of empty Unary Solution with STAR operation **/

                    if(temp->getLength()<=0) {
                        Match* tmp = temp;
                        temp = temp->parent;
                        if(temp->isSequentialM()) {
                            SequentialM* seq = (SequentialM*)temp;
                            /// Remove empty match from parent if parent sequential
                            for(int i=0; i<seq->matches.count(); i++) {
                                if(seq->matches.at(i) == tmp) {
                                    seq->matches.remove(i);
                                }
                            }
                        }
                        else if(temp->isBinaryM()) {
                            BinaryM* binary = (BinaryM*)temp;
                            /// Remove empty solution from parent if parent binary
                            if(binary->leftMatch == tmp) {
                                binary->leftMatch = NULL;
                            }
                            else if(binary->rightMatch == tmp) {
                                binary->rightMatch = NULL;
                            }
                        }
                        delete tmp;
                    }
                    else {
                        temp = temp->parent;
                    }
                    /** Done **/
                }
                else {
                    if(msf->isMBF()) {
                        KeyM* keym = new KeyM(temp,"",-1,-1);
                        keym->msf = msf;
                        temp->setMatch(keym);
                        temp = keym;
                    }
                    else if(msf->isUnary()) {
                        UNARYF* unary = (UNARYF*)msf;
                        UnaryM* unarym = new UnaryM(unary->op,temp,unary->limit);
                        unarym->msf = msf;
                        temp->setMatch(unarym);
                        temp = unarym;
                    }
                    else if(msf->isBinary()) {
                        /// AND case is resolved down in code
                        BINARYF* binary = (BINARYF*)msf;
                        if(binary->op == OR) {
                            BinaryM* binarym = new BinaryM(OR,temp);
                            binarym->msf = msf;
                            temp->setMatch(binarym);
                            temp = binarym;
                        }
                    }
                    else if(msf->isSequential()) {
                        SequentialM* seqm = new SequentialM(temp);
                        seqm->msf = msf;
                        temp->setMatch(seqm);
                        temp = seqm;
                    }
                    else {
                        /// TODO: Case of a formula
                    }
                }
            }
            /** Done **/
            matches.append(temp);
        }
    }

    bool isSpecial = false;
    MSFormula* formula = NULL;
    /** check for formula transition **/
    if(!done) {
        for(int i=0; i<nfaVector->count(); i++) {
            if(!(nfa->transitions.contains(state + '|' + nfaVector->at(i)->formula->name))) {
                continue;
            }
            QList<QString> nstates = nfa->transitions.values(state + '|' + nfaVector->at(i)->formula->name);
            for(int j = 0; j < nstates.size(); j++) {
                isSpecial = true;
                formula = (MSFormula*)(nfaVector->at(i)->formula);
                Match* temp = simulateNFA(nfaVector->at(i),nfaVector->at(i)->start,wordIndex);
                if(temp != NULL) {
                    matches.append(temp);
                }
            }
            break;
        }
    }

    if(matches.count() != 0) {

        /** check if we have AND formula **/
        if(nfa->transitions.contains(state + "|*AND*")) {

            isSpecial = true;
            if((matches.count() == 2) && (matches.at(0)->getMatchCount() == matches.at(1)->getMatchCount())) {
                /// We have two matches referring to the two AND paths and both of same length

                int wordSkip = matches.at(0)->getMatchCount();
                int nextWordIndex = wordIndex + wordSkip;

                QList<QString> nstates =nfa->transitions.values(state + "|*AND*");
                for(int j = 0; j < nstates.size(); j++) {
                    Match* temp = simulateNFA(nfa,nstates.at(j),nextWordIndex);
                    if(temp != NULL) {
                        matches.append(temp);
                    }
                    else {
                        return NULL;
                    }
                }

                /** Update Match: add AND matches to consequent match **/
                QPair<MSF*,QString> pair = nfa->stateTOmsfMap.value(state);
                MSF* msf = pair.first;

                Match* parent = matches[2];
                BinaryM* binarym = new BinaryM(AND,parent);
                binarym->msf = msf;
                parent->setMatch(binarym);
                MERFTag* mt1 = (MERFTag*)(matches[0]);
                SequentialM* seq1 = (SequentialM*)(mt1->match);
                if(seq1->matches.count()>1) {
                    binarym->setMatch(seq1);
                    seq1->parent = binarym;
                }
                else {
                    binarym->setMatch(seq1->matches[0]);
                    seq1->matches[0]->parent = binarym;
                }
                MERFTag* mt2 = (MERFTag*)(matches[1]);
                SequentialM* seq2 = (SequentialM*)(mt2->match);
                if(seq2->matches.count()>1) {
                    binarym->setMatch(seq2);
                    seq2->parent = binarym;
                }
                else {
                    binarym->setMatch(seq2->matches[0]);
                    seq2->matches[0]->parent = binarym;
                }
                matches.clear();
                matches.append(parent);
                /** Done **/
            }
            else {
                for(int i=0; i<matches.count(); i++) {
                    delete matches[i];
                }
                matches.clear();
                return NULL;
            }
        }

        /** Check if we have a MSF use **/
        if(formula != NULL) {
            if(!(matches.isEmpty())) {
                /// We have a match referring to the MSF path

                int wordSkip = matches.at(0)->getMatchCount();
                int nextWordIndex = wordIndex + wordSkip;

                QList<QString> nstates =nfa->transitions.values(state + '|' + formula->name);
                for(int j = 0; j < nstates.size(); j++) {
                    Match* temp = simulateNFA(nfa,nstates.at(j),nextWordIndex);
                    if(temp != NULL) {
                        matches.append(temp);
                    }
                    else {
                        return NULL;
                    }
                }

                /** Update Match: add formula matches to consequent match **/
                QPair<MSF*,QString> pair = nfa->stateTOmsfMap.value(state);
                MSF* msf = pair.first;

                /*
                Match* parent = matches[1];
                MERFTag* mtag = new MERFTag(formula,sarf);
                mtag->msf = msf;
                mtag->parent = parent;
                parent->setMatch(mtag);
                mtag->setMatch(matches[0]);
                matches[0]->parent = mtag;
                matches.clear();
                matches.append(mtag);
                */

                Match* parent = matches[1];
                matches[0]->msf = msf;
                matches[0]->parent = parent;
                parent->setMatch(matches[0]);
                matches.clear();
                matches.append(parent);
                /** Done **/
            }
            else {
                return NULL;
            }
        }

        /** get longest match and return it **/
        int maxCount = -1;
        int maxIndex;
        /*
        if(isSpecial) {
            /// This is the case where we have a subformula or AND operation
            /// In such a case, the matches are connected to each other and shouldn't be deleted
            if(formula != NULL) {
                /// We have a formula
                return matches.at(1);
            }
            else {
                /// We have an AND operation
                return matches.at(2);
            }

        }
        */
        for(int i=0; i< matches.count(); i++) {
            Match* temp = matches[i];
            while(temp->parent != NULL) {
                temp = temp->parent;
            }
            int tempMatchCount = temp->getMatchCount();
            if( tempMatchCount > maxCount) {
                maxIndex = i;
                maxCount = tempMatchCount;
            }
        }

        /// Delete smaller matches
        for(int i=0; i< matches.count(); i++) {
            if(i != maxIndex) {
                Match* temp = matches[i];
                while(temp->parent != NULL) {
                    temp = temp->parent;
                }
                delete temp;
            }
        }
        return matches.at(maxIndex);
    }
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
            data.insert("Tag",tagtype->name);
            data.insert("Description",tagtype->description);
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
            data.insert("Tag",(_atagger->tagTypeVector->at(i))->name);
            data.insert("Description",(_atagger->tagTypeVector->at(i))->description);
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
    else if(_data == tagV || _data == sarfTV || _data == sarfMSF) {

        /** Convert Tag to JSON **/

        QVariantMap tagdata;
        tagdata.insert("file",_atagger->textFile);
        tagdata.insert("TagTypeFile",_atagger->tagtypeFile);
        tagdata.insert("textchecksum", _atagger->text.count());
        tagdata.insert("version", 1.0);
        QVariantList tagset;
        QHashIterator<int, Tag> iTag(_atagger->tagHash);
        while (iTag.hasNext()) {
            iTag.next();
            QVariantMap data;
            data.insert("type",iTag.value().tagtype->name);
            data.insert("pos",iTag.value().pos);
            data.insert("length",iTag.value().length);
            data.insert("wordIndex",iTag.value().wordIndex);
            data.insert("source",iTag.value().source);
            tagset << data;
        }
        tagdata.insert("TagArray",tagset);

        if(!(_atagger->simulationVector.isEmpty())) {
            QVariantList simulationList;
            for(int i=0; i<_atagger->simulationVector.count(); i++) {
                MERFTag *mtag = (MERFTag*)(_atagger->simulationVector.at(i));
                MSFormula* formula = mtag->formula;
                QVariantMap data;
                data.insert("formula", formula->name);
                data.insert("pos", mtag->pos);
                data.insert("length", mtag->length);
                data.insert("source",mtag->source);
                if(mtag->match != NULL) {
                    data.insert("match",mtag->match->getJSON());
                }

                /*
                if(!(mtag->relationMatchVector.isEmpty())) {
                    QVariantList relMatchList;
                    for(int j=0; j<mtag->relationMatchVector.count(); j++) {
                        RelationM* relm = mtag->relationMatchVector[j];
                        QVariantMap relmData;
                        relmData.insert("relation",relm->relation->name);
                        relmData.insert("e1Label",relm->e1Label);
                        relmData.insert("e2Label",relm->e2Label);
                        relmData.insert("edgeLabel",relm->edgeLabel);
                        relMatchList << relmData;
                    }
                    data.insert("relationMatches",relMatchList);
                }
                */
                simulationList << data;
            }
            tagdata.insert("simulationTags",simulationList);
        }

        QJson::Serializer serializer;
        json = serializer.serialize(tagdata);
    }

    return json;
}

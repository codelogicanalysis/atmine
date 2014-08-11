#include "atagger.h"
#include "ger.h"
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
    tagHash = new QMultiHash<int,Tag*>();
    isTagMBF = true;
    uniqueID = 0;
}

ATagger::~ATagger() {
    tagHash->clear();
    delete tagHash;
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

bool ATagger::insertTag(const TagType* type, int pos, int length, int wordIndex, Source source, Dest dest, int id) {

    Tag* tag;
    if(id == -1) {
        tag = new Tag(type,pos,length,wordIndex,source,uniqueID,_atagger->textFile.split('/').last());
    }
    else {
        tag = new Tag(type,pos,length,wordIndex,source,id,_atagger->textFile.split('/').last());
    }
    if(dest == original) {
        bool insert = true;
        QMultiHash<int,Tag*>::iterator it;
        for(it=tagHash->begin(); it != tagHash->end(); ++it) {
            if((it.value()->pos == pos) && (it.value()->tagtype->name == type->name)) {
                insert = false;
                break;
            }
        }
        if(insert) {
            tagHash->insert(wordIndex,tag);
        }
        else {
            return false;
        }
    }
    else {
        bool insert = true;
        QMultiHash<int,Tag*>::iterator it;
        for(it=compareToTagHash.begin(); it != compareToTagHash.end(); ++it) {
            if((it.value()->pos == pos) && (it.value()->tagtype->name == type->name)) {
                insert = false;
                break;
            }
        }
        if(insert) {
            compareToTagHash.insert(wordIndex,tag);
        }
        else {
            return false;
        }
    }
    uniqueID++;
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
    formula->actionData.append("extern \"C\" void " + formula->name + "_" + textFile.split('/').last().remove(".txt") + "_actions() {\n");
    for(int i=index; i<simulationVector.count(); i++) {
        MERFTag* mTag = (MERFTag*)(simulationVector.at(i));
        MSFormula* _formula = mTag->formula;
        if(_formula->name == formula->name) {
            mTag->match->executeActions(nfa);
        }
    }
    formula->actionData.append("}\n\n");

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
        QString actionFunction = msfName + '_' + textFile.split('/').last().remove(".txt")  + "_actions";
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
            QVector<Match*> entity1;
            QVector<Match*> entity2;
            QVector<Match*> edge;
            merftag->match->constructRelation(relation,entity1,entity2,edge);
            if(!(entity1.isEmpty()) && !(entity2.isEmpty()) && (!(edge.isEmpty()) || (relation->edge == NULL))) {
                if((entity1.count() == 1) && (entity2.count() == 1) && (edge.count() == 1)) {
                    /// single relation
                    RelationM* relM = new RelationM(relation,entity1[0],entity2[0],edge[0]);
                    merftag->relationMatchVector.append(relM);
                }
                else if((entity1.count() == 1) && (entity2.count() == 1) && (edge.isEmpty())) {
                    /// single with no edge
                    RelationM* relM = new RelationM(relation,entity1[0],entity2[0],NULL);
                    merftag->relationMatchVector.append(relM);
                }
                else if(edge.count() > 1) {
                    /// cases where the edge has multiple matches

                    /// extract the edge label first
                    QString edgeLabel;
                    if(relation->edgeLabel == "text") {
                        for(int k=0; k<edge.count(); k++) {
                            edgeLabel.append(edge.at(k)->getText() + ", ");
                        }
                        edgeLabel.chop(2);
                    }
                    else if(relation->edgeLabel == "position") {
                        for(int k=0; k<edge.count(); k++) {
                            edgeLabel.append(QString::number(edge.at(k)->getPOS()) + ", ");
                        }
                        edgeLabel.chop(2);
                    }
                    else if(relation->edgeLabel == "length") {
                        for(int k=0; k<edge.count(); k++) {
                            edgeLabel.append(QString::number(edge.at(k)->getLength()) + ", ");
                        }
                        edgeLabel.chop(2);
                    }
                    else if(relation->edgeLabel == "number") {
                        for(int k=0; k<edge.count(); k++) {
                            QString text = edge.at(k)->getText();
                            NumNorm nn(&text);
                            nn();
                            int number = NULL;
                            if(nn.extractedNumbers.count()!=0) {
                                number = nn.extractedNumbers[0].getNumber();
                                 edgeLabel.append(QString::number(number) + ", ");
                            }
                        }
                        if(!(edgeLabel.isEmpty())) {
                            edgeLabel.chop(2);
                        }
                    }

                    if(entity1.count() == 1 && entity2.count() == 1) {
                        RelationM* relM = new RelationM(relation,entity1[0],entity2[0],NULL);
                        relM->edgeLabel = edgeLabel;
                        merftag->relationMatchVector.append(relM);
                    }
                    else if(relation->entity1->name == relation->entity2->name) {
                        /// Relation entities belong to a + or *
                        for(int k=1; k< entity1.count(); k++) {
                            RelationM* relM = new RelationM(relation,entity1[k-1],entity2[k],NULL);
                            relM->edgeLabel = edgeLabel;
                            merftag->relationMatchVector.append(relM);
                        }
                    }
                    else if(entity1.count() > 1 && entity2.count() == 1) {
                        /// multiple matches for entity1 and 1 for entity 2
                        for(int k=0; k<entity1.count(); k++) {
                            RelationM* relM = new RelationM(relation,entity1[k],entity2[0],NULL);
                            relM->edgeLabel = edgeLabel;
                            merftag->relationMatchVector.append(relM);
                        }
                    }
                    else if(entity1.count() == 1 && entity2.count() > 1) {
                        /// multiple matches for entity2 and 1 for entity 1
                        for(int k=0; k<entity2.count(); k++) {
                            RelationM* relM = new RelationM(relation,entity1[0],entity2[k],NULL);
                            relM->edgeLabel = edgeLabel;
                            merftag->relationMatchVector.append(relM);
                        }
                    }
                    else {
                        /// multiple matches for entities 1 and 2
                        for(int m=0; m<entity1.count(); m++) {
                            for(int n=0; n<entity2.count(); n++) {
                                RelationM* relM = new RelationM(relation,entity1[m],entity2[n],NULL);
                                relM->edgeLabel = edgeLabel;
                                merftag->relationMatchVector.append(relM);
                            }
                        }
                    }
                }
                else {
                    /// cases where the edge has only one match

                    if(relation->entity1->name == relation->entity2->name) {
                        /// Relation entities belong to a + or *
                        for(int k=1; k< entity1.count(); k++) {
                            RelationM* relM;
                            if(edge.count() == 1) {
                                relM = new RelationM(relation,entity1[k-1],entity2[k],edge[0]);
                            }
                            else {
                                relM = new RelationM(relation,entity1[k-1],entity2[k],NULL);
                            }
                            merftag->relationMatchVector.append(relM);
                        }
                    }
                    else if(entity1.count() > 1 && entity2.count() == 1) {
                        /// multiple matches for entity1 and 1 for entity 2
                        for(int k=0; k<entity1.count(); k++) {
                            RelationM* relM;
                            if(edge.count() == 1) {
                                relM = new RelationM(relation,entity1[k],entity2[0],edge[0]);
                            }
                            else {
                                relM = new RelationM(relation,entity1[k],entity2[0],NULL);
                            }
                            merftag->relationMatchVector.append(relM);
                        }
                    }
                    else if(entity1.count() == 1 && entity2.count() > 1) {
                        /// multiple matches for entity2 and 1 for entity 1
                        for(int k=0; k<entity2.count(); k++) {
                            RelationM* relM;
                            if(edge.count() == 1) {
                                relM = new RelationM(relation,entity1[0],entity2[k],edge[0]);
                            }
                            else {
                                relM = new RelationM(relation,entity1[0],entity2[k],NULL);
                            }
                            merftag->relationMatchVector.append(relM);
                        }
                    }
                    else {
                        /// multiple matches for entities 1 and 2
                        for(int m=0; m<entity1.count(); m++) {
                            for(int n=0; n<entity2.count(); n++) {
                                RelationM* relM;
                                if(edge.count() == 1) {
                                    relM = new RelationM(relation,entity1[m],entity2[n],edge[0]);
                                }
                                else {
                                    relM = new RelationM(relation,entity1[m],entity2[n],NULL);
                                }
                                merftag->relationMatchVector.append(relM);
                            }
                        }
                    }
                }
            }
        }
    }
}

void ATagger::constructCrossRelations() {

    QHash<QString,QSet<QString>* > synSetHash;
    QVector<QPair<Match*, Match*> > crossRelations;
    for(int i=0; i<_atagger->simulationVector.count(); i++) {
        MERFTag* merftag1 = (MERFTag*)(simulationVector[i]);
        for(int j=0; j<_atagger->simulationVector.count(); j++) {
            MERFTag* merftag2 = (MERFTag*)(simulationVector[j]);

            if(merftag1 == merftag2) {
                continue;
            }

            for(int m=0; m<merftag1->relationMatchVector.count(); m++) {
                RelationM* relation1 = merftag1->relationMatchVector[m];
                for(int n=0; n<merftag2->relationMatchVector.count(); n++) {
                    RelationM* relation2 = merftag2->relationMatchVector[n];

                    QString r1e1Label = relation1->entity1->getText();
                    QString r1e2Label = relation1->entity2->getText();
                    QString r2e1Label = relation2->entity1->getText();
                    QString r2e2Label = relation2->entity2->getText();

                    QSet<QString>* set1;
                    if(synSetHash.contains(r1e1Label)) {
                        set1 = synSetHash.value(r1e1Label);
                    }
                    else {
                        GER ger1(r1e1Label,0,2);
                        ger1();
                        set1 = ger1.wStem;
                        synSetHash.insert(r1e1Label,set1);
                    }

                    QSet<QString>* set2;
                    if(synSetHash.contains(r1e2Label)) {
                        set2 = synSetHash.value(r1e2Label);
                    }
                    else {
                        GER ger2(r1e2Label,0,2);
                        ger2();
                        set2 = ger2.wStem;
                        synSetHash.insert(r1e2Label,set2);
                    }

                    Gamma gamma1(&r2e1Label);
                    gamma1();
                    QStringList * stems1 = gamma1.getStems();
                    Gamma gamma2(&r2e2Label);
                    gamma2();
                    QStringList * stems2 = gamma2.getStems();

                    /// Relation1 entity 1 with relation 2 entity1
                    if(relation1->entity1 != relation2->entity1) {
                        bool skip = false;
                        for(int l=0; l<crossRelations.count(); l++) {
                            if((relation1->entity1 == crossRelations.at(l).first &&
                               relation2->entity1 == crossRelations.at(l).second) ||
                               (relation2->entity1 == crossRelations.at(l).first &&
                                relation1->entity1 == crossRelations.at(l).second)) {
                                skip = true;
                                break;
                            }
                        }
                        if(!skip) {
                            for(int l=0; l<stems1->count(); l++) {
                                if(set1->contains(stems1->at(l))) {
                                    RelationM* rel = new RelationM(NULL,relation1->entity1,
                                                                   r1e1Label,relation2->entity1,
                                                                   r2e1Label,NULL,"cross-reference");
                                    crossRelationVector.append(rel);
                                    break;
                                }
                            }
                            crossRelations.append(QPair<Match*,Match*>(relation1->entity1,relation2->entity1));
                        }
                    }
                    /// Relation1 entity 1 with relation 2 entity2
                    if(relation1->entity1 != relation2->entity2) {
                        bool skip = false;
                        for(int l=0; l<crossRelations.count(); l++) {
                            if((relation1->entity1 == crossRelations.at(l).first &&
                               relation2->entity2 == crossRelations.at(l).second) ||
                               (relation2->entity2 == crossRelations.at(l).first &&
                                relation1->entity1 == crossRelations.at(l).second)) {
                                skip = true;
                                break;
                            }
                        }
                        if(!skip) {
                            for(int l=0; l<stems2->count(); l++) {
                                if(set1->contains(stems2->at(l))) {
                                    RelationM* rel = new RelationM(NULL,relation1->entity1,
                                                                   r1e1Label,relation2->entity2,
                                                                   r2e2Label,NULL,"cross-reference");
                                    crossRelationVector.append(rel);
                                    break;
                                }
                            }
                            crossRelations.append(QPair<Match*,Match*>(relation1->entity1,relation2->entity2));
                        }
                    }
                    /// Relation1 entity 2 with relation 2 entity1
                    if(relation1->entity2 != relation2->entity1) {
                        bool skip = false;
                        for(int l=0; l<crossRelations.count(); l++) {
                            if((relation1->entity2 == crossRelations.at(l).first &&
                               relation2->entity1 == crossRelations.at(l).second) ||
                               (relation2->entity1 == crossRelations.at(l).first &&
                                relation1->entity2 == crossRelations.at(l).second)) {
                                skip = true;
                                break;
                            }
                        }
                        if(!skip) {
                            for(int l=0; l<stems1->count(); l++) {
                                if(set2->contains(stems1->at(l))) {
                                    RelationM* rel = new RelationM(NULL,relation1->entity2,
                                                                   r1e2Label,relation2->entity1,
                                                                   r2e1Label,NULL,"cross-reference");
                                    crossRelationVector.append(rel);
                                    break;
                                }
                            }
                            crossRelations.append(QPair<Match*,Match*>(relation1->entity2,relation2->entity1));
                        }
                    }
                    /// Relation1 entity 2 with relation 2 entity2
                    if(relation1->entity2 != relation2->entity2) {
                        bool skip = false;
                        for(int l=0; l<crossRelations.count(); l++) {
                            if((relation1->entity2 == crossRelations.at(l).first &&
                               relation2->entity2 == crossRelations.at(l).second) ||
                               (relation2->entity2 == crossRelations.at(l).first &&
                                relation1->entity2 == crossRelations.at(l).second)) {
                                skip = true;
                                break;
                            }
                        }
                        if(!skip) {
                            for(int l=0; l<stems2->count(); l++) {
                                if(set2->contains(stems2->at(l))) {
                                    RelationM* rel = new RelationM(NULL,relation1->entity2,
                                                                   r1e2Label,relation2->entity2,
                                                                   r2e2Label,NULL,"cross-reference");
                                    crossRelationVector.append(rel);
                                    break;
                                }
                            }
                            crossRelations.append(QPair<Match*,Match*>(relation1->entity2,relation2->entity2));
                        }
                    }
                }
            }
        }
    }

    QHashIterator<QString,QSet<QString>* > i(synSetHash);
     while (i.hasNext()) {
         i.next();
         i.value()->clear();
         delete i.value();
     }
     synSetHash.clear();
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

bool ATagger::runSimulator(bool isBatch) {

    /// Build NFA
    if(!isBatch) {
        if(!buildNFA()) {
            return false;
        }

        drawNFA();

        if(!buildActionFile()) {
            return false;
        }

        simulationVector.clear();
    }

    /// Simulate NFAs referring to all the MSFs built
    for(int i=0; i<nfaVector->count(); i++) {
        int lastCount = simulationVector.count();
        /// Simulate current MSF starting from a set of tokens referring to a single word
        for(int j=1; j<=wordCount; j++) {
            Match* match = simulateNFA(nfaVector->at(i), nfaVector->at(i)->start, j);
            if(match != NULL && match->getLength() > 0) {
                MERFTag* merftag = (MERFTag*)(match->parent);
                merftag->pos = merftag->getPOS();
                merftag->length = merftag->getLength();
                merftag->id = uniqueID;
                uniqueID++;
                merftag->sourceText = textFile.split('/').last();
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

    return true;
}

Match* ATagger::simulateNFA(NFA* nfa, QString state, int wordIndex) {
    if((state == nfa->accept) || (nfa->andAccept.contains(state))) {
        /// initialize a MERFTag and return it
        MERFTag* merftag = new MERFTag((MSFormula*)(nfa->formula),sarf);
        SequentialM* seq = new SequentialM(merftag,uniqueID);
        uniqueID++;
        merftag->setMatch(seq);
        seq->msf = NULL;
        return seq;
    }

    /** checked when transition found from MBF before MSF**/
    bool done = false;

    /// Use a vector of matches to collect correct solutions and choose longest
    QVector<Match*> matches;
    QVector<QString> priorityTransitions;

    /// Get the tags for word at wordIndex
    QList<Tag*> tokens;
    if(tagHash->contains(wordIndex)) {
        tokens = tagHash->values(wordIndex);
    }
    else {
        /// Word is tagged by NONE
        int pos = wordIndexMap.key(wordIndex);
        Word word = nextWord(text, pos);
        int length = word.end-word.start+1;
        Tag* t = new Tag(NULL,pos,length,wordIndex,sarf,uniqueID);
        uniqueID++;
        tokens.append(t);
    }

    for(int i=0; i<tokens.count(); i++) {
        QList<QString> nstates;
        if(tokens.at(i)->tagtype == NULL) {
            nstates = nfa->transitions.values(state + "|NONE");
        }
        else {
            nstates = nfa->transitions.values(state + '|' + tokens.at(i)->tagtype->name);
        }
        for(int j = 0; j < nstates.size(); j++) {
            done = true;
            int nextWordIndex = wordIndex;

            /** check if end of statement based on full stop or punctuation criteria **/
            if(((MSFormula*)(nfa->formula))->isFullStop) {
                if(_atagger->isStatementEndFSSet.contains(wordIndex)) {
                    nextWordIndex = (_atagger->wordCount) + 1;
                }
                else {
                    nextWordIndex++;
                }
            }
            else {
                if(_atagger->isStatementEndPSet.contains(wordIndex)) {
                    nextWordIndex = (_atagger->wordCount) + 1;
                }
                else {
                    nextWordIndex++;
                }
            }
            /** Done **/

            Match* temp = simulateNFA(nfa, nstates.at(j), nextWordIndex);
            if(temp != NULL) {
                /** Update Match **/
                KeyM* keyMatch = (KeyM*)temp;
                if(tokens.at(i)->tagtype == NULL) {
                    keyMatch->key = "NONE";
                    keyMatch->id = tokens.at(i)->id;
                }
                else {
                    keyMatch->key = tokens.at(i)->tagtype->name;
                    keyMatch->id = tokens.at(i)->id;
                }
                keyMatch->length = tokens.at(i)->length;
                keyMatch->pos = tokens.at(i)->pos;
                keyMatch->msf = nfa->stateTOmsfMap.value(state).first;
                keyMatch->word = text.mid(tokens.at(i)->pos,tokens.at(i)->length);
                temp = temp->parent;
                /** Done **/
                matches.append(temp);
                priorityTransitions.append(state + nstates.at(j));
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
                        KeyM* keym = new KeyM(temp,"",-1,-1,-1);
                        keym->msf = msf;
                        temp->setMatch(keym);
                        temp = keym;
                    }
                    else if(msf->isUnary()) {
                        UNARYF* unary = (UNARYF*)msf;
                        UnaryM* unarym = new UnaryM(unary->op,uniqueID,temp,unary->limit);
                        uniqueID++;
                        unarym->msf = msf;
                        temp->setMatch(unarym);
                        temp = unarym;
                    }
                    else if(msf->isBinary()) {
                        /// AND case is resolved down in code
                        BINARYF* binary = (BINARYF*)msf;
                        if(binary->op == OR) {
                            BinaryM* binarym = new BinaryM(OR,temp,uniqueID);
                            uniqueID++;
                            binarym->msf = msf;
                            temp->setMatch(binarym);
                            temp = binarym;
                        }
                    }
                    else if(msf->isSequential()) {
                        SequentialM* seqm = new SequentialM(temp,uniqueID);
                        uniqueID++;
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
            priorityTransitions.append(state + nstates.at(j));
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
                    priorityTransitions.append(state + nstates.at(j));
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
                BinaryM* binarym = new BinaryM(AND,parent,uniqueID);
                uniqueID++;
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
        QVector<int> maxIndexes;
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
                maxIndexes.clear();
                maxIndexes.append(i);
                maxCount = tempMatchCount;
            }
            else if(tempMatchCount == maxCount) {
                maxIndexes.append(i);
            }
        }
        int matchIndex = maxIndexes.at(0);

        if (maxIndexes.count() != 1) {
            for(int i=0; i< matches.count(); i++) {
                if(nfa->prioritySet.contains(priorityTransitions.at(i))) {
                    matchIndex = i;
                    break;
                }
            }
        }

        /// Delete smaller matches
        for(int i=0; i< matches.count(); i++) {
            if(i != matchIndex) {
                Match* temp = matches[i];
                while(temp->parent != NULL) {
                    temp = temp->parent;
                }
                delete temp;
            }
        }
        return matches.at(matchIndex);
    }
    return NULL;
}

QByteArray ATagger::dataInJsonFormat(Data _data, QVector<QMultiHash<int,Tag*>* >* filesHash, QVector<QString>* textFiles) {
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
        if(textFiles == NULL) {
            tagdata.insert("file",_atagger->textFile);
            tagdata.insert("batch", false);
        }
        else {
            QVariantList tFiles;
            for(int i=0; i< textFiles->count(); i++) {
                tFiles.append(textFiles->at(i));
            }
            tagdata.insert("file", tFiles);
            tagdata.insert("batch", true);
        }
        tagdata.insert("TagTypeFile",_atagger->tagtypeFile);
        tagdata.insert("textchecksum", _atagger->text.count());
        tagdata.insert("version", 1.1);
        QVariantList tagset;
        if(filesHash == NULL) {
            QHashIterator<int, Tag*> iTag(*(_atagger->tagHash));
            while (iTag.hasNext()) {
                iTag.next();
                QVariantMap data;
                data.insert("type",iTag.value()->tagtype->name);
                data.insert("pos",iTag.value()->pos);
                data.insert("length",iTag.value()->length);
                data.insert("wordIndex",iTag.value()->wordIndex);
                data.insert("source",iTag.value()->source);
                data.insert("id",iTag.value()->id);
                data.insert("sText", iTag.value()->sourceText);
                tagset << data;
            }
        }
        else {
            for(int i=0; i<filesHash->count(); i++) {
                QHashIterator<int, Tag*> iTag(*(filesHash->at(i)));
                while (iTag.hasNext()) {
                    iTag.next();
                    QVariantMap data;
                    data.insert("type",iTag.value()->tagtype->name);
                    data.insert("pos",iTag.value()->pos);
                    data.insert("length",iTag.value()->length);
                    data.insert("wordIndex",iTag.value()->wordIndex);
                    data.insert("source",iTag.value()->source);
                    data.insert("id",iTag.value()->id);
                    data.insert("sText", iTag.value()->sourceText);
                    tagset << data;
                }
            }
        }
        tagdata.insert("TagArray",tagset);

        QHash<Match*,int> entityHash;
        QVariantList entityList;
        QVariantList relationList;

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
                data.insert("sText",mtag->sourceText);
                if(mtag->match != NULL) {
                    data.insert("match",mtag->match->getJSON());
                }

                for(int j=0; j<mtag->relationMatchVector.count(); j++) {
                    RelationM* rel = mtag->relationMatchVector[j];
                    QVariantMap entity1Data;
                    QVariantMap entity2Data;
                    QVariantMap edgeData;
                    QVariantMap relationData;

                    int e1ID = -1;
                    if(!entityHash.contains(rel->entity1)) {
                        entity1Data.insert("id",uniqueID);
                        e1ID = uniqueID;
                        uniqueID++;
                        entity1Data.insert("text",rel->e1Label);
                        entity1Data.insert("pos",rel->entity1->getPOS());
                        entityHash.insert(rel->entity1,e1ID);
                        entityList.append(entity1Data);
                    }
                    else {
                        e1ID = entityHash.value(rel->entity1);
                    }

                    int e2ID = -1;
                    if(!entityHash.contains(rel->entity2)) {
                        entity2Data.insert("id",uniqueID);
                        e2ID = uniqueID;
                        uniqueID++;
                        entity2Data.insert("text",rel->e2Label);
                        entity2Data.insert("pos",rel->entity2->getPOS());
                        entityHash.insert(rel->entity2,e2ID);
                        entityList.append(entity2Data);
                    }
                    else {
                        e2ID = entityHash.value(rel->entity2);
                    }

                    int edgeID = -1;
                    if(rel->edge != NULL) {
                        if(!entityHash.contains(rel->edge)) {
                            edgeData.insert("id",uniqueID);
                            edgeID = uniqueID;
                            uniqueID++;
                            edgeData.insert("text",rel->edgeLabel);
                            edgeData.insert("pos",rel->edge->getPOS());
                            entityHash.insert(rel->edge,edgeID);
                            entityList.append(edgeData);
                        }
                        else {
                            edgeID = entityHash.value(rel->edge);
                        }
                    }

                    relationData.insert("id", uniqueID);
                    uniqueID++;
                    relationData.insert("e1ID", e1ID);
                    relationData.insert("e2ID", e2ID);
                    if(edgeID != -1) {
                        relationData.insert("edgeID", edgeID);
                    }
                    else {
                        relationData.insert("edge", rel->edgeLabel);
                    }
                    relationList.append(relationData);
                }

                simulationList << data;
            }
            tagdata.insert("simulationTags",simulationList);
            tagdata.insert("entities", entityList);
            tagdata.insert("relations", relationList);

            /** Add the cross reference tags **/

            if(!(crossRelationVector.isEmpty())) {
                QVariantList crossRelList;
                for(int i=0; i<crossRelationVector.count(); i++) {
                    RelationM* relM = crossRelationVector[i];
                    QVariantMap crossRel;
                    crossRel.insert("id",uniqueID);
                    uniqueID++;
                    crossRel.insert("e1ID",relM->entity1->id);
                    crossRel.insert("e2ID",relM->entity2->id);
                    crossRelList.append(crossRel);
                }
                tagdata.insert("crossRels",crossRelList);
            }
        }

        QJson::Serializer serializer;
        json = serializer.serialize(tagdata);
    }

    return json;
}

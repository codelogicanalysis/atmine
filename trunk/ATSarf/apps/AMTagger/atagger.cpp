#include "atagger.h"
#include <QVariant>
#include <qjson/serializer.h>

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

bool ATagger::runSimulator() {

    /// Build NFA
    if(!buildNFA()) {
        return false;
    }

    /// Simulate NFAs referring to all the MSFs built
    simulationVector.clear();
    for(int i=0; i<nfaVector->count(); i++) {
        int index = 0;
        /// Simulate current MSF starting from a set of tokens referring to a single word
        for(int j=index; j<tagVector.count(); j++) {
            QVector<Tag*>* tags = simulateNFA(nfaVector->at(i), nfaVector->at(i)->start, index);
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
            }
            else {
                while((j<tagVector.count()) && (tagVector.at(index).pos == tagVector.at(j).pos)) {
                    j++;
                }
                index = j;
            }
        }
    }

    return true;
}

QVector<Tag*>* ATagger::simulateNFA(NFA* nfa, QString state, int tagIndex) {
    if((state == nfa->accept) || (nfa->andAccept.contains(state))) {
        QVector<Tag*> *tags = new QVector<Tag*>();
        return tags;
    }
    /// Use a vector of vectors to collect correct solutions and choose longest
    QVector<QVector<Tag*>*> tags;

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
            QVector<Tag*>* temp = simulateNFA(nfa, nstates.at(j), nextIndex);
            if(temp != NULL) {
                Tag* t = new Tag(tagVector.at(tokens.at(i)).type, tagVector.at(tokens.at(i)).pos, tagVector.at(tokens.at(i)).length, sarf);
                temp->prepend(t);
                tags.append(temp);
            }
        }
    }

    QList<QString> nstates =nfa->transitions.values(state + '|' + "epsilon");
    for(int j = 0; j < nstates.size(); j++) {
        QVector<Tag*>* temp = simulateNFA(nfa, nstates.at(j), tagIndex);
        if(temp != NULL) {
            tags.append(temp);
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
                    QVector<Tag*>* temp = simulateNFA(nfa, nstates.at(j), index);
                    if(temp != NULL) {
                        tags.append(temp);
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
            }
        }
        return tags.at(maxIndex);
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

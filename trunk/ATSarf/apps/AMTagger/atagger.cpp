#include "atagger.h"
#include <QVariant>
#include <qjson/serializer.h>

ATagger * _atagger = NULL;

ATagger::ATagger() {

    //tagVector = new QVector<Tag>();
    //compareToTagVector = new QVector<Tag>();
    tagTypeVector = new QVector<TagType*>();
    compareToTagTypeVector = new QVector<TagType*>();
}

ATagger::~ATagger() {

    //delete tagVector;
    tagVector.clear();

    for(int i=0; i<tagTypeVector->size(); i++) {
        delete (*tagTypeVector)[i];
    }
    delete tagTypeVector;
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
        //if(_atagger->isSarf) {
            //tagdata.insert("TagTypeFile",_atagger->sarftagtypeFile);
        //}
        //else {
        tagdata.insert("TagTypeFile",_atagger->tagtypeFile);
        //}
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

    return json;
}

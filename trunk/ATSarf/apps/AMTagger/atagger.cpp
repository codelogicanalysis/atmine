#include "atagger.h"
#include <QVariant>
#include <qjson/serializer.h>

ATagger * _atagger = NULL;

ATagger::ATagger() {

    tagVector = new QVector<Tag>();
    sarfTagVector = new QVector<Tag>();
    tagTypeVector = new QVector<TagType>();
    sarfTagTypeVector = new QVector<SarfTagType>();
}

bool ATagger::insertTag(QString type, int pos, int length, Source source) {

    Tag tag(type,pos,length,source);
    tagVector->append(tag);
    return true;
}

bool ATagger::insertSarfTag(QString type, int pos, int length, Source source) {

    Tag tag(type,pos,length,source);
    sarfTagVector->append(tag);
    return true;
}

bool ATagger::insertTagType(QString tag, QString desc, int id, QString fgcolor, QString bgcolor, int font, bool underline, bool bold, bool italic) {

    TagType tagtype(tag,desc,id,fgcolor,bgcolor,font,underline,bold,italic);
    tagTypeVector->append(tagtype);
    return true;
}

bool ATagger::insertSarfTagType(QString tag, QVector<QPair<QString, QString> > tags, QString desc, int id, QString fgcolor, QString bgcolor, int font, bool underline, bool bold, bool italic) {
    SarfTagType sarftagtype(tag,tags,desc,id,fgcolor,bgcolor,font,underline,bold,italic);
    sarfTagTypeVector->append(sarftagtype);
    return true;
}

QByteArray ATagger::dataInJsonFormat(Data _data) {
    QByteArray json;

    if(_data == sarfTTV) {
        QVariantMap sarftagtypedata;
        QVariantList sarftagtypeset;
        for( int i=0; i < _atagger->sarfTagTypeVector->count(); i++) {
            QVariantMap data;
            data.insert("Tag",(_atagger->sarfTagTypeVector->at(i)).tag);
            data.insert("Description",(_atagger->sarfTagTypeVector->at(i)).description);
            data.insert("id",_atagger->sarfTagTypeVector->count());
            data.insert("foreground_color",(_atagger->sarfTagTypeVector->at(i)).fgcolor);
            data.insert("background_color",(_atagger->sarfTagTypeVector->at(i)).bgcolor);
            data.insert("font",(_atagger->sarfTagTypeVector->at(i)).font);
            data.insert("underline",(_atagger->sarfTagTypeVector->at(i)).underline);
            data.insert("bold",(_atagger->sarfTagTypeVector->at(i)).bold);
            data.insert("italic",(_atagger->sarfTagTypeVector->at(i)).italic);

            QVariantMap tags;
            for(int j=0; j < (_atagger->sarfTagTypeVector->at(i).tags.count()); j++) {
                const QPair< QString, QString> * pair = &(_atagger->sarfTagTypeVector->at(i).tags.at(j));
                tags.insert(pair->first,pair->second);
            }
            data.insert("Tags",tags);

            sarftagtypeset << data;
        }
        sarftagtypedata.insert("TagSet",sarftagtypeset);
        QJson::Serializer serializer;
        json = serializer.serialize(sarftagtypedata);
    }
    else if(_data == tagTV) {
        /** Convert TagType to QJSON **/

        QVariantMap tagtypedata;
        QVariantList tagtypeset;
        for(int i=0; i < _atagger->tagTypeVector->count(); i++) {
            QVariantMap data;
            data.insert("Tag",(_atagger->tagTypeVector->at(i)).tag);
            data.insert("Description",(_atagger->tagTypeVector->at(i)).description);
            data.insert("id",_atagger->tagTypeVector->count());
            data.insert("foreground_color",(_atagger->tagTypeVector->at(i)).fgcolor);
            data.insert("background_color",(_atagger->tagTypeVector->at(i)).bgcolor);
            data.insert("font",(_atagger->tagTypeVector->at(i)).font);
            data.insert("underline",(_atagger->tagTypeVector->at(i)).underline);
            data.insert("bold",(_atagger->tagTypeVector->at(i)).bold);
            data.insert("italic",(_atagger->tagTypeVector->at(i)).italic);

            tagtypeset << data;
        }
        tagtypedata.insert("TagSet",tagtypeset);
        QJson::Serializer serializer;
        json = serializer.serialize(tagtypedata);
    }
    else if(_data == tagV) {

        /** Convert Tag to JSON **/

        QVariantMap tagdata;
        tagdata.insert("file",_atagger->textFile);
        tagdata.insert("TagTypeFile",_atagger->tagtypeFile);
        QVariantList tagset;
        for(int i=0; i<_atagger->tagVector->count(); i++) {
            QVariantMap data;
            data.insert("type",(_atagger->tagVector->at(i)).type);
            data.insert("pos",(_atagger->tagVector->at(i)).pos);
            data.insert("length",(_atagger->tagVector->at(i)).length);
            data.insert("source",(_atagger->tagVector->at(i)).source);
            tagset << data;
        }
        tagdata.insert("TagArray",tagset);
        QJson::Serializer serializer;
        json = serializer.serialize(tagdata);
    }
    else if(_data == sarfTV) {

        /** Convert Tag to JSON **/

        QVariantMap sarftagdata;
        sarftagdata.insert("file",_atagger->textFile);
        sarftagdata.insert("TagTypeFile",_atagger->sarftagtypeFile);
        QVariantList sarftagset;
        for(int i=0; i<_atagger->sarfTagVector->count(); i++) {
            QVariantMap data;
            data.insert("type",(_atagger->sarfTagVector->at(i)).type);
            data.insert("pos",(_atagger->sarfTagVector->at(i)).pos);
            data.insert("length",(_atagger->sarfTagVector->at(i)).length);
            data.insert("source",(_atagger->sarfTagVector->at(i)).source);
            sarftagset << data;
        }
        sarftagdata.insert("TagArray",sarftagset);
        QJson::Serializer serializer;
        json = serializer.serialize(sarftagdata);
    }

    return json;
}

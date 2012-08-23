#include "atagger.h"
#include <QVariant>
#include <qjson/serializer.h>

ATagger * _atagger = NULL;

ATagger::ATagger() {

    tagVector = new QVector<Tag>();
    tagTypeVector = new QVector<TagType>();
}

bool ATagger::insertTag(QString type, int pos, int length, Source source) {

    Tag tag(type,pos,length,source);
    tagVector->append(tag);
    return true;
}

bool ATagger::insertTagType(QString tag, QString desc, int id, QString fgcolor, QString bgcolor, int font, bool underline, bool bold, bool italic) {

    TagType tagtype(tag,desc,id,fgcolor,bgcolor,font,underline,bold,italic);
    tagTypeVector->append(tagtype);
    return true;
}

QByteArray ATagger::dataInJsonFormat(Data _data) {
    QByteArray json;

    if(_data == tagTV) {
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
    else {

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
    return json;
}

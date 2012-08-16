#include "atagger.h"

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



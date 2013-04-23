#include "sarftagtype.h"

SarfTagType::SarfTagType()
    : TagType() {
}

SarfTagType::SarfTagType(QString tag, QVector< Quadruple< QString , QString , QString , QString > > tags, QString desc, int id, QString fgcolor, QString bgcolor, int font, bool underline, bool bold, bool italic, Source source)
    :TagType(tag, desc, id, fgcolor, bgcolor, font, underline, bold, italic,source) {

    for(int i=0; i< tags.count(); i++) {
        this->tags.append(tags[i]);
    }
}

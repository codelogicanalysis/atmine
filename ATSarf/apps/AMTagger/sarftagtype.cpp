#include "sarftagtype.h"

SarfTagType::SarfTagType()
    : TagType() {
}

SarfTagType::SarfTagType(QString name, QVector< Quadruple< QString , QString , QString , QString > > tags, QString desc, QString fgcolor, QString bgcolor, int font, bool underline, bool bold, bool italic, Source source)
    :TagType(name, desc, fgcolor, bgcolor, font, underline, bold, italic,source) {

    for(int i=0; i< tags.count(); i++) {
        this->tags.append(tags[i]);
    }
}

#include "sarftagtype.h"

SarfTagType::SarfTagType()
{
}

SarfTagType::SarfTagType(QString tag, QVector<QPair<QString, QString> > tags, QString desc, int id, QString fgcolor, QString bgcolor, int font, bool underline, bool bold, bool italic) {

    this->tag = tag;
    for(int i=0; i< tags.count(); i++) {
        this->tags.append(tags[i]);
    }
    this->description = desc;
    this->id = id;
    this->fgcolor = fgcolor;
    this->bgcolor = bgcolor;
    this->font = font;
    this->underline = underline;
    this->bold = bold;
    this->italic = italic;
}

#include "tagtype.h"

TagType::TagType() {

}

TagType::TagType(QString tag, QString description, int id, QString fgcolor, QString bgcolor, int font, bool underline, bool bold, bool italic)
{
    this->tag = tag;
    this->description = description;
    this->id = id;
    this->fgcolor = fgcolor;
    this->bgcolor = bgcolor;
    this->font = font;
    this->underline = underline;
    this->bold = bold;
    this->italic = italic;
}

#include "tagtype.h"

TagType::TagType() {

}

TagType::TagType(QString name, QString description, QString fgcolor, QString bgcolor, int font, bool underline, bool bold, bool italic, Source source)
{
    this->name = name;
    this->description = description;
    this->fgcolor = fgcolor;
    this->bgcolor = bgcolor;
    this->font = font;
    this->underline = underline;
    this->bold = bold;
    this->italic = italic;
    this->source = source;
}

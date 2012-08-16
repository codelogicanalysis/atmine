#include "tag.h"

Tag::Tag() {

}

Tag::Tag(QString type, int pos, int length, Source source) {
    this->type = type;
    this->pos = pos;
    this->length = length;
    this->source = source;
}

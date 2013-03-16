#include "tag.h"

Tag::Tag() {

}

Tag::Tag(QString type, int pos, int length, Source source) {
    this->type = type;
    this->pos = pos;
    this->length = length;
    this->source = source;
}

bool Tag::operator ==(const Tag& tag) const {
    if((type == tag.type) && (pos == tag.pos) && (length == tag.length) && (source == tag.source)) {
        return true;
    }
    else {
        return false;
    }
}

#include "tag.h"

Tag::Tag() {
    sourceText.clear();
}

Tag::Tag(const TagType* tagtype, int pos, int length, int wordIndex, Source source, int id, QString sourceText) {
    this->tagtype = tagtype;
    this->pos = pos;
    this->length = length;
    this->wordIndex = wordIndex;
    this->source = source;
    this->id = id;
    this->sourceText = sourceText;
}

bool Tag::operator ==(const Tag& tag) const {
    if((tagtype->name == tag.tagtype->name) && (pos == tag.pos) && (length == tag.length) && (source == tag.source)) {
        return true;
    }
    else {
        return false;
    }
}

#include "merftag.h"

MERFTag::MERFTag() : Tag() {
}

MERFTag::MERFTag(QString name, int pos, int length) : Tag(name, pos, length, sarf) {
}

MERFTag::~MERFTag() {
    /*
    if(tags == NULL) {
        return;
    }
    for(int i=0; i<tags->count(); i++) {
        delete tags->at(i);
    }
    tags->clear();
    tags = NULL;
    */
}

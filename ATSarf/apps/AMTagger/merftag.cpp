#include "merftag.h"

MERFTag::MERFTag() : Tag() {
}

MERFTag::MERFTag(QString name, int pos, int length) : Tag(name, pos, length, sarf) {
}

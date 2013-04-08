#include "getGloss.h"

QStringList getGloss(QString desc) {
    if(desc.contains("from/of",Qt::CaseInsensitive)) {
        return QStringList(desc);
    }
    else if(desc.contains("of/from",Qt::CaseInsensitive)) {
        return QStringList(desc);
    }
    else if(desc.contains("from/about",Qt::CaseInsensitive)) {
        if(desc == "from/about") {
            return desc.split('/',QString::SkipEmptyParts);
        }
        else {
            return QStringList(desc);
        }
    }
    else if(desc.contains("city/town",Qt::CaseInsensitive)) {
        return QStringList(desc);
    }
    else if(desc.contains("+",Qt::CaseInsensitive)) {
        return QStringList(desc);
    }

    return desc.split('/', QString::SkipEmptyParts);
}

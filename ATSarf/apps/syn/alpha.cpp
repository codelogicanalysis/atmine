#include "alpha.h"
#include <QStringList>

Alpha::Alpha(QString *text) : Stemmer(text,0) {

    this->text = *text;
    theSarf->out<<"\nGlosses of stem "<< this->text<<":\n";
};


bool Alpha::on_match() {

    minimal_item_info & stem = *stem_info;
    QStringList gloss = stem.description().split('/', QString::SkipEmptyParts);

    for(int i=0; i<gloss.count(); i++) {

        bool Present = wGlosses.contains(gloss[i], Qt::CaseInsensitive);

        if(!Present)
        {
            wGlosses << gloss[i];
            theSarf->out<<gloss[i]<<'\n';
        }
    }

    return true;
};

QStringList* Alpha::getGlosses() {
    return & wGlosses;
};

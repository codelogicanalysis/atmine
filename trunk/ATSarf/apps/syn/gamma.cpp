#include "gamma.h"

Gamma::Gamma(QString *text):Stemmer(text,0) {
    this->text = *text;
    //theSarf->out<<"\nStems of the word "<<text<<":\n";
};

bool Gamma::on_match() {

    minimal_item_info & stem = *stem_info;
    QString match_Stem = stem.raw_data;

    bool Present = wStems.contains(match_Stem, Qt::CaseInsensitive);
    if(!Present) {
        wStems << match_Stem;
        //theSarf->out<<match_Stem<<'\n';
    }

    return true;
};

QStringList * Gamma::getStems() {
    return &wStems;
};

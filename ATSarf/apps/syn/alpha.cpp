#include <QStringList>
#include <QVariant>
#include "alpha.h"
#include "logger.h"
/*
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
*/

QStringList getGlosses(QString *text) {

    //theSarf->out<<"\nGlosses of the word "<<*text<<":\n";

    QStringList wGlosses;

    theSarf->query.exec("select d.name from stem_category s, description d where s.description_id=d.id and raw_data=\"" + *text + '"');
    while(theSarf->query.next()) {

        if(!(theSarf->query.value(0).toString().isEmpty())) {
            QString glosses = theSarf->query.value(0).toString();
            QStringList gList = glosses.split('/',QString::SkipEmptyParts);

            for(int i=0; i<gList.size(); i++) {
                wGlosses << gList[i];
                //theSarf->out<<gList[i]<<'\n';
            }
        }
    }
    return wGlosses;
};

#include <QStringList>
#include <QVariant>
#include <QVector>
#include "alpha.h"
#include "logger.h"

QVector<DescIdGloss> getGlosses(QString *text) {

    QVector<DescIdGloss> wGlosses;

    theSarf->query.exec("select d.name, d.id from stem_category s, description d where s.description_id=d.id and raw_data=\"" + *text + '"');
    while(theSarf->query.next()) {

        if(!(theSarf->query.value(0).toString().isEmpty())) {
            QString glosses = theSarf->query.value(0).toString();
            QString id = theSarf->query.value(1).toString();
            QStringList gList = glosses.split('/',QString::SkipEmptyParts);

            for(int i=0; i<gList.size(); i++) {
                DescIdGloss dig(gList[i],id);
                wGlosses.push_back(dig);
            }
        }
    }
    return wGlosses;
};

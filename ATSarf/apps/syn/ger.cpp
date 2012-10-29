#include "ger.h"
#include <QStringList>

GER::GER(QString word, int order) {

    this->word = word;
    this->order = order;
};

bool GER::operator ()() {

    /** Get all stems of the input word **/
    Gamma gamma(&word);
    gamma();

    QStringList * stems = gamma.getStems();

    for(int i=0; i< stems->count(); i++) {
        this->wStem << stems->at(i);
        //theSarf->out<<stems->at(i)<<'\n';
    }
    /** Get all glosses and dictionary entries for "word" stems **/

    for(int i=0; i<stems->count(); i++) {

        QString stem = (*stems)[i];
        QStringList glosses = getGlosses(&stem);


        for(int j=0; j<glosses.count(); j++) {

            if(!(wGloss.contains(glosses[j]))) {
                wGloss << glosses[j];
            }
        }
    }
    /** Find Synonymity of entered order
      * If entered order us infinity, get all possible matches for undefined order
      **/

    /// This boolean is used to catch the order after which the set of related words doesn't grow
    bool stop = false;

    /// This variable counts the number of iterations referring to the synonimity order
    int iteration = 0;

    while(!stop && (iteration != order)) {

        stop = true;
        QStringList gloss_id;

        QString desc_query = "SELECT id, name FROM description where ";
        for(int i=0; i< wGloss.size(); i++) {
            desc_query.append("name like '%/" + wGloss[i] + "/%' OR name like '" + wGloss[i] + "/%' OR name like '%/" + wGloss[i] + "' OR ");
        }
        desc_query = desc_query.mid(0,desc_query.length()-4);
        theSarf->query.exec(desc_query);

        while(theSarf->query.next()) {

            if(!(theSarf->query.value(0).toString().isEmpty())) {
                gloss_id << theSarf->query.value(0).toString();
            }

            if(!(theSarf->query.value(1).toString().isEmpty())) {
                QStringList desc_entry = theSarf->query.value(1).toString().split('/');

                for(int i=0; i<desc_entry.size(); i++) {
                    if(!(wGloss.contains(desc_entry[i]))) {
                        wGloss << desc_entry[i];
                    }
                }
            }
        }

        QString stem_query = "SELECT raw_data from stem_category where";
        for(int i=0; i<gloss_id.size(); i++) {
            stem_query.append(" description_id=" + gloss_id[i] + " or ");
        }
        stem_query = stem_query.mid(0,stem_query.length()-4);
        theSarf->query.exec(stem_query);

        while(theSarf->query.next()) {

            if(!(theSarf->query.value(0).toString().isEmpty())) {
                QString stem = theSarf->query.value(0).toString();
                if(!(wStem.contains(stem))) {
                    wStem << stem;
                    stop = false;
                }
            }
        }

        iteration++;
    }

    theSarf->out<<"\nThe list of related words for order "<<order<<" are:\n";
    for(int i=0; i<wStem.count(); i++) {
        theSarf->out<<wStem[i]<<'\n';
    }
    return true;
};

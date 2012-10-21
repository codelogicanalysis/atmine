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

    QStringList listStems;

    theSarf->query.exec("SELECT raw_data FROM stem_category");
    while(theSarf->query.next()) {
        if(!(theSarf->query.value(0).toString().isEmpty())) {
            listStems << theSarf->query.value(0).toString();
        }
    }

    while(!stop && (iteration != order)) {

        stop = true;
        for(int i=0; i< listStems.count(); i++) {

            if(wStem.contains(listStems[i])) {
                continue;
            }

            //Alpha alpha(&(listStems[i]));
            //alpha();

            //QStringList* tempGloss = alpha.getGlosses();
            QStringList tempGloss = getGlosses(&(listStems[i]));

            bool include = false;
            for(int j=0; j<tempGloss.count(); j++) {

                if(wGloss.contains(tempGloss[j])) {
                    wStem << listStems[i];
                    theSarf->out<< listStems[i]<<'\n';
                    include = true;
                    stop = false;
                    break;
                }
            }

            if(include) {
                for(int j=0; j<tempGloss.count(); j++) {

                    if(!(wGloss.contains(tempGloss[j]))) {
                        wGloss << tempGloss[j];
                    }
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

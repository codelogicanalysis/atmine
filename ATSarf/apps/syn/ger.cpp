#include "ger.h"
#include <QStringList>
#include <QHash>

GER::GER(QString word, int order) {

    this->word = word;
    this->order = order;
};

bool GER::operator ()() {

    /** Build the Desciption hash map **/

    QHash< QString, QString> descMap;
    QString query = "SELECT id,name from description";
    theSarf->query.exec(query);
    while(theSarf->query.next()) {
        if(!(theSarf->query.value(0).toString().isEmpty() || theSarf->query.value(1).toString().isEmpty())) {
            QStringList tempG = theSarf->query.value(1).toString().split('/', QString::SkipEmptyParts);
            QString tempId = theSarf->query.value(0).toString();
            for(int i=0; i<tempG.count(); i++) {
                if((descMap.find(tempG[i])) == descMap.end()) {
                    descMap[tempG[i]] = tempId;
                }
                else {
                    descMap[tempG[i]] = descMap[tempG[i]] + '/' + tempId;
                }
            }
        }
    }

    /** Build the desc_id -> stem hash map **/

    QHash< QString, QString> stemMap;
    query= "SELECT description_id,raw_data from stem_category";
    theSarf->query.exec(query);
    while(theSarf->query.next()) {
        if(!(theSarf->query.value(0).toString().isEmpty() || theSarf->query.value(1).toString().isEmpty())) {
            QString tempS = theSarf->query.value(1).toString();
            QString tempId = theSarf->query.value(0).toString();
            if((stemMap.find(tempId)) == stemMap.end()) {
                stemMap[tempId] = tempS;
            }
            else {
                stemMap[tempId] = stemMap[tempId] + '/' + tempS;
            }
        }
    }

    /** Get all stems of the input word **/
    Gamma gamma(&word);
    gamma();

    QStringList * stems = gamma.getStems();

    for(int i=0; i< stems->count(); i++) {
        this->wStem[stems->at(i)] = stems->at(i);
        //theSarf->out<<stems->at(i)<<'\n';
    }
    /** Get all glosses and dictionary entries for "word" stems **/
    QHash<QString, QString> wGloss;

    for(int i=0; i<stems->count(); i++) {

        QString stem = (*stems)[i];
        QStringList glosses = getGlosses(&stem);


        for(int j=0; j<glosses.count(); j++) {

            if(!(wGloss.contains(glosses[j]))) {
                wGloss[glosses[j]] = glosses[j];
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

    /// List saving the new glosses extracted for each iteration
    QStringList newG;

    QHashIterator<QString, QString> gIterator(wGloss);
     while (gIterator.hasNext()) {
         gIterator.next();
         newG << gIterator.key();
     }

    while(!stop && (iteration != order)) {

        stop = true;
        QStringList gloss_id;
        QHash<QString, QString> idHash;
        QStringList tempG;

        /*
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
        */

        for(int i=0; i<newG.count(); i++) {
            QStringList tempIds = (descMap[newG[i]]).split('/',QString::SkipEmptyParts);
            for(int j=0; j<tempIds.count(); j++) {
                if(!(gloss_id.contains(tempIds[j]))) {
                    gloss_id << tempIds[j];
                    idHash[tempIds[j]] = newG[i];
                }

                theSarf->query.exec("SELECT name from description where id=" + tempIds[j]);
                if(theSarf->query.first() && !(theSarf->query.value(0).toString().isEmpty())) {
                    QStringList glosses = theSarf->query.value(0).toString().split('/',QString::SkipEmptyParts);
                    for(int k=0; k<glosses.count(); k++) {
                        if(!(wGloss.contains(glosses[k]))) {
                            tempG << glosses[k];
                            wGloss[glosses[k]] = glosses[k];
                        }
                    }
                }
            }
        }

        newG.clear();
        for(int i=0; i<tempG.count(); i++) {
            newG << tempG[i];
        }

        /*
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
        */

        for(int i=0; i< gloss_id.count(); i++) {

            QStringList tempStems = stemMap[gloss_id[i]].split('/',QString::SkipEmptyParts);
            for(int j=0; j<tempStems.count(); j++) {
                if(!(wStem.contains(tempStems[j]))) {
                    wStem[tempStems[j]] = tempStems[j];
                    QString id = gloss_id[i];
                    QString gloss = idHash[id];
                    IGS igs(id,gloss,tempStems[j]);
                    descT.push_back(igs);
                    stop = false;
                }
            }
        }

        iteration++;;
    }

    theSarf->out << "\nThe list of related words for order " << order << " at iteration " << iteration << " are:\n";
    theSarf->out << "Stem" << '\t' << "Source Gloss" << '\t' << "Source desc_id" << '\n';
    /*
    for(int i=0; i<wStem.count(); i++) {
        theSarf->out<<wStem[i]<<'\n';
    }
    */
    /*
    QHashIterator<QString, QString> stem_Iterator(wStem);
     while (stem_Iterator.hasNext()) {
         stem_Iterator.next();
         theSarf->out<< stem_Iterator.key()<<'\n';
     }
     */
    for(int i=0; i<descT.count(); i++) {
        theSarf->out<< descT[i].getStem() << '\t' << descT[i].getGloss() << '\t' << descT[i].getId() << '\n';
    }

    return true;
};

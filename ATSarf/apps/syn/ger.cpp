#include "ger.h"
#include <QStringList>
#include <QHash>

GER::GER(QString word, int order) {

    this->word = word;
    this->order = order;
};

class SDG {
public:
    QString stem;
    QString desc_id;
    QString gloss;
    SDG(){}
    SDG(QString s, QString d, QString g): stem(s),desc_id(d),gloss(g){}
};

bool GER::operator ()() {

    /** Build the Desciption hash map gloss -> desc_Ids **/

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

    /** Build the desc_id -> stem hash map desc_Id -> Stems **/

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
        this->wStem.insert(stems->at(i), stems->at(i));
    }
    /** Get all glosses and dictionary entries for "word" stems **/
    QHash<QString, QString> wGloss;

    /** This structure holds gloss -> source stem/desc_id/glosses */
    QHash<QString, SDG> sdgHash;

    for(int i=0; i<stems->count(); i++) {

        QString stem = (*stems)[i];
        QVector<DescIdGloss> glosses = getGlosses(&stem);


        for(int j=0; j<glosses.count(); j++) {

            if(!(wGloss.contains(glosses[j].gloss))) {
                wGloss.insert(glosses[j].gloss, stem);

                SDG sdg(stem,glosses[j].desc_Id, "BLANK");
                sdgHash.insert(glosses[j].gloss, sdg);
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

    /// Hash saving the new glosses along with their source stems
    QHash<QString, QString> newGS;

    QHashIterator<QString, QString> gIterator(wGloss);
     while (gIterator.hasNext()) {
         gIterator.next();
         newG << gIterator.key();
         newGS.insert(gIterator.key(),gIterator.value());
     }

    while(!stop && (iteration != order)) {

        stop = true;
        /// Store new Gloss Ids
        QStringList gloss_id;
        /// Store gloss along with source id for stem tracking next iteration
        QHash<QString, QString> gId;
        /// Store ids along with source gloss for current iteration
        QHash<QString, QString> idHash;
        /// Store new glosses for next iteration
        QStringList tempG;

        for(int i=0; i<newG.count(); i++) {

            QStringList tempIds = (descMap[newG[i]]).split('/',QString::SkipEmptyParts);
            for(int j=0; j<tempIds.count(); j++) {

                if(!(gloss_id.contains(tempIds[j]))) {
                    gloss_id << tempIds[j];
                    idHash.insert(tempIds[j], newG[i]);
                }
                else {
                    continue;
                }


                theSarf->query.exec("SELECT name from description where id=" + tempIds[j]);
                if(theSarf->query.first() && !(theSarf->query.value(0).toString().isEmpty())) {
                    QStringList glosses = theSarf->query.value(0).toString().split('/',QString::SkipEmptyParts);
                    for(int k=0; k<glosses.count(); k++) {
                        if(!(wGloss.contains(glosses[k]))) {
                            tempG << glosses[k];
                            wGloss.insert(glosses[k], glosses[k]);

                            /** This part is to save all the data regarding source of each new gloss **/
                            QString sStem = "BLANK";
                            SDG sdg(sStem,tempIds[j],newG[i]);
                            sdgHash.insert(glosses[k], sdg);
                            /** End of this part **/

                            /** Needed to track the source gloss of an id for source stem identification **/

                        }
                        if(gId.contains(tempIds[j])) {
                            gId[tempIds[j]] += ('/' + glosses[k]);
                        }
                        else {
                            gId.insert(tempIds[j], glosses[k]);
                        }
                    }
                }
            }
        }

        /** Temp storage for stems and source gloss **/
        QHash<QString, QString> tempGS;

        for(int i=0; i< gloss_id.count(); i++) {

            QStringList tempStems = stemMap.value(gloss_id[i]).split('/',QString::SkipEmptyParts);
            for(int j=0; j<tempStems.count(); j++) {

                QString id = gloss_id[i];
                if(!(wStem.contains(tempStems[j]))) {

                    wStem.insert(tempStems[j], tempStems[j]);
                    //QString id = gloss_id[i];
                    QString gloss = idHash.value(id);
                    QString sStem = newGS.value(gloss);
                    IGS igs(id,gloss,tempStems[j], sStem);
                    descT.push_back(igs);
                    stop = false;
                }

                /** desc_id -> descendant glosses -> source stems **/
                QStringList sGloss = gId.value(id).split('/', QString::SkipEmptyParts);
                for(int k=0; k<sGloss.count(); k++) {
                    if(!(newGS.contains(sGloss[k]))) {
                        tempGS.insert(sGloss[k], tempStems[j]);
                    }
                }
            }
        }

        newG.clear();
        for(int i=0; i<tempG.count(); i++) {
            newG << tempG[i];
        }

        /** Used to track stems and their source glosses **/
        newGS.clear();
        QHashIterator<QString, QString> gsIterator(tempGS);
        while (gsIterator.hasNext()) {
            gsIterator.next();
            newGS.insert(gsIterator.key(), gsIterator.value());
        }

        iteration++;;
    }

    theSarf->out << "\nThe list of related words for order " << order << " at iteration " << iteration << " are:\n";
    theSarf->out << "\"Stem\"" << '\t' << "\"Source desc_id\"" << '\t' << "\"Source Gloss\"" << '\t' << "\"Gloss Source desc_id\"" << '\t' << "\"Source Gloss of Gloss\"" << '\t' << "\"Source Stem\"" << '\n';

    for(int i=0; i<descT.count(); i++) {
        theSarf->out << descT[i].getStem() << '\t' << descT[i].getId() << '\t' << descT[i].getGloss() << '\t';
        theSarf->out << sdgHash[descT[i].getGloss()].desc_id << '\t';
        theSarf->out << sdgHash[descT[i].getGloss()].gloss << '\t';
        theSarf->out << descT[i].getsStem() << '\n';
    }

    return true;
};

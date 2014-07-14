#ifndef GER_H
#define GER_H

#include <QString>
#include "alpha.h"
#include "gamma.h"

#define infinity -1

class IGS {
private:
    QString sStem;
    QString sDesc_id;
    QString sGloss;
    QString cStem;

public:
    IGS() {
        sDesc_id = "";
        sGloss = "";
        cStem = "";
        sStem = "";
    }

    IGS(QString desc_id, QString gloss, QString stem, QString sstem) {
        this->sDesc_id = desc_id;
        this->sGloss = gloss;
        this->cStem = stem;
        this->sStem = sstem;
    }

    QString getId() const {
        return sDesc_id;
    }
    QString getGloss() const {
        return sGloss;
    }
    QString getStem() const {
        return cStem;
    }
    QString getsStem() const {
        return sStem;
    }
};

class GER
{
  public:
        GER(QString word, int input = 0 ,int order = infinity, bool print = false);
	bool operator()();
        /// List to store triplet desc_id, source gloss, source stem
        QVector<IGS> descT;
        /// Consider the key to be the source gloss
        //QHash<QString,IGS> descT;
        QSet<QString> wStem;
 private:
	QString word;
        int input;
        bool print;
	int order;
        //QStringList wStem;
};
#endif // GER_H

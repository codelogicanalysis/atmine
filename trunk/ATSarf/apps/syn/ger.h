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

    QString getId() {
        return sDesc_id;
    }
    QString getGloss() {
        return sGloss;
    }
    QString getStem() {
        return cStem;
    }
    QString getsStem() {
        return sStem;
    }
};

class GER
{
  public:
	GER(QString word, int order = infinity);
	bool operator()();

 private:
	QString word;
	int order;
        /// List to store triplet desc_id, source gloss, source stem
        QVector<IGS> descT;
        //QStringList wStem;
        QHash<QString, QString> wStem;
};
#endif // GER_H

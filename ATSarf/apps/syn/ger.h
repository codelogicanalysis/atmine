#ifndef GER_H
#define GER_H

#include <QString>
#include "alpha.h"
#include "gamma.h"

#define infinity -1

class IGS {
private:
    QString desc_id;
    QString gloss;
    QString stem;

public:
    IGS() {
        desc_id = "";
        gloss = "";
        stem = "";
    }

    IGS(QString desc_id, QString gloss, QString stem) {
        this->desc_id = desc_id;
        this->gloss = gloss;
        this->stem = stem;
    }

    QString getId() {
        return desc_id;
    }
    QString getGloss() {
        return gloss;
    }
    QString getStem() {
        return stem;
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
	QStringList wStem;
};
#endif // GER_H

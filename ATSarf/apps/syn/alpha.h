#ifndef ALPHA_H
#define ALPHA_H
#include <QStringList>

class DescIdGloss {
public:
    QString gloss;
    QString desc_Id;
    DescIdGloss(){}
    DescIdGloss(QString g, QString d):gloss(g), desc_Id(d) {}
};

QVector<DescIdGloss> getGlosses(QString * text);
#endif // ALPHA_H

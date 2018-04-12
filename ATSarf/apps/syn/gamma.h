#ifndef GAMMA_H
#define GAMMA_H
#include "stemmer.h"
#include <QStringList>

class Gamma : public Stemmer
{
private:
    QString text;
    QStringList wStems;

public:
    Gamma(QString * text);
    bool on_match();
    QStringList* getStems();
};

#endif // GAMMA_H

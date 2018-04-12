#ifndef GLOSSSFR_H
#define GLOSSSFR_H
#include "stemmer.h"
#include <QStringList>

class GlossSFR : public Stemmer
{
private:
    QString text;
    QString RelatedW;
    QStringList _LIST;

public:
    GlossSFR(QString * text);
    bool on_match();
};

#endif // GLOSSSFR_H

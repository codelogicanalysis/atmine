#ifndef ALPHA_H
#define ALPHA_H
//#include "stemmer.h"
#include <QStringList>

/*
class Alpha : public Stemmer
{
private:
    QString text;
    QStringList wGlosses;

public:
    Alpha(QString * text);
    bool on_match();
    QStringList * getGlosses();
};
*/

QStringList getGlosses(QString * text);
#endif // ALPHA_H

#ifndef AMFILLER_H
#define AMFILLER_H
/** This class (abreviation for Action Match Filler) gets the morphological features of a word **/
#include "stemmer.h"

class AMFiller : public Stemmer
{
public:
    AMFiller(QString text, QString* sarfMatches, QString mVName);
    bool on_match();
private:
    QString text;
    QString* sarfMatches;
    QString mVName;
    int count;
};

#endif // AMFILLER_H

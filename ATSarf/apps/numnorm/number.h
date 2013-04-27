#ifndef NUMBER_H
#define NUMBER_H
#include <stemmer.h>
#include <numnorm.h>

class NumNorm;

class Number : public Stemmer
{
private:
    bool isDigitsTens(QStringList& stem_glosses, int& val);
    bool isKey(QStringList& stem_glosses, int& val);
    bool isHundred(QStringList& stem_glosses, int& val);
    void digitsTensActions(int val);
    void keyActions(int val);
    void hundredActions(int val);
public:
    QString * word;
    QHash<QString, int>* hashGlossInt;
    NumNorm * controller;
    Number(NumNorm *, QString);
    bool on_match();
};

#endif // NUMBER_H

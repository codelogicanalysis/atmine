#ifndef DIACRITICRULES_H
#define DIACRITICRULES_H

#include <iostream>
#include <QSet>
#include "enumerator.h"

class WordDensity : public Enumerator
{
private:
    //QHash<QString, int> wordHash;
    QSet<QString> wordSet;
    int max_word_length;
    long long *number_of_uvWords;
    long long *number_of_vWords;

public:
    WordDensity(long max_word_length, enumeration_type type=ENUMALL, bool get_all_details=true);
    void getuvWordStats();
    void getvWordStats();
    bool on_match();
};

#endif // DIACRITICRULES_H

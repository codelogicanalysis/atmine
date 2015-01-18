#ifndef DIACRITICGUIDELINES_H
#define DIACRITICGUIDELINES_H

#include <iostream>
#include <QHash>
#include "enumerator.h"
//#include "vwtrie.h"

class DiacriticGuidelines : public Enumerator
{
private:
//    VWTrie* trie;
    QHash<QString,qint32> uvWords;
    long number_of_solutions;
    long solution_counter;

public:

    DiacriticGuidelines(long number_of_solutions, bool get_all_details=true);
    //bool saveTrie();
    bool serializeHash();
    bool on_match();
};

#endif // DIACRITICGUIDELINES_H

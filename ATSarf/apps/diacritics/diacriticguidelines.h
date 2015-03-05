#ifndef DIACRITICGUIDELINES_H
#define DIACRITICGUIDELINES_H

#include <iostream>
#include <QHash>
#include "enumerator.h"

class DiacriticGuidelines : public Enumerator
{
private:
//    VWTrie* trie;
    QHash<QString,int> uvWords;
    long number_of_solutions;
    long solution_counter;
    enumeration_type enum_type;

public:

    DiacriticGuidelines(long number_of_solutions, enumeration_type enum_type=ENUMALL, bool get_all_details=true);
    //bool saveTrie();
    bool serializeHash();
    bool on_match();
};

#endif // DIACRITICGUIDELINES_H

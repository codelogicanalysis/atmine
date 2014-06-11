#ifndef DIACRITICGUIDELINES_H
#define DIACRITICGUIDELINES_H

#include <iostream>
#include "enumerator.h"
#include "vwtrie.h"

class DiacriticGuidelines : public Enumerator
{
private:
    VWTrie* trie;
    long number_of_solutions;
    long solution_counter;
    long long index;

public:

    DiacriticGuidelines(long number_of_solutions, bool get_all_details=true);
    bool saveTrie();
    bool on_match();
};

#endif // DIACRITICGUIDELINES_H

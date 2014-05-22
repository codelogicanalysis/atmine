#ifndef DIACRITICRULES_H
#define DIACRITICRULES_H

#include <iostream>
#include "enumerator.h"

class DiacriticRules : public Enumerator
{
private:
    long number_of_solutions;
    long solution_counter;
    long filtered_items;

public:

    DiacriticRules(long number_of_solutions, bool get_all_details=true);
    long getFilteredItems() {return filtered_items;}
    bool on_match();
};

#endif // DIACRITICRULES_H

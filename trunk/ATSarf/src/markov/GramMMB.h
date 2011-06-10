#ifndef GRAMMMB_H
#define GRAMMMB_H



#include <map>


class GramMMB{



public:
    void removePrepsitions(map<string, double> & map){
        string prepostions[]= {"and", "or", "as", "a", "an", "are", "is", "the", "to", "with", "To", "in", "of", "you", "your", "such", "for", "from", "at", "be", "The", "it"};
        string numbers[] = {"111", "11", "1111", "232","200", "100", "500", "25", "11", "10", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "90", "45" };
        for(int i=0; i< 22; i++){
            map.erase(prepostions[i]);
            map.erase(numbers[i]);
        }
    }






};




#endif // GRAMMMB_H

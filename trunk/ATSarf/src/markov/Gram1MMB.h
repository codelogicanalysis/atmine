#ifndef GRAM1MMB_H
#define GRAM1MMB_H

#include "maxArray.h"
#include "GramMMB.h"
#include <map>

class Gram1MMB: public GramMMB{

 private:
    map<string, double> wordFrequency;
    MaxArray highFreqArray;


public:
    Gram1MMB(){
    }


    void printWordFrequency(){
        removePrepsitions(wordFrequency);
        map<string, double>::const_iterator iter;
        for (iter=wordFrequency.begin(); iter != wordFrequency.end(); ++iter){
//            cout << iter->second << " " << iter->first << endl;
            highFreqArray.insert(iter->first, iter->second);
        }
        highFreqArray.print();


    }
    void insert(string word){
        wordFrequency[word] ++;

    }




    MaxArray getMaxArray(){

        return highFreqArray;

    }
    void setArrayMaximum(int length){
        highFreqArray.setMaxNumber(length);

    }




};











#endif // GRAM1MMB_H

#ifndef GRAM3MMB_H
#define GRAM3MMB_H


#include "GramMMB.h"


class Gram3MMB: public GramMMB{
private:
    map<string, map<string, double> > gram3MMB;
    MaxArray highFreqArray;

public:
    Gram3MMB(){}

    void insert(string word, string secondWord){
        gram3MMB[word][secondWord] ++;

    }


    void remove(MaxArray oneGramArray){
        map<string, map<string, double> > copiedMap;
        for(int i=0; i<oneGramArray.getMaxNumber(); i++){
                copiedMap[oneGramArray.getWord(i)] = gram3MMB[oneGramArray.getWord(i)];
        }
        gram3MMB.clear();
        gram3MMB = copiedMap;

    }



    void printWordFrequency(MaxArray oneGramArray){
         remove(oneGramArray);
         map<string, map<string, double> >::const_iterator iter;
         for (iter=gram3MMB.begin(); iter != gram3MMB.end(); ++iter){
            cout<<"Word: "<<iter->first<<endl;
            map<string, double>::const_iterator iter2;
            map<string, double> secondMap = gram3MMB[iter->first];
            for (iter2=secondMap.begin(); iter2 != secondMap.end(); ++iter2){
                cout<<iter2->first<<" "<<iter2->second<<endl;
            }

         }
    }


    int getMapSize(string word){
        return gram3MMB[word].size();

    }


     map<string, map<string, double> >  getNormalizedMap(int fileCount){
        string word;
        map<string, double>::const_iterator iter2;
        map<string, map<string, double> >::const_iterator iter;
        for (iter=gram3MMB.begin(); iter != gram3MMB.end(); ++iter){
            word = iter->first;
            for (iter2=gram3MMB[word].begin(); iter2 != gram3MMB[word].end(); ++iter2){
                gram3MMB[word][iter2->first] /= fileCount;
            }

         }
        return gram3MMB;

    }



};





#endif // GRAM3MMB_H

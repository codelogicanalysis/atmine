#ifndef GRAM2MARKOVMODELBUILDER_H
#define GRAM2MARKOVMODELBUILDER_H

#include "maxArray.h"
#include "GramMMB.h"

class Gram2MMB: public GramMMB{

private:
    map<string, map<string, double> > gram2Map;
    MaxArray highFreqArray;


public:
    Gram2MMB(){}

    void insert(string word, string secondWord){
        gram2Map[word][secondWord] ++;

    }

    void remove(MaxArray oneGramArray){
        map<string, map<string, double> > copiedMap;
        for(int i=0; i<oneGramArray.getMaxNumber(); i++){
                copiedMap[oneGramArray.getWord(i)] = gram2Map[oneGramArray.getWord(i)];
        }
        gram2Map.clear();
        gram2Map = copiedMap;

    }


    void printWordFrequency(MaxArray oneGramArray){
         remove(oneGramArray);
         map<string, map<string, double> >::const_iterator iter;
         for (iter=gram2Map.begin(); iter != gram2Map.end(); ++iter){
            cout<<endl<<"Word: "<<iter->first<<endl;
            map<string, double>::const_iterator iter2;
            map<string, double> secondMap = gram2Map[iter->first];
            removePrepsitions(gram2Map[iter->first]);
            for (iter2= secondMap.begin(); iter2 != secondMap.end(); ++iter2){
                cout<<iter2->first<<" "<<iter2->second<<endl;
            }

         }
    }


    int getMapSize(string word){
        return gram2Map[word].size();

    }

    map<string, map<string, double> >  getNormalizedMap(int fileCount){
        cout<<"Printing normalized map."<<endl;
        map<string, map<string, double> >::const_iterator iter;
        map<string, double>::const_iterator iter2;
        string word;
        for (iter=gram2Map.begin(); iter != gram2Map.end(); ++iter){
            word = iter->first;
            cout<<endl<<"Word is: "<<word<<endl;
            for (iter2=gram2Map[word].begin(); iter2 != gram2Map[word].end(); ++iter2){
                gram2Map[word][iter2->first] /= fileCount;
                cout<<iter2->first<<" "<<iter2->second<<endl;
            }

         }


        return gram2Map;
    }



};

#endif // GRAM2MARKOVMODELBUILDER_H

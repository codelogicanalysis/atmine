#ifndef TEXTCOUNTER_H
#define TEXTCOUNTER_H

#include "maxArray.h"


#include <fstream>
#include <iostream>
#include <string>
#include <map>
using namespace std;



class TextCounter
{
private:
    map<string, int> wordFrequency;
    MaxArray highFreqArray;


public:
    TextCounter(){
        highFreqArray.setMaxNumber(10);
    }
    void getWords(string fileName){
        ifstream in;
        in.open(fileName.c_str());
        string word;
        in>>word;
        while(!in.eof()){
            if(word !="*" &&word!="-"&&word!="&")
                wordFrequency[removePunctuation(word)] ++;
            in>>word;
        }
    }
    string removePunctuation(string word){
        char punc = word[word.length()-1];
        if(word[0]=='(')
            word = word.substr(1, word.length());
        if(punc =='!' || punc=='.'|| punc==',' || punc=='?' ||punc==':' ||punc==';'||punc==')'||punc=='"'){
            string toReturn = word.substr(0, word.length()-1);
            return toReturn;
        }
        return word;



    }
    void printWordFrequency(){
        removePrepsitions();
        map<string, int>::const_iterator iter;
        for (iter=wordFrequency.begin(); iter != wordFrequency.end(); ++iter){
//            cout << iter->second << " " << iter->first << endl;
            highFreqArray.insert(iter->first, iter->second);
        }
        highFreqArray.print();

    }

    void removePrepsitions(){
        string prepostions[]= {"and", "or", "as", "a", "an", "are", "is", "the", "to", "with", "To", "in", "of", "you", "your", "such", "for", "from", "at", "be", "The", "it"};
        string numbers[] = {"111", "11", "1111", "232","200", "100", "500", "25", "11", "10", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "90", "45" };
        for(int i=0; i< 22; i++){
            wordFrequency.erase(prepostions[i]);
            wordFrequency.erase(numbers[i]);
        }
    }

    MaxArray getMaxArray(){
        return highFreqArray;
    }



};




#endif // TEXTCOUNTER_H

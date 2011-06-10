#ifndef MMB_H
#define MMB_H


#include "maxArray.h"
#include "Gram1MMB.h"
#include "Gram2MMB.h"
#include "Gram3MMB.h"


#include <fstream>
#include <iostream>
#include <string>
#include <map>
using namespace std;



class MMB
{
private:
    Gram1MMB gram1MMB;
    Gram2MMB gram2MMB;
    Gram3MMB gram3MMB;



public:
    MMB(){}

    MMB(int depth){
        gram1MMB.setArrayMaximum(depth);

    }

    void setDepth(int depth){
        gram1MMB.setArrayMaximum(depth);
    }


    void getWords(string fileName){
        ifstream in;
        in.open(fileName.c_str());
        string word;
        string prevWord= "";
        string prevWord2 = "";
        in>>word;
        while(!in.eof()){

            if(word !="*" &&word!="-"&&word!="&"){
                gram1MMB.insert(removePunctuation(word));
                gram2MMB.insert(prevWord, word);
                gram3MMB.insert(prevWord2,word);
                prevWord2 = prevWord;
                prevWord = removePunctuation(word);

            }

            in>>word;
        }
    }

    void addWord(QString word, QString prevWord, QString prevWord2){
        gram1MMB.insert(removePunctuation(word));
        gram2MMB.insert(prevWord, word);
        gram3MMB.insert(prevWord2,word);


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
        gram1MMB.printWordFrequency();
        gram2MMB.printWordFrequency(gram1MMB.getMaxArray());
//        gram3MMB.printWordFrequency(gram1MMB.getMaxArray());

    }


    map<string, map<string, double> >  normGram2(int fileCount){
        return gram2MMB.getNormalizedMap(fileCount);
    }

     map<string, map<string, double> >  normGram3(int fileCount){
        return gram3MMB.getNormalizedMap(fileCount);
    }

    MaxArray getMaxArray(){
         return gram1MMB.getMaxArray();

     }

};




#endif // MMB_H

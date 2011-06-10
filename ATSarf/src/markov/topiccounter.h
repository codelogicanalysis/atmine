#ifndef TOPICCOUNTER_H
#define TOPICCOUNTER_H

#include "MMB.h"
#include "maxArray.h"
#include "markovModel.h"
#include <vector>
#include <map>

class TopicCounter
{
private:
    MarkovModel markovModel;
    string topic;
    string* textFileNames;
    MMB* mmbuilder;
    int fileCount;
    map<string, double> maxFreqGram1;
    map<string, map<string, double> > maxFreqGram2;
    map<string, map<string, double> > maxFreqGram3;
    double sum;
    vector<string> textNames;

public:
    TopicCounter(){
    fileCount = 0;
    }
    void addTextName(string text){
        textNames.push_back(text);

    }
    void clearTextNames(){
        textNames.clear();

    }
    void evalutateMarkov(){
        calculateTopicMaps();
        printTopicMap();
        printWeightMap();
        buildMarkovModel();
        markovModel.printModel();

    }
    void setTopic(string newTopic){
        topic = newTopic;
    }
    void setTexts(string textNames[], int length){
        fileCount = length;
        cout<<"From file: "<<textNames[0]<<endl;
        cout<<"From file: "<<textNames[1]<<endl;
        textFileNames = textNames;
        mmbuilder = new MMB[length];
        for(int i=0; i<length; i++){
            cout<<"From file: "<<textNames[i]<<endl;
            mmbuilder[i].getWords(textNames[i]);
            mmbuilder[i].printWordFrequency();
        }
    }
    void setTexts(vector<string> textNames){
        cout<<textNames[0];
        fileCount = textNames.size();
        textFileNames = new string[fileCount];
        mmbuilder = new MMB[fileCount];
        for(int i=0; i<textNames.size(); i++){
            textFileNames[i] = textNames[i];
            cout<<"From file: "<<textNames[i]<<endl;
            mmbuilder[i].getWords(textNames[i]);
            mmbuilder[i].printWordFrequency();
        }
    }

    void buildMarkovModel(){
        cout<<"Building Markov Model..."<<endl;
        map<string, double>::const_iterator iter;
        map<string, double>::const_iterator iter2;
        map<string, double>::const_iterator iter3;
        double gram1Weight;
        double gram2Weight;
        double gram3Weight;
        string word;
        for (iter=maxFreqGram1.begin(); iter != maxFreqGram1.end(); ++iter){
            word = iter->first;
            markovModel.setGram1Prob(word, iter->second);
            gram1Weight = static_cast<double>(iter->second);
            for(iter2 = maxFreqGram2[word].begin(); iter2 != maxFreqGram2[word].end(); ++iter2){
                 gram2Weight = static_cast<double>(iter2->second);
                 markovModel.setGram2Prob(iter2->first, gram1Weight*gram2Weight);
            }
            for(iter3 = maxFreqGram3[word].begin();iter3 !=maxFreqGram3[word].end(); ++iter3){
                 gram3Weight = static_cast<double>(iter3->second);
                 markovModel.setGram3Prob(iter3->first, gram1Weight*gram3Weight);
            }
        }
        cout<<"Completed Building Markov Model.."<<endl;
    }
    MarkovModel getMarkovModel(){
        return markovModel;

    }

    void calculateTopicMaps(){
        //file loop
        for(int i=0; i < fileCount; i++){
            cout<<"From file: "<<textFileNames[i]<<endl;
            MaxArray maxArray = mmbuilder[i].getMaxArray();
            cout<<"Normalizing..."<<endl;
            maxFreqGram2 = mmbuilder[i].normGram2(fileCount);
            maxFreqGram3 = mmbuilder[i].normGram3(fileCount);
            //document loop
            for(int j=0; j< maxArray.getMaxNumber(); j++){
                cout<<maxArray.getWord(j)<<" "<<maxArray.getFreq(j)/fileCount<<endl;
                 maxFreqGram1[maxArray.getWord(j)] += maxArray.getFreq(j)/fileCount;
                 sum += maxArray.getFreq(j)/fileCount;
            }
        }
        cout<<endl<<"Completed Topic Map calculations..."<<endl;
    }

    int maximumOf(int int1, int int2){
        if(int1 >= int2)
            return int1;
        return int2;

    }

    void printWeightMap(){
        cout<<endl<<endl<<"The average weights"<<endl;
        map<string, double>::const_iterator iter;
        for (iter=maxFreqGram1.begin(); iter != maxFreqGram1.end(); ++iter){
             cout<<iter->first<< "      "<<iter->second*10/sum<<endl;
        }

        cout<<"Completed Printing Gram 1 weights..."<<endl;

    }
    void printTopicMap(){
        cout<<endl<<endl<<"The average frequencies"<<endl;
        map<string, double>::const_iterator iter;
        map<string, double>::const_iterator iter2;
        map<string, double>::const_iterator iter3;
        string word;
        for (iter=maxFreqGram1.begin(); iter != maxFreqGram1.end(); ++iter){
            word = iter->first;
            cout<<endl<<word<< "      "<<iter->second<<endl;
             for (iter2 =maxFreqGram2[word].begin();iter2 != maxFreqGram2[word].end();  ++iter2){
                  cout<<"Gram2: "<<iter2->first<<" "<<iter2->second<<endl;
             }
             for(iter3 = maxFreqGram3[word].begin(); iter3 != maxFreqGram3[word].end(); ++iter3){
                 cout<<"Gram3: "<<iter3->first<<" "<<iter3->second<<endl;

             }
        }
        cout<<"Completed printing topic Map."<<endl;

    }
    double getWeightGram1(string word){
        return maxFreqGram1[word]*10/sum;
    }
    double getWeightGram2(string word, string secondWord){
        return maxFreqGram2[word][secondWord]*10/sum;
    }
    double getWeightGram3(string word, string thirdWord){
        return maxFreqGram3[word][thirdWord]*10/sum;
    }




};

#endif // TOPICCOUNTER_H

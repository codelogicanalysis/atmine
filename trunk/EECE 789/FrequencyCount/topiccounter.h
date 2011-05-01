#ifndef TOPICCOUNTER_H
#define TOPICCOUNTER_H

#include "textcounter.h"
#include "maxArray.h"

class TopicCounter
{
private:
    string topic;
    string* textFileNames;
    TextCounter* counter;
    int fileCount;
    map<string, double> maxfreqAverage;
    map<string, double> maxfreqWeight;

public:
    TopicCounter(){
    fileCount = 0;
    }
    void setTopic(string newTopic){
        topic = newTopic;
    }
    void setTexts(string textNames[], int length){
        fileCount = length;
        textFileNames = textNames;
        counter = new TextCounter[length];
        for(int i=0; i<length; i++){
            cout<<"From file: "<<textNames[i]<<endl;
            counter[i].getWords(textNames[i]);
            counter[i].printWordFrequency();
        }
        getTopicMap();
        printTopicMap();
        printWeightMap();

    }
    void getTopicMap(){
        double sum = 0;
        for(int i=0; i < fileCount; i++){
            cout<<"From file: "<<textFileNames[i]<<endl;
            MaxArray maxArray = counter[i].getMaxArray();
            for(int j=0; j< maxArray.getMaxNumber(); j++){
                 maxfreqAverage[maxArray.getWord(j)] += maxArray.getFreq(j)/fileCount;
                 sum += maxArray.getFreq(j)/fileCount;
             }

         }

        map<string, double>::const_iterator iter;
        for (iter=maxfreqAverage.begin(); iter != maxfreqAverage.end(); ++iter){
            double average = iter->second;
            maxfreqWeight[iter->first] = average*10/sum;
            }

    }
    void printWeightMap(){
        cout<<endl<<endl<<"The average weights are:"<<endl;
        map<string, double>::const_iterator iter;
            for (iter=maxfreqWeight.begin(); iter != maxfreqWeight.end(); ++iter){
                 cout<<iter->first<< "      "<<iter->second<<endl;
            }

    }
    void printTopicMap(){
        cout<<endl<<endl<<"The average frequencies"<<endl;
        map<string, double>::const_iterator iter;
            for (iter=maxfreqAverage.begin(); iter != maxfreqAverage.end(); ++iter){
                 cout<<iter->first<< "      "<<iter->second<<endl;
            }

    }
    double getWeight(string word){
        return maxfreqWeight[word];
    }
};

#endif // TOPICCOUNTER_H

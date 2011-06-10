#ifndef TOPICCOMPARATOR_H
#define TOPICCOMPARATOR_H

#include "topiccounter.h"
#include "MMB.h"
#include "maxArray.h"

#include <string>
#include <fstream>
#include <iostream>
using namespace std;

class TopicComparator
{
private:

    TopicCounter topicCounter;

    double crossWeight;

public:
    TopicComparator(){
    crossWeight = 0;
    }
    TopicComparator(TopicCounter tCounter){
        topicCounter = tCounter;
        crossWeight =0;
    }
    void compare(string fileName){
        MMB mmbuilder;
        crossWeight = 0;
        cout<<"File to be compared: "<<fileName<<endl;
        mmbuilder.getWords(fileName);
        mmbuilder.printWordFrequency();

        //see if maximum array
        MaxArray maxArrray = mmbuilder.getMaxArray();
        //get weight from Comparitor and multiply with maximum number of occurances
        for(int i=0; i<maxArrray.getMaxNumber(); i++)
            crossWeight += topicCounter.getWeightGram1(maxArrray.getWord(i))*maxArrray.getFreq(i);


        crossWeight /= maxArrray.getFreqSum();

        cout<<"Cross Weight is: "<<crossWeight<<endl;

//still needs to be fixed



    }


};

#endif // TOPICCOMPARATOR_H

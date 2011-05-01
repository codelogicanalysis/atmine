#ifndef TOPICCOMPARATOR_H
#define TOPICCOMPARATOR_H

#include "topiccounter.h"
#include "textcounter.h"
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
        TextCounter textCounter;
        crossWeight = 0;
        cout<<"File to be compared: "<<fileName<<endl;
        textCounter.getWords(fileName);
        textCounter.printWordFrequency();

        //see if maximum array
        MaxArray maxArrray = textCounter.getMaxArray();
        //get weight from Comparitor and multiply with maximum number of occurances
        for(int i=0; i<maxArrray.getMaxNumber(); i++)
            crossWeight += topicCounter.getWeight(maxArrray.getWord(i))*maxArrray.getFreq(i);


        crossWeight /= maxArrray.getFreqSum();

        cout<<"Cross Weight is: "<<crossWeight<<endl;





    }


};

#endif // TOPICCOMPARATOR_H

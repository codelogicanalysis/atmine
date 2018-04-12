#include "textcounter.h"
#include "maxArray.h"
#include "topiccounter.h"
#include "topiccomparator.h"
#include <string>
#include <iostream>
#include <fstream>
using namespace std;




int main(){
//    TextCounter counter;
//    counter.getWords("religion.txt");
//    counter.printWordFrequency();

    string  texts[]= {"text1.txt", "text2.txt", "text3.txt"};
    TopicCounter topicCounter;
    topicCounter.setTexts(texts, 3);

    TopicComparator topicComparator(topicCounter);
    topicComparator.compare("text4.txt");
    topicComparator.compare("religion.txt");
    topicComparator.compare("text5.txt");







    return 0;
}

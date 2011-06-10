#ifndef TOPICTEXTS_H
#define TOPICTEXTS_H

#include <vector>
#include <iostream>
#include <fstream>
using namespace std;


class TopicTexts{
  private:
    string topic;
    vector<string> texts;
    int position;
  public:
    TopicTexts(){
    topic ="";
    }
    TopicTexts(string topicName){
        topic = topicName;
    }
    void setTopic(string topicName){

        topic = topicName;
    }
    void addText(string textName){
        texts.push_back(textName);
    }
    string getTopic(){
        return topic;

    }
    vector<string> getTexts(){
        return texts;

    }





};

class TextIntiator{
  private:
    vector<TopicTexts* > topicText;
    int position;
  public:
    TextIntiator(){
    position =0;
    }
    void intialize(){
        ifstream in;
        in.open("./topic/topic.txt");
        string word;
        in>>word;
        while(!in.eof()){
            cout<<word<<endl;
            topicText.push_back( new TopicTexts(word)) ;
            position++;
            in>>word;

        }
        in.close();
        ifstream inFile;
        for(int i=0; i< topicText.size(); i++){
            cout<<"Topic is: "<<topicText[i]->getTopic()<<endl;
            inFile.open(topicText[i]->getTopic().c_str());
            string texts;
            inFile>>texts;
            while(!inFile.eof()){
                cout<<texts<<endl;
                topicText[i]->addText(texts);
                inFile>>texts;
            }
            inFile.close();
        }

    }
    TopicTexts* getTopicText(string topic){
        for(int i=0; i< topicText.size(); i++){
            if(topicText[i]->getTopic()==topic){
                return topicText[i];
            }
        }
        return new TopicTexts();

    }


};



#endif // TOPICTEXTS_H

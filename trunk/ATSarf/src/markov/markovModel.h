#ifndef MARKOVMODEL_H
#define MARKOVMODEL_H

class GramProb{
 private:
    double gram1Weight;
    double gram2Weight;
    double gram3Weight;
public:
    GramProb(){
    gram1Weight = 0;
    gram2Weight = 0;
    gram3Weight = 0;
    }
    GramProb(double gram1, double gram2, double gram3){
        gram1Weight = gram1;
        gram2Weight = gram2;
        gram3Weight = gram3;
    }
    double getGram1Weight(){
        return gram1Weight;
    }
    double getGram2Weight(){
        return gram2Weight;
    }
    double getGram3Weight(){
        return gram3Weight;
    }
    void setGram2(double gram2){
       gram2Weight += gram2;
    }
    void setGram1(double gram1){
       gram1Weight += gram1;
    }
    void setGram3(double gram3){
       gram3Weight += gram3;
    }



};




class MarkovModel{
  private:
      map<string, GramProb> markovModelMap;

  public:
      MarkovModel(){}

      void setGram1Prob(string word, double weight){
            markovModelMap[word].setGram1(weight);
      }
      void setGram2Prob(string word, double weight){
          markovModelMap[word].setGram2(weight);
      }
      void setGram3Prob(string word, double weight){
          markovModelMap[word].setGram3(weight);
      }
      //return probability of word existing.
      double getProbWord(string word){
          double probability = 0;
          GramProb  gramProbWord = markovModelMap[word];
          probability = gramProbWord.getGram1Weight() + gramProbWord.getGram1Weight()*gramProbWord.getGram2Weight();
          probability += gramProbWord.getGram3Weight()*gramProbWord.getGram2Weight()*gramProbWord.getGram1Weight();
          return probability;
      }
      double getCrossWeight(string word, double gram1, double gram2, double gram3){
          GramProb  gramProbWord = markovModelMap[word];
          return gram1*gramProbWord.getGram1Weight() + gram2*gramProbWord.getGram2Weight()+ gram3*gramProbWord.getGram3Weight();

      }
      void printModel(){
        cout<<endl<<endl<<"Markov Model"<<endl;
        map<string, GramProb>::const_iterator iter;
        for (iter=markovModelMap.begin(); iter != markovModelMap.end(); ++iter){
            GramProb gramProb = iter->second;
            cout<<iter->first<<"\t \t"<<gramProb.getGram1Weight()<<"\t \t"<<gramProb.getGram2Weight()<<"\t \t"<<gramProb.getGram3Weight()<<endl;
        }

      }

};






#endif // MARKOVMODEL_H

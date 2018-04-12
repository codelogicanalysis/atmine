#ifndef MAXARRAY_H
#define MAXARRAY_H

#include <iostream>
using namespace std;


class MaxArray{

    private:
        string * mArray;
        int *numberArray;
        int max;
        int min;
        int minPosition;
        int head;
        int arrayMaximum;
        void initilize(){
            mArray = new string[arrayMaximum];
            numberArray = new int[arrayMaximum];
        }
    public:
        MaxArray(){;
            arrayMaximum=10;
            head=0;
            max=0;
            minPosition=0;
            initilize();

        }
        MaxArray(int maximum){
            arrayMaximum = maximum;
            head=0;
            max=0;
            minPosition=0;
            initilize();

        }

        void insert(string word, int number){
            if(number >= max){
             mArray[head] = word;
             numberArray[head] = number;
             max=number;
             head++;
             head= head%arrayMaximum;
            }
            else if(number>numberArray[minPosition]){
                numberArray[minPosition] = number;
                mArray[minPosition] = word;
                for(int i=0; i<arrayMaximum; i++){
                    if(numberArray[i] <numberArray[minPosition]){
                        minPosition=i;
                }
            }


            }


        }
        void print(){
            cout<<"Maximum Occuring Numbers"<<endl;
            for(int i=0; i<arrayMaximum; i++)
                cout<<mArray[i]<<"  "<<numberArray[i]<<endl;
        }
        string getWord(int pos){
            return mArray[pos];
        }
        double getFreq(int pos){
            return static_cast<double>(numberArray[pos]);
        }
        int getMaxNumber(){
            return arrayMaximum;
        }
        void setMaxNumber(int number){
            arrayMaximum = number;
            initilize();
        }
        int getFreqSum(){
            int sum =0;
            for(int i=0; i<arrayMaximum; i++)
                sum+=numberArray[i];
            return sum;
        }





};



#endif // MAXARRAY_H

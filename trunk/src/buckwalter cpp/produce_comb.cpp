#include <Qstring>
#include <QDebug>
#include <QStringRef>
#include <cmath>
#include <Qdebug>

int produce_comb(QString word, QString**result){
    int n=word.length();
    int max_comb=pow(2,n-1);
    int max_length=5;

    result=new QString*[max_comb];


    int splitter[32][5]={{-1},{0,-1},{1,-1},{0,1,-1},{2,-1},
                         {0,2,-1},{1,2,-1},{0,1,2,-1},{3,-1},
                         {0,3,-1},{1,3,-1},{0,1,3,-1},{2,3,-1},
                         {0,2,3,-1},{1,2,3,-1},{0,1,2,3,-1},
                         {4,-1},{0,4,-1},{1,4,-1},{0,1,4,-1},
                         {2,4,-1},{0,2,4,-1},{1,2,4,-1},{0,1,2,4,-1},
                         {3,4,-1},{0,3,4,-1},{1,3,4,-1},{0,1,3,4,-1},
                         {2,3,4,-1},{0,2,3,4,-1},{1,2,3,4,-1},{0,1,2,3,4}};


    for (int i=0;i<max_comb;i++){ //loop over all the combinations

        int k=0;//counter of number of parts in each combination

        int last_index=0;//start of next partition

        qDebug()<<i;


        int one_count=0;
        if (i==max_comb-1){
        one_count=n-1;
        }

        else{

            for (int j=max_length-1; j>=0;j--){ //n-1 seperators possible
                if (splitter[i][j]==-1){
                    one_count=j;
                    break;
                }
            }
        }

        result[i]=new QString[one_count+1];//allocate memory

        for (int j=0; j<n;j++) //n-1 seperators possible
        {

                if (splitter[i][j]!=-1)
                    result[i][k]=word.midRef (last_index, splitter[i][j]-last_index+1).toString();

                else
                {
                    result[i][k]=word.midRef (last_index, n-1-last_index+1).toString();
                    qDebug()<<result[i][k];
                    break;
                }
                qDebug()<<result[i][k];


                last_index=splitter[i][j]+1;
                k++;
        }


    }

return max_comb;
}


int main(){
    QString **result;
    produce_comb("",result);
    return 0;
}


#include <Qstring>
#include <QDebug>
#include <QStringRef>
#include <cmath>
#include <Qdebug>

//prefix_info *array
int produce_comb(QString word, QString**result){
    int n=word.length();
    int max_comb=pow(2,n-1);

    QString splitter;
    result=new QString*[max_comb];


    for (int i=0;i<max_comb;i++){ //loop over all the combinations

        splitter=QString::number(i,2);
        int k=0;//counter of number of parts in each combination
        int next_index=0;//start of next partition

        printf(("splitter:"+splitter+"\n").toStdString().data());

        int splitter_length=splitter.length();

        int one_count=0;
        for (int j=splitter_length-1; j>=0;j--){ //n-1 seperators possible
            if (splitter[j]=='1'){
                one_count++;
            }
        }

        result[i]=new QString[one_count+1];//allocate memory

        for (int j=splitter_length-1; j>=0;j--) //n-1 seperators possible
        {

            if (splitter[j]=='1'){

                //printf((splitter+"\n").toStdString().data());

                result[i][k]=word.midRef (next_index, splitter_length-1-j-(next_index-1)).toString();

                //printf(word.midRef (next_index, splitter_length-1-j-(next_index-1)).toString().toStdString().data());
                printf("%s\n",result[i][k].toStdString().data());
                //printf("\n%d,%d\n",next_index,splitter_length-1-j-(next_index-1));
                next_index=splitter_length-j;
                k++;

            }

        }

            result[i][k]=word.midRef (next_index, n-next_index).toString();
            printf("%s\n",result[i][k].toStdString().data());

    }
return max_comb;
}

int main(){
    QString **result;
    produce_comb("abc",result);
    return 0;
}

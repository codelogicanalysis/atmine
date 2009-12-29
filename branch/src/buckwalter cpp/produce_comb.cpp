#include <Qstring>
#include <QDebug>
#include <QStringRef>

produce_comb(QString word, int n, QString ***array){

    if (word.length()==0)
    {
        n++;
        return;
    }

    for (i=0;i<word.length();i++){

        QString left_portion=word.leftRef(i).toString();
        QString right_portion=word.rightRef(i).toString();

        store (left_portion,n,array);
        produce_comb(right_portion,n);

    }

}

store (QString left_portion,int n, QString ***array){
    int i=0;
    while (array[n][i][0]!="\0"){
    i++;
    }
    array[n][i][0]=new QString(left_portion);
    get_and_store_prefix_cats(left_portion,0);

}


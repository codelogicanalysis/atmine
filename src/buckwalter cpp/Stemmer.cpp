//#include "Stemmer.h"

#include <QString>
 #include <QStringRef>
typedef struct id_category {
//add data
} id_category_t;


bool prefix_exists(QString prefix){
return QStringRef   ("hello");
}


bool suffix_exists(QString suffix){
return 1;
}


bool stem_exists(QString stem){
return 1;
}

bool P_T_compatible(QString stem_cat,QString suffix_cat){
return 1;
}

bool P_S_compatible(QString prefix_cat, QString stem_cat){
return 1;
}

bool T_S_compatible(QString stem_cat, QString suffix_cat){
return 1;
}

void get_and_output_info_id(int stem_id){

}

QString* get_prefix_cats(QString prefix){
    return ;
}
id_category_t* get_stem_cats(QString stem){
return 0;
}
QString* get_suffix_cats(QString suffix){
return 0;
}





QString * segment_word(QString str){

QString a;
QString segmented;
QString prefix_cat, suffix_cat;
QString prefix,stem, suffix;
id_category_t stem_cat;
int prefix_len=0;
int suffix_len=0;
int stem_len,stem_id;
int str_len=str.length();


while(prefix_len<=4){
    QString prefix=str.leftRef(prefix_len).toString();
    stem_len=str_len-prefix_len;
    suffix_len=0;
    while ((stem_len>=1) and (suffix_len<=6)){

        stem=str.midRef  (prefix_len, stem_len).toString();
        suffix = str.midRef ((prefix_len + stem_len), suffix_len).toString();

        //push the segmented word parts
        if (prefix_exists (prefix)){
            if (suffix_exists(suffix)){
                if (stem_exists(stem)){

                    // all 3 components exist in their respective lexicons, but are they compatible? (check the $cat pairs)
                    QString *prefix_array;
                    QString *suffix_array;
                    id_category_t * stem_array;//create a new struct

                    prefix_array=get_prefix_cats(prefix);
                    stem_array=get_stem_cats (stem);
                    suffix_array=get_suffix_cats (suffix);

                    foreach (prefix_cat,prefix_array){
                        foreach (stem_cat,stem_array){

                                if (P_T_compatible(prefix_cat,stem_cat))

                                        foreach (suffix_cat,suffix_array){
                                            if (P_S_compatible(stem_cat,suffix_cat))
                                                if (T_S_compatible(stem_cat,suffix_cat))
                                                        get_and_output_info_id (stem_id);


                                        }
                        }
                     }
               }
           }
        }

        stem_len--;
        suffix_len++;
        }
prefix_len++;
}

}


int main(){
return 0;
}

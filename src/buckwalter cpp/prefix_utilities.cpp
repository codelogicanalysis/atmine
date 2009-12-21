#include "produce_comb.cpp"

bool prefix_exists(QString prefix,prefix_info_t * info){
    QString ** result;
    QString * cat_ids_info;
    QString portion;
    int max_comb=produce_comb(prefix,result);
    info=new prefix_info_t [max_comb*max_cat];

    for (int i=0;i<max_comb;i++){

        bool comb_exists=TRUE;
        foreach(portion,result[i]){//verify existence.
            minimal_item_info * info_array;
            if (search(PREFIX,portion,info_array)==0)
                comb_exists=FALSE;
            else{}

            //returns true if portion exists and gives its category
            }
        }
        if (comb_exists){
            foreach(portion,result[i]){//verify compat.
                cat_ids_info[k]=get_portion_info(portion);//returns true if portion exists and gives its category


        }

    }
}


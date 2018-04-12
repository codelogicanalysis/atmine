#ifndef SEARCH_COMPATIBILITY_H
#define SEARCH_COMPATIBILITY_H

#include "sql_queries.h"

class Search_Compatibility {
    private:
        QSqlQuery query;
        rules rule;
        bool err;
        bool retrieve_internal(long &category2, long &resulting_category);
    public:
        //first means first category provided as parameter, while being false means second provided
        Search_Compatibility(rules rule, long category_id, bool first = true);
        //total size and not what is left
        int size();
        bool retrieve(long &category2, long &resulting_category);
        bool retrieve(long &category2);
        int retrieve(long category2_ids[], long resulting_categorys[], int size_of_array);
        int retrieve(long category2_ids[], int size_of_array);
        QString getInflectionRules();
};

#endif

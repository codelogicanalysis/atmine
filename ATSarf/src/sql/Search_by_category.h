#ifndef SEARCH_BY_CATEGORY_H
#define SEARCH_BY_CATEGORY_H

#include "sql_queries.h"

class Search_by_category {
    private:
        QSqlQuery query;
        item_types type;
        bool err;
        bool retrieve_internal(long long &item_id);
        bool retrieve_internal(all_item_info &info);
    public:
        explicit Search_by_category(long category_id);
        //total size and not what is left
        int size();
        bool retrieve(long long &item_id);
        bool retrieve(all_item_info &info);
        int retrieve(long long item_ids[], int size_of_array);
};

#endif

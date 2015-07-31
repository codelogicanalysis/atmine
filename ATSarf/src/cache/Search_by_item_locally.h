#ifndef SEARCH_BY_ITEM_LOCALLY_LOCALLY_H
#define SEARCH_BY_ITEM_LOCALLY_LOCALLY_H

#include <QHash>
#include "sql_queries.h"
#include "database_info_block.h"

class Search_by_item_locally {
    private:
        QHash<ItemEntryKey, ItemEntryInfo > *map;
        item_types type;
        long long id;
        long category_id;
        QString raw_data;

        QList<ItemEntryInfo > list;
        int index;
    public:
        Search_by_item_locally(item_types type, long long id, long category_id, QString &raw_data);
        bool retrieve(minimal_item_info &info);
};

#endif


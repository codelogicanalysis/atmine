
#include "Search_by_category.h"
#include "sql_queries.h"

bool Search_by_category::retrieve_internal(long long &item_id)
{
    bool ok;
    item_id =query.value(0).toULongLong(&ok);
    if (!ok)
    {
        _error << "Unexpected Error: Non-integer item_id's\n";
        return false;
    }
    return true;
}
bool Search_by_category::retrieve_internal(all_item_info & info)
{
    info.item_id=query.value(0).toULongLong();
    info.category_id =query.value(1).toULongLong();
    info.sources=string_to_bitset(query.value(2));
    info.raw_data=query.value(3).toString();
    info.POS=query.value(4).toString();
    if (query.value(5).isNull())
        info.setDescription(-1);
    else
    {
        info.setDescription(query.value(5).toULongLong());
    }
    if (type==STEM)
    {
        info.abstract_categories=string_to_bitset(query.value(6));
        info.lemma_ID=query.value(7).toString();
    }
    else
    {
        info.abstract_categories.reset();
        info.lemma_ID="";
    }
    return true;
}
Search_by_category::Search_by_category(long category_id)
{
    err=false;
    QSqlQuery temp(theSarf->db);
    query=temp;
    get_type_of_category(category_id,type);
    QString table = interpret_type(type);
    QString stmt( "SELECT %1_id, category_id, sources, raw_data, POS, description_id %3 FROM %1_category WHERE category_id ='%2' ORDER BY %1_id ASC");
    stmt=stmt.arg(table).arg(category_id).arg((type==STEM?", abstract_categories, lemma_ID":""));
    if (!execute_query(stmt,query)) //will use the local query
        err=true;
}
int Search_by_category::size() //total size and not what is left
{
    return query.size();
}
bool Search_by_category::retrieve(long long &item_id)
{
    if (!err && query.next())
        return retrieve_internal(item_id);
    else
        return false;
}
bool Search_by_category::retrieve(all_item_info &info)
{
    if (!err && query.next())
        return retrieve_internal(info);
    else
        return false;
}
int Search_by_category::retrieve(long long item_ids[],int size_of_array)
{
    if (size_of_array<=0 || !err)
        return 0;
    int i;
    for (i=0; i<size_of_array && query.next();i++)
    {
        if (!retrieve_internal(item_ids[i]))
            return -1;
    }
    return i;
}

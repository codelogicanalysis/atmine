#ifndef _SEARCH_BY_CATEGORY_H
#define	_SEARCH_BY_CATEGORY_H

#include "sql_queries.h"

class Search_by_category
{
private:
	QSqlQuery query;
	item_types type;
	bool err;
	inline bool retrieve_internal(long long &item_id);
	inline bool retrieve_internal(all_item_info & info);
public:
	Search_by_category(long category_id);
	inline int size(); //total size and not what is left
	inline bool retrieve(long long &item_id);
	inline bool retrieve(all_item_info &info);
	inline int retrieve(long long item_ids[],int size_of_array);
};

#endif	/* _SEARCH_BY_CATEGORY_H */


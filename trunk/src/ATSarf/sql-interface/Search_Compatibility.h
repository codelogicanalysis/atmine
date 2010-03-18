
#ifndef _SEARCH_BY_COMPATIBILITY_H
#define	_SEARCH_BY_COMPATIBILITY_H

#include "sql_queries.h"

class Search_Compatibility
{
private:
	QSqlQuery query;
	rules rule;
	bool err;
	inline bool retrieve_internal(long &category2, long &resulting_category);
public:
	Search_Compatibility(rules rule, long category_id1);
	inline int size(); //total size and not what is left
	inline bool retrieve(long &category2, long &resulting_category);
	inline bool retrieve(long &category2);
	inline int retrieve(long category2_ids[],long resulting_categorys[],int size_of_array);
	inline int retrieve(long category2_ids[],int size_of_array);
};


#endif	/* _SEARCH_BY_COMPATIBILITY_H */


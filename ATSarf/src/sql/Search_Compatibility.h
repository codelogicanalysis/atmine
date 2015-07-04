#ifndef _SEARCH_BY_COMPATIBILITY_H
#define	_SEARCH_BY_COMPATIBILITY_H

#include "sql_queries.h"

class Search_Compatibility
{
private:
	QSqlQuery query;
	rules rule;
	bool err;
	bool retrieve_internal(long &category2, long &resulting_category);
public:
	Search_Compatibility(rules rule, long category_id,bool first=true);//first means first category provided as parameter, while being false means second provided
	int size(); //total size and not what is left
	bool retrieve(long &category2, long &resulting_category);
	bool retrieve(long &category2);
	int retrieve(long category2_ids[],long resulting_categorys[],int size_of_array);
	int retrieve(long category2_ids[],int size_of_array);
	QString getInflectionRules();
};


#endif	/* _SEARCH_BY_COMPATIBILITY_H */


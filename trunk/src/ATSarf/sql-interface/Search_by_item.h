#ifndef _SEARCH_BY_ITEM_H
#define	_SEARCH_BY_ITEM_H

#include "sql_queries.h"

class Search_by_item
{
private:
	QSqlQuery query;
	item_types type;
	long long id;
	QString name;
	inline bool retrieve_internal(long &category_id); //returns just a category but can contain redundancy
	inline bool retrieve_internal(all_item_info &info);
	inline bool retrieve_internal(minimal_item_info &minimal);
public:
	Search_by_item(item_types type,long long id);
	Search_by_item(item_types type,QString name);
	inline long long ID();
	inline QString Name();
	inline int size(); //total size and not what is left
	inline bool retrieve(long &category_id); //returns just a category but can contain redundancy
	inline bool retrieve(all_item_info & info);
	inline bool retrieve(minimal_item_info &info);
	inline int retrieve(long category_ids[],int size_of_array);
	inline int retrieve(minimal_item_info info[], int size_of_array);
	inline int retrieve(all_item_info info[], int size_of_array);
	inline QString getGrammarStem(); //just for stems
	inline bitset<max_sources> getGrammarStem_sources(); //just for stems
};


#endif	/* _SEARCH_BY_ITEM_H */


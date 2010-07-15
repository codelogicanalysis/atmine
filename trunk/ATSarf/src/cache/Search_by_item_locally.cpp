#include "Search_by_item_locally.h"
#include "sql_queries.h"

Search_by_item_locally::Search_by_item_locally(item_types type,long long id, long category_id, QString & raw_data)
{
	if (type==PREFIX)
		map=database_info.map_prefix;
	else if (type==STEM)
		map=database_info.map_stem;
	else if (type==SUFFIX)
		map=database_info.map_suffix;
	this->type=type;
	this->id=id;
	this->category_id=category_id;
	this->raw_data=raw_data;
	list = map->values(Map_key(id,category_id,raw_data));
	index=0;
}
bool Search_by_item_locally::retrieve(minimal_item_info &info)
{
	if (index<list.size())
	{
		info.abstract_categories=list[index].first;
		info.description_id=list[index].second;
		info.POS=list[index].third;
		info.category_id=category_id;
		info.raw_data=raw_data;
		info.type=type;
		++index;
		return true;
	}
	else
		return false;
}

#ifndef _Search_by_item_locally_LOCALLY_H
#define	_Search_by_item_locally_LOCALLY_H

#include <QHash>
#include "../sql-interface/sql_queries.h"
#include "../caching_structures/database_info_block.h"


class Search_by_item_locally
{
private:
	QHash<Map_key,Map_entry > * map;
	item_types type;
	long long id;
	long category_id;
	QString raw_data;

	QList<Map_entry > list;
	int index;
public:
	Search_by_item_locally(item_types type,long long id, long category_id, QString raw_data);
	bool retrieve(minimal_item_info &info);
};


#endif	/* _Search_by_item_locally_LOCALLY_H */


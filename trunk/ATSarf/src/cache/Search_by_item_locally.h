#ifndef _Search_by_item_locally_LOCALLY_H
#define	_Search_by_item_locally_LOCALLY_H

#include <QHash>
#include "sql_queries.h"
#include "database_info_block.h"


class Search_by_item_locally
{
private:
	ItemCatRaw2PosDescAbsMap * map;
	item_types type;
	long long id;
	long category_id;
	QString raw_data;

	QList<Map_entry > list;
	int index;
public:
	Search_by_item_locally(item_types type,long long id, long category_id, QString & raw_data);
	bool retrieve(minimal_item_info &info);
	bool retrieve(minimal_item_info & info, int index);//provides its own index and does not change the internal index
	int count();
};


#endif	/* _Search_by_item_locally_LOCALLY_H */


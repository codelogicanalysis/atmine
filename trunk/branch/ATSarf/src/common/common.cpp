#include "common.h"
#include "database_info_block.h"

QString minimal_item_info_::description() const
{
	if (description_id>0 && description_id<database_info.descriptions->size())
		return database_info.descriptions->at(description_id);
	else
		return QString::null;
}

QString all_item_info_::description() const
{
	if (description_id>0 && description_id<database_info.descriptions->size())
		return database_info.descriptions->at(description_id);
	else
		return QString::null;
}

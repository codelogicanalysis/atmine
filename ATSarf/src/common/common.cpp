#include "common.h"
#include "database_info_block.h"

QString minimal_item_info_::description()
{
	if (!desc.isEmpty())
		return desc;
	if (desc_id>0 && desc_id<database_info.descriptions->size()) {
		desc=(*database_info.descriptions)[desc_id];
		return desc;
	} else
		return QString::null;
}


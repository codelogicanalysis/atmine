#ifndef NODE_INFO_H
#define NODE_INFO_H

#include "result_node.h"
#include "database_info_block.h"

class node_info //TODO: rename, get and set
{
public:
	result_node * node;
	int pos_in_tree;//just for internal use
	long start;
	long finish;
}; 
typedef QPair<int,ItemCatRaw2PosDescAbsMapItr> AffixPosition;
typedef QVector<AffixPosition> InternalPositions;

class solution_position
{
public:
	int current_index;
	InternalPositions indexes; //index of raw_data and of correxponding entry in map corresponding for this raw_data
};

#endif // NODE_INFO_H

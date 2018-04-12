/**
  * @file   node_info.h
  * @author Jad Makhlouta and documented by Ameen Jaber
  * @brief  This file implements the node_info class which holds information about a node found in the trie structure
  */
#ifndef NODE_INFO_H
#define NODE_INFO_H

#include "result_node.h"
#include "database_info_block.h"

/**
  * @class  node_info
  * @brief  This class defines a structure that holds information about a node that is found in the trie structure used
  * to store the affixes
  */
class node_info //TODO: rename, get and set
{
public:
        /// Pointer to the result_node structure which holds the information about the solution node
        result_node * node;
	int pos_in_tree;//just for internal use
        long start;
	long finish;
}; 
#endif // NODE_INFO_H

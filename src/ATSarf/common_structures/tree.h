#ifndef _TREE_H
#define	_TREE_H

#include "node.h"
#include "letter_node.h"
#include "result_node.h"
#include "../common_structures/common.h"

class tree
{
    protected:
		node* base;
		int letter_nodes, result_nodes;
		bool isAffix;
		item_types type;
		void delete_helper(node * current);
		void print_tree_helper(node * current_node, int level);
		int build_helper(item_types type, long cat_id1, int size, node * current);
#ifdef MEMORY_EXHAUSTIVE
		node* addElement(QString letters, long affix_id,long category_id, long resulting_category_id,QString raw_data,QString description,node * current);
#elif defined (REDUCE_THRU_DIACRITICS)
		node* addElement(QString letters, long affix_id,long category_id, long resulting_category_id,QString raw_data,node * current);
#else
		node* addElement(QString letters, long affix_id,long category_id, long resulting_category_id,node * current);
#endif
	public:
		tree();
		tree(item_types type);
		bool getAffixType(item_types &type);
		node* getFirstNode();
#if !defined(MEMORY_EXHAUSTIVE) && !defined(REDUCE_THRU_DIACRITICS)
		void sample();
#endif
		int build_affix_tree(item_types type);
		void reset();
		void print_tree();
		virtual ~tree();
};


#endif	/* _TREE_H */


#ifndef _TREE_H
#define	_TREE_H

#include "node.h"
#include "letter_node.h"
#include "result_node.h"
#include "common.h"

class tree
{
#ifdef LOAD_FROM_FILE
	private:
		typedef QMap<node *,int>  IDMap;
		typedef QMap<int,node *>  ReverseIDMap;
		IDMap idMap;
		ReverseIDMap reverseIDMap;
		int last_id;
		QDataStream * file;
		int generateNodeID(node* n);
		node* getNodeID(int num);
		void setNodeID(int num, node * n);
#endif
	protected:
		node* base;
		int letter_nodes, result_nodes;
		bool isAffix;
		item_types type;
		void delete_helper(node * current);
		void print_tree_helper(node * current_node, int level);
		int build_helper(item_types type, long cat_id1, int size, node * current);
#ifdef MEMORY_EXHAUSTIVE
		node* addElement(QString letters, long affix_id,long category_id, long resulting_category_id,bool isAccept,QString raw_data,QString description,node * current);
#elif defined (REDUCE_THRU_DIACRITICS)
		node* addElement(QString letters, long affix_id,long category_id, long resulting_category_id,bool isAccept,QString raw_data,node * current);
#else
		node* addElement(QString letters, long affix_id,long category_id, long resulting_category_id,bool isAccept,node * current);
#endif
	public:
		tree();
		tree(item_types type);
		bool getAffixType(item_types &type);
		node* getFirstNode()
		{
			return base;
		}
#if !defined(MEMORY_EXHAUSTIVE) && !defined(REDUCE_THRU_DIACRITICS)
		void sample();
#endif
		int build_affix_tree_from_file(item_types type);
		int build_affix_tree(item_types type);
		void reset()
		{
			base->resetChildren();
			letter_nodes=1;
			result_nodes=0;
			isAffix=false;
		}
		void print_tree();
		virtual ~tree()
		{
			reset();
			delete base;
		}
};


#endif	/* _TREE_H */


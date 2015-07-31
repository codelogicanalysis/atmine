#ifndef _TREE_H
#define	_TREE_H

#include "node.h"
#include "letter_node.h"
#include "result_node.h"
#include "common.h"
#include "Retrieve_Template.h"

class tree {
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
		node* addElement(QString letters, long affix_id,long category_id, long resulting_category_id,bool isAccept,QString raw_data, QString inflected_raw_data,QString descriptionInflectionRule,node * current);
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

inline bool isAcceptState(item_types type,long cat_r_id) {
	bool isAccept=false;
	if (type==PREFIX) {
		Retrieve_Template existABcheck("compatibility_rules","COUNT(*)",QString("category_id1=%1 AND type=%2").arg(cat_r_id).arg((int)AB));
		if (existABcheck.retrieve() && existABcheck.get(0).toInt()>0) {
			Retrieve_Template existACcheck("compatibility_rules","COUNT(*)",QString("category_id1=%1 AND type=%2").arg(cat_r_id).arg((int)AC));
			if (existACcheck.retrieve() && existACcheck.get(0).toInt()>0) {
				isAccept=true;
			}
		}
	} else {
		Retrieve_Template existACcheck("compatibility_rules","COUNT(*)",QString("category_id2=%1 AND type=%2").arg(cat_r_id).arg((int)AC));
		if (existACcheck.retrieve() && existACcheck.get(0).toInt()>0) {
			Retrieve_Template existBCcheck("compatibility_rules","COUNT(*)",QString("category_id2=%1 AND type=%2").arg(cat_r_id).arg((int)BC));
			if (existBCcheck.retrieve() && existBCcheck.get(0).toInt()>0) {
				isAccept=true;
			}
		}
	}
	return isAccept;
}


#endif	/* _TREE_H */


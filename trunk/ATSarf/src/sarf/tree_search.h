#ifndef _TREE_SEARCH_H
#define _TREE_SEARCH_H

#include <QVector>
#include <QString>
#include <QList>
#include <QQueue>
#include "tree.h"
#include "database_info_block.h"
#include "common.h"

class TreeSearch
{
	public://private
		bool filled_details;
		QList<int> sub_positionsOFCurrentMatch;//end of last split
        QList<long> catsOFCurrentMatch;
        QList<long> idsOFCurrentMatch;
        long resulting_category_idOFCurrentMatch;
	public:
		result_node * previousNode(result_node * node/*, int & start_pos, int & end_pos*/)
		{
			return node->getPreviousResultNode();
		}
		result_node * lastNode(/*int & start_pos, int & end_pos*/)
		{
			return reached_node;
		}
		QList<result_node *> * getSplitList()
		{
			QList<result_node *> * list=new QList<result_node *>();
			result_node * node=reached_node;
			while (node!=NULL)
			{
				list->prepend(node);
				node=node->getPreviousResultNode();
			}
			return list;
		}
		QList<int> & getSplitPositions()
		{
			return sub_positionsOFCurrentMatch;
		}
		long getFinalResultingCategory()
		{
			return resulting_category_idOFCurrentMatch;
		}
#ifdef REDUCE_THRU_DIACRITICS
		QList<QList <QString > > possible_raw_datasOFCurrentMatch;
#endif
        tree* Tree;
		text_info info;
        item_types type;
#if defined(PARENT)
		result_node * reached_node;
#endif
        int position;//note that provided position is 1+last_letter after traversal
		bool reduce_thru_diacritics;

		virtual bool shouldcall_onmatch(int)
		{
			return true;
		}
		bool on_match_helper();
    public:
		TreeSearch(item_types type,QString* text,int start,bool reduce_thru_diacritics=false)
		{
			info.text=text;
			this->type=type;
			info.start=start;
			info.finish=start;
			this->reduce_thru_diacritics=reduce_thru_diacritics;
			if (type==PREFIX)
				Tree=database_info.Prefix_Tree;
			else if (type==SUFFIX)
				Tree=database_info.Suffix_Tree;
		}
        virtual bool operator()();
		virtual void fill_details(); //this function fills the public member functions such as QList<int> sub_positionsOFCurrentMatch & QList<long> catsOFCurrentMatch;
        virtual bool onMatch() = 0;// returns true to continue, false to abort
		virtual ~TreeSearch(){}
};

#endif // TREE_SEARCH_H

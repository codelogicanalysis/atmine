#ifndef _TREE_SEARCH_H
#define _TREE_SEARCH_H

#include <assert.h>
#include <QVector>
#include <QString>
#include <QList>
#include <QQueue>
#include "tree.h"
#include "database_info_block.h"
#include "common.h"
#include "node_info.h"
#include "solution_position.h"
#include "node.h"
#include "diacritics.h"
//#include "Search_by_item_locally.h"

class Stemmer;

class TreeSearch
{
	public://private
		bool filled_details;
		multiply_params multi_p;
		QList<int> sub_positionsOFCurrentMatch;//end of last split
	#ifndef MULTIPLICATION
		QList<long> catsOFCurrentMatch;
		QList<long> idsOFCurrentMatch;
	#else
		AffixSolutionVector affix_info;
		QList<result_node *> * result_nodes;
	private:
		ItemCatRaw2PosDescAbsMap * map;
		friend class Stemmer;
	public:
	#endif
        long resulting_category_idOFCurrentMatch;
	public:
		const AffixSolutionVector & getSolution() const {return affix_info;}//make sure affix_info is not null!!
		node_info * previousNode(node_info * current)
		{
			node* head=Tree->getFirstNode();
			if (current->node==NULL)
				return NULL;
			node_info * previous=new node_info;
			//previous->node=current->node->getPreviousResultNode();
			int pos_in_tree=current->pos_in_tree,finish=current->start,start=current->start;
			//pos_in_tree-=(current->finish-current->start+1);
			node * pre=current->node;
			pre=pre->getPrevious();
			while (pos_in_tree>=-1 && pre!=head && pre->isLetterNode())
			{
				pos_in_tree--;
				start--;
				pre=pre->getPrevious();
				//check if diacritic and ignore
				while (start>=0 && isDiacritic(info.text->at(start)))
					start--;
				assert(start>=0);
			}
			if (pos_in_tree==-1)
				assert(pre==head && start==info.start);
			previous->start=start;
			previous->finish=finish;
			previous->pos_in_tree=pos_in_tree;
			previous->node=(result_node*)pre;
			assert(start>=-1);
			assert(start<=finish);
			return previous;
		}
		node_info * lastNode()
		{
			node_info * last=new node_info;
			last->node=reached_node->getPreviousResultNode();
			last->pos_in_tree=position-1;
			last->finish=info.finish;
			last->start=(sub_positionsOFCurrentMatch.size()>1?sub_positionsOFCurrentMatch[sub_positionsOFCurrentMatch.size()-2]-1:info.start);
			return last;
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
#ifdef MULTIPLICATION
	private:
		void initializeAffixInfo(solution_position * sol_pos,int start_index); //zero and initialize solutions till 'last_index' exclusive
		bool increment(solution_position * info,int index);
	public:
		solution_position * computeFirstSolution()
		{
			if (type==PREFIX)
				map=database_info.map_prefix;
			else if (type==SUFFIX)
				map=database_info.map_suffix;

			affix_info.clear();
			/*if (result_nodes!=NULL)
				delete result_nodes;*/
			result_nodes=getSplitList();
			solution_position * first=new solution_position();
			initializeAffixInfo(first,0);
			return first;
		}
		bool computeNextSolution(solution_position * current)//compute next posibility
		{
			return increment(current,sub_positionsOFCurrentMatch.count()-1);
		}
#endif
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
		virtual bool shouldcall_onmatch_ex(int)
		{
			return true;
		}
		virtual bool shouldcall_onmatch(int)
		{
			return true;
		}
		bool on_match_helper();
		void setSolutionSettings(multiply_params params)
		{
			multi_p=params;
		}
    public:
		TreeSearch(item_types type,QString* text,int start,bool reduce_thru_diacritics=true)
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
			multi_p=M_ALL;
		#ifdef MULTIPLICATION
			result_nodes=NULL;
		#endif
		}
        virtual bool operator()();
		virtual void fill_details(); //this function fills the public member functions such as QList<int> sub_positionsOFCurrentMatch & QList<long> catsOFCurrentMatch;
        virtual bool onMatch() = 0;// returns true to continue, false to abort
		virtual ~TreeSearch(){}
};

#endif // TREE_SEARCH_H

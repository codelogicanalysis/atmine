#include "tree_search.h"

#include "suffix_search.h"
#include "stem_search.h"
#include "prefix_search.h"
#include "stemmer.h"
#include <QVector>
#include "../common_structures/node.h"
#include "../common_structures/tree.h"
#include "../utilities/text_handling.h"
#include "../utilities/diacritics.h"
#include "../caching_structures/database_info_block.h"

bool TreeSearch::shouldcall_onmatch(int)//re-implemented in case of SUFFIX search tree
{
        return true;
}
void TreeSearch::fill_details() //this function fills the public member functions such as QList<int> sub_positionsOFCurrentMatch & QList<long> catsOFCurrentMatch;
{
#ifdef QUEUE
        //nothing needs to be done, members are already filled during traversals
#elif defined(PARENT)
        sub_positionsOFCurrentMatch.clear();
        catsOFCurrentMatch.clear();
        idsOFCurrentMatch.clear();

        catsOFCurrentMatch.insert(0,((result_node *)reached_node)->get_previous_category_id());
        idsOFCurrentMatch.insert(0, ((result_node *)reached_node)->get_affix_id());
#ifdef REDUCE_THRU_DIACRITICS
        possible_raw_datasOFCurrentMatch.clear();
        possible_raw_datasOFCurrentMatch.insert(0,((result_node *)reached_node)->raw_datas);
#endif
        sub_positionsOFCurrentMatch.insert(0,position-1);
        letter_node * tree_head=(letter_node*)Tree->getFirstNode();
        node * current_parent=((result_node *)reached_node)->parent;

        int count=0;
        while (current_parent!=tree_head)
        {
                if (current_parent->isLetterNode())
                {
                        if (((letter_node* )current_parent)->getLetter()!='\0')
                                count++;
                }
                else
                {
                         catsOFCurrentMatch.insert(0,((result_node *)current_parent)->get_previous_category_id());
                         idsOFCurrentMatch.insert(0, ((result_node *)current_parent)->get_affix_id());//was : reached_node
                         sub_positionsOFCurrentMatch.insert(0, position-count-1);
#ifdef REDUCE_THRU_DIACRITICS
                         possible_raw_datasOFCurrentMatch.insert(0,((result_node *)current_parent)->raw_datas);
#endif
                }
                current_parent=current_parent->parent;
        }
#endif
}
TreeSearch::~TreeSearch() {}
bool TreeSearch::operator ()()
{
	queue.clear();
#ifdef QUEUE
	QList <long> catsOFCurrentMatch;
	QList<int> sub_positionsOFCurrentMatch;
	QList<long> idsOFCurrentMatch;
	QQueue<QList<int> > all_positions;
	QQueue<QList<long> > all_categories;
	QQueue<QList<long> > all_ids;
	all_categories.clear();
	all_positions.clear();
	all_ids.clear();
	all_categories.enqueue(catsOFCurrentMatch);
	all_positions.enqueue(sub_positionsOFCurrentMatch);
	all_ids.enqueue(idsOFCurrentMatch);
#ifdef REDUCE_THRU_DIACRITICS
	QList<QList< QString> > possible_raw_datasOFCurrentMatch;
	QQueue<QList<QList< QString> > > all_raw_datas;
	all_raw_datas.clear();
	all_raw_datas.enqueue(possible_raw_datasOFCurrentMatch);
#endif
#elif defined(PARENT)
	/*this->sub_positionsOFCurrentMatch.clear();
	this->catsOFCurrentMatch.clear();
	this->idsOFCurrentMatch.clear();*/
#endif
	queue.enqueue((letter_node*)Tree->getFirstNode());
	//show_queue_content();
#ifdef QUEUE
	QList  <int> temp_partition;
	QList  <long> temp_ids;
	QList <long> temp_categories;
#ifdef REDUCE_THRU_DIACRITICS
	QList <QList<QString> > temp_raw_datas;
#endif
#endif
	bool stop=false;
	int nodes_per_level=1;
	bool wait_for_dequeue=false;
	position=startingPos;
	while (!queue.isEmpty() && !stop)
	{
		if (wait_for_dequeue)
			position++;
		wait_for_dequeue=false;
		node * current_node=queue.dequeue();
#ifdef QUEUE
		sub_positionsOFCurrentMatch=all_positions.dequeue();
		catsOFCurrentMatch =all_categories.dequeue();
		idsOFCurrentMatch=all_ids.dequeue();
#ifdef REDUCE_THRU_DIACRITICS
		possible_raw_datasOFCurrentMatch=all_raw_datas.dequeue();
#endif
#endif
		nodes_per_level--;
		if (nodes_per_level==0)
		{
			wait_for_dequeue=true;
			nodes_per_level=queue.count();
		}
		//show_queue_content();
		QList<node *> current_children=current_node->getChildren();
		QChar current_letter=info->word[position];
		int num_children=current_children.count();
		for (int j=0;j<num_children;j++)
		{
			node *current_child=current_children[j];
			if (current_child->isLetterNode())
			{
				if(((letter_node*)current_child)->getLetter()==current_letter)
				{
					queue.enqueue((letter_node*)current_child);
#ifdef QUEUE
					temp_partition=sub_positionsOFCurrentMatch;
					temp_categories=catsOFCurrentMatch;
					temp_ids=idsOFCurrentMatch;
					all_positions.enqueue(temp_partition);
					all_categories.enqueue(temp_categories);
					all_ids.enqueue(temp_ids);
#ifdef REDUCE_THRU_DIACRITICS
					temp_raw_datas=possible_raw_datasOFCurrentMatch;
					all_raw_datas.enqueue(temp_raw_datas);
#endif
#endif
					if (wait_for_dequeue)
						nodes_per_level++;
					//show_queue_content();
				}
			}
			else
			{
#ifdef QUEUE
				temp_partition=sub_positionsOFCurrentMatch;
				temp_categories=catsOFCurrentMatch;
				temp_ids=idsOFCurrentMatch;
				temp_partition.append(position-1);
				temp_categories.append(((result_node *)current_child)->get_previous_category_id());
				temp_ids.append(((result_node *)current_child)->get_affix_id());
				this->sub_positionsOFCurrentMatch=temp_partition;
				this->catsOFCurrentMatch=temp_categories;
				this->idsOFCurrentMatch=temp_ids;
#ifdef REDUCE_THRU_DIACRITICS
				temp_raw_datas=possible_raw_datasOFCurrentMatch;
				temp_raw_datas.append(((result_node *)current_child)->raw_datas);
				this->possible_raw_datasOFCurrentMatch=temp_raw_datas;
#endif
#endif
#if defined(PARENT)
				reached_node=current_child;
#endif
				/*if (type==STEM)
					resulting_category_idOFCurrentMatch=((result_node *)current_child)->get_previous_category_id();
				else*/
					resulting_category_idOFCurrentMatch=((result_node *)current_child)->get_resulting_category_id();
				if (shouldcall_onmatch(position) && !(on_match_helper()))
				{
					stop=true;
					break;
				}
				else
				{
					QList<node *> result_node_children=current_child->getChildren();
					int num_result_children=result_node_children.count();
					for (int j=0;j<num_result_children;j++)
					{
						if(((letter_node*)result_node_children[j])->getLetter()==current_letter)
						{
							queue.enqueue((letter_node*)result_node_children[j]);
#ifdef QUEUE
							all_positions.enqueue(temp_partition);
							all_categories.enqueue(temp_categories);
							all_ids.enqueue(temp_ids);
#ifdef REDUCE_THRU_DIACRITICS
							all_raw_datas.enqueue(temp_raw_datas);
#endif
#endif
							if (wait_for_dequeue)
								nodes_per_level++;
							//show_queue_content();
						}
					}
				}
			 }
		}
	}
	return (!stop);
}
#ifdef REDUCE_THRU_DIACRITICS
void TreeSearch::get_all_possibilities(int i, QList< QString>  raw_datas)
{
	if (i>possible_raw_datasOFCurrentMatch.count() || i<0)
		return;
	else if (i==possible_raw_datasOFCurrentMatch.count())
	{
		if (!a_branch_returned_false)
		{
			raw_datasOFCurrentMatch=raw_datas;
			if (info->called_everything)
			{
				if (!onMatch())
					a_branch_returned_false=true;
			}
			else
			{
				if (!info->on_match_helper())
					a_branch_returned_false=true;
			}
		}
	}
	else
	{
		for (int j=0;j<possible_raw_datasOFCurrentMatch[i].count();j++)
		{
			if (!a_branch_returned_false)
			{
				QList< QString> raw_datas_modified=raw_datas;
				raw_datas_modified.append(possible_raw_datasOFCurrentMatch[i][j]);
				get_all_possibilities(i+1,raw_datas_modified);
			}
		}
	}
}
#endif
bool TreeSearch::on_match_helper()
{
	//out<<"p-S:"<<info->word.mid(startingPos)<<"\n";
	//check if matches with Diacritics
#ifdef REDUCE_THRU_DIACRITICS
#ifdef PARENT
	fill_details();
#endif
	int startPos=startingPos;
	//out<<"startPos="<<startPos<<"position-1="<<position-1<<"\n";
	QString subword=getDiacriticword(position-1,startPos,info->diacritic_word);
	for (int k=0;k<sub_positionsOFCurrentMatch.count();k++)
	{
		for (int j=0;j<possible_raw_datasOFCurrentMatch[k].count();j++)
		{
			//out<<"p-S:"<<subword<<"-"<<possible_raw_datasOFCurrentMatch[k][j]<<"\n";
			if (isDiacritic(possible_raw_datasOFCurrentMatch[k][j][0])) //in this case we can assume we are working in the suffix region
			{
				if (!equal(getDiacriticword(startPos-1,startPos-1,info->diacritic_word),QString("%1%2").arg(info->word[startPos-1]).arg(possible_raw_datasOFCurrentMatch[k][j][0])))
				{
					possible_raw_datasOFCurrentMatch[k].removeAt(j);
					j--;
					continue;
				}
				possible_raw_datasOFCurrentMatch[k][j]=possible_raw_datasOFCurrentMatch[k][j].mid(1);
			}
			//out<<"p-S:"<<subword<<"-"<<possible_raw_datasOFCurrentMatch[k][j]<<"\n";
			if (!equal(subword,possible_raw_datasOFCurrentMatch[k][j]))
			{
				possible_raw_datasOFCurrentMatch[k].removeAt(j);
				j--;
			}
		}

		startPos=sub_positionsOFCurrentMatch[k]+1;
	}
	//number_of_matches++;
	for (int i=0;i<possible_raw_datasOFCurrentMatch.count();i++)
		if (0==possible_raw_datasOFCurrentMatch[i].count())
			return true; //not matching, continue without doing anything
	QList<QString> raw_datas;
	raw_datas.clear();
	a_branch_returned_false=false;
	get_all_possibilities(0, raw_datas);
	return !a_branch_returned_false;
#else
	if (info->called_everything)
		return onMatch();
	else
		return info->on_match_helper();
	/*
	for (int i=0;i<positions.count();i++)
	{
		out<<positions[i]<<" "<< getColumn("category","name",cats[i])<<" ";
		//qDebug()<<positions[i]<<" "<< getColumn("category","name",cats[i])<<" ";
	}
	out <<"\n";
	return true;
	*/
#endif
}
TreeSearch::TreeSearch(item_types type,Stemmer* info,int position)
{
	this->info=info;
	this->type=type;
	startingPos=position;
	if (type==PREFIX)
		Tree=database_info.Prefix_Tree;
	else if (type==SUFFIX)
		Tree=database_info.Suffix_Tree;
	/*else if (type==STEM)
		Tree=database_info.Stem_Tree;*/
	//number_of_matches=0;
}


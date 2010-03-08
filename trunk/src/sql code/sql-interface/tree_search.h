#ifndef TREE_SEARCH_H
#define TREE_SEARCH_H

#include "tree.h"
#include "database_info.h"

class TreeSearch
{
	protected:
		tree* Tree;

		int startingPos;
		QString original_word;
		QList<int> sub_positionsOFCurrentMatch;
		QList<long> catsOFCurrentMatch;
		long resulting_category_idOFCurrentMatch;
		QQueue<letter_node *> queue;
		QQueue<QList<int> > all_positions;
		QQueue<QList<long> > all_categories;
		//int number_of_matches;
		virtual bool shouldcall_onmatch(int)//re-implemented in case of SUFFIX search tree
		{
			return true;
		}
		void traverse_text()
		{
			int position=startingPos;
			QList <long> catsOFCurrentMatch;
			QList<int> sub_positionsOFCurrentMatch;
			queue.clear();
			all_categories.clear();
			all_positions.clear();
			all_categories.enqueue(catsOFCurrentMatch);
			all_positions.enqueue(sub_positionsOFCurrentMatch);
			queue.enqueue((letter_node*)Tree->getFirstNode());
			//show_queue_content();
			QList  <int> temp_partition;
			QList <long> temp_categories;
			bool stop=false;
			int nodes_per_level=1;
			bool wait_for_dequeue=false;
			while (!queue.isEmpty() && !stop)
			{
				if (wait_for_dequeue)
					position++;
				wait_for_dequeue=false;
				node* current_node=queue.dequeue();
				sub_positionsOFCurrentMatch=all_positions.dequeue();
				catsOFCurrentMatch =all_categories.dequeue();
				nodes_per_level--;
				if (nodes_per_level==0)
				{
					wait_for_dequeue=true;
					nodes_per_level=queue.count();
				}
				//show_queue_content();
				QList<node *> current_children=current_node->getChildren();
				QChar current_letter=original_word[position];
				int num_children=current_children.count();
				for (int j=0;j<num_children;j++)
				{
					node *current_child=current_children[j];
					if (current_child->isLetterNode())
					{
						if(((letter_node*)current_child)->getLetter()==current_letter)
						{
							queue.enqueue((letter_node*)current_child);
							temp_partition=sub_positionsOFCurrentMatch;
							temp_categories=catsOFCurrentMatch;
							all_positions.enqueue(temp_partition);
							all_categories.enqueue(temp_categories);
							if (wait_for_dequeue)
								nodes_per_level++;
							//show_queue_content();
						}
					}
					else
					{
						temp_partition=sub_positionsOFCurrentMatch;
						temp_categories=catsOFCurrentMatch;
						temp_partition.append(position-1);
						temp_categories.append(((result_node *)current_child)->get_previous_category_id());
						resulting_category_idOFCurrentMatch=((result_node *)current_child)->get_resulting_category_id();
						this->sub_positionsOFCurrentMatch=temp_partition;
						this->catsOFCurrentMatch=temp_categories;
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
									all_positions.enqueue(temp_partition);
									all_categories.enqueue(temp_categories);
									if (wait_for_dequeue)
										nodes_per_level++;
									//show_queue_content();
								}
							}
						}
					 }
				}
				if (stop)
					return;
			}
		}
		inline bool on_match_helper()
		{
			//number_of_matches++;
			return onMatch();
			/*
			for (int i=0;i<positions.count();i++)
			{
				out<<positions[i]<<" "<< getColumn("category","name",cats[i])<<" ";
				//qDebug()<<positions[i]<<" "<< getColumn("category","name",cats[i])<<" ";
			}
			out <<"\n";
			return true;
			*/
		}
	public:

		TreeSearch(item_types type,QString original_word,int position)
		{
			//this->type=type;
			this->original_word=original_word;
			startingPos=position;
			if (type==PREFIX)
				Tree=database_info.Prefix_Tree;
			else if (type==SUFFIX)
				Tree=database_info.Suffix_Tree;
			//number_of_matches=0;
		}
		virtual void operator()()
		{
			//number_of_matches=0;
			traverse_text();
			//return number_of_matches;
		}
		virtual bool onMatch() = 0;// returns true to continue, false to abort
		virtual ~TreeSearch() {}
};
class SuffixSearch : public TreeSearch
{
	private:
		QList<int> pos_prefix;
		QList<long> cat_prefix;
		long prefix_cat_id;
		long stem_cat_id;
	public:
		SuffixSearch(QString original_word,int position,long prefix_cat_id,long stem_cat_id,QList<int> pos_prefix,QList<long> cat_prefix):TreeSearch(SUFFIX,original_word,position)
		{
			//print_tree();
			this->pos_prefix=pos_prefix;
			this->cat_prefix=cat_prefix;
			this->prefix_cat_id=prefix_cat_id;
			this->stem_cat_id=stem_cat_id;
		}
		virtual void operator()()
		{
			TreeSearch::operator()();
		}
		virtual bool shouldcall_onmatch(int position)//note that provided position is 1+last_letter
		{
			if (position==original_word.length())
				if (database_info.rules_AC->operator ()(prefix_cat_id,resulting_category_idOFCurrentMatch) && database_info.rules_BC->operator ()(stem_cat_id,resulting_category_idOFCurrentMatch))
					return true;
			return false;
		}
		virtual bool onMatch()
		{
			//out<<"S:"<<original_word.mid(startingPos)<<"\n";
			for (int i=0;i<pos_prefix.count();i++)
				out<<pos_prefix[i]<<" "<< getColumn("category","name",cat_prefix[i])<<" ";
			out <<"--- ";
			out<<startingPos-1<<" "<<getColumn("category","name",stem_cat_id)<<" --- ";
			for (int i=0;i<sub_positionsOFCurrentMatch.count();i++)
				out<<sub_positionsOFCurrentMatch[i]<<" "<< getColumn("category","name",catsOFCurrentMatch[i])<<" ";
			out <<"\n";
			//get additional info from database
			//display result somehow or call some function that does this
			return true;
		}
};

class StemSearch /*: public Trie*/ //just a proof of concept
{
	private:
		QList<int> pos_prefix;
		QList<long> cat_prefix;

		QString original_word;
		int currentMatchPos;
		long category_of_currentmatch;
		int starting_pos;
		long prefix_category;
	public:
		StemSearch(QString word,int pos,long prefix_category,QList<int> pos_prefix,QList<long> cat_prefix)
		{
			original_word=word;
			starting_pos=pos;
			this->prefix_category=prefix_category;
			this->pos_prefix=pos_prefix;
			this->cat_prefix=cat_prefix;
			/*QSqlQuery query(db);
			QString stmt=QString("SELECT id, name FROM stem");
			QString name;
			unsigned long long stem_id;
			bool ok;
			if (!execute_query(stmt,query))
				return ;
			while (query.next())
			{
				name=query.value(1).toString();
				stem_id=query.value(0).toLongLong(&ok);
				stems.append(name);
				stem_ids.append(stem_id);
			}*/

		}
		int operator()()
		{
			for (int i=starting_pos;i<=original_word.length();i++)
			{
				QString name=original_word.mid(starting_pos,i-starting_pos);
				long cat_id;
				Search_by_item s1(STEM,name);
				while(s1.retrieve(cat_id))
				{
					if (database_info.rules_AB->operator ()(prefix_category,cat_id))
					{
						category_of_currentmatch=cat_id;
						currentMatchPos=i-1;
						onMatch();
					}
				}
			}
			return 0;
		}
		bool onMatch()
		{
			//out<<"s:"<<original_word.mid(starting_pos,currentMatchPos-starting_pos+1)<<"\n";
			SuffixSearch Suffix(original_word,currentMatchPos+1,prefix_category,category_of_currentmatch,pos_prefix,cat_prefix);
			Suffix();
			return true;
		}
};
class PrefixSearch : public TreeSearch
{
	public:

		PrefixSearch(QString original_word):TreeSearch(PREFIX,original_word,0)
		{
			//print_tree();
		}
		virtual void operator()()
		{
			TreeSearch::operator()();
		}
		virtual bool onMatch()
		{
			//out<<"p:"<<original_word.mid(0,sub_positionsOFCurrentMatch.last()+1)<<"\n";
			StemSearch Stem(original_word,sub_positionsOFCurrentMatch.last()+1,resulting_category_idOFCurrentMatch,sub_positionsOFCurrentMatch,catsOFCurrentMatch);
			Stem();
			return true;
		}
};




#endif // TREE_SEARCH_H

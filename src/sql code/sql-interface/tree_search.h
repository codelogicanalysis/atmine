#ifndef TREE_SEARCH_H
#define TREE_SEARCH_H

#include "tree.h"
#include "database_info.h"


class Stemmer;

class TreeSearch
{
	public:
		int startingPos;
		QList<int> sub_positionsOFCurrentMatch;
		QList<long> catsOFCurrentMatch;
		QList<long> idsOFCurrentMatch;
		long resulting_category_idOFCurrentMatch;
	protected:
		tree* Tree;
		Stemmer* info;
		QQueue<letter_node *> queue;
#ifdef PARENT
		node * current_node;
#endif
		//int number_of_matches;

		virtual bool shouldcall_onmatch(int)//re-implemented in case of SUFFIX search tree
		{
			return true;
		}
		void traverse_text();
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
		TreeSearch(item_types type,Stemmer* info,int position);
		virtual void operator()()
		{
			//number_of_matches=0;
			traverse_text();
			//return number_of_matches;
		}
		void fill_details() //this function fills the public member functions such as QList<int> sub_positionsOFCurrentMatch & QList<long> catsOFCurrentMatch;
		{
#ifdef QUEUE
			//nothing needs to be done, members are already filled during traversals
#elif defined(PARENT)
                        catsOFCurrentMatch.insert(0,((result_node *)current_node)->get_previous_category_id());
                        idsOFCurrentMatch.insert(0, ((result_node *)current_node)->get_affix_id());
                        sub_positionsOFCurrentMatch.insert(0,position);
                        letter_node * tree_head=(letter_node*)Tree->getFirstNode();
                        node * current_parent=((result_node *)current_node)->parent;

                        int counter=0;
                        while (current_parent!=tree_head)
                        {
                            if (current_parent->isLetterNode())
                            {
                                count++;
                            }
                            else
                            {
                                 catsOFCurrentMatch.insert(0,((result_node *)current_parent)->get_previous_category_id());
                                 idsOFCurrentMatch.insert(0, ((result_node *)current_node)->get_affix_id());
                                 sub_positionsOFCurrentMatch.insert(0, position-count);

                            }
                            current_parent=current_parent->parent;

                        }
			//use 'current_node'
			//do the backward traversals to fill the function
#endif
		}
		virtual bool onMatch() = 0;// returns true to continue, false to abort
		virtual ~TreeSearch() {}
};
class SuffixSearch : public TreeSearch
{
	public:
		SuffixSearch(Stemmer * info,int position):TreeSearch(SUFFIX,info,position)
		{
			//print_tree();
		}
		virtual void operator()()
		{
			TreeSearch::operator()();
		}
		virtual bool shouldcall_onmatch(int position);
		virtual bool onMatch();
		~SuffixSearch(){}
};
class StemSearch /*: public Trie*/ //just a proof of concept
{
	public:
		int currentMatchPos;
		long category_of_currentmatch;
		long id_of_currentmatch;
		int starting_pos;
	private:
		Stemmer * info;
	public:
		StemSearch(Stemmer * info,int pos)
		{
			this->info=info;
			starting_pos=pos;
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
		int operator()();
		bool onMatch();
		~StemSearch(){	}
};
class PrefixSearch : public TreeSearch
{
	public:

		PrefixSearch(Stemmer * info):TreeSearch(PREFIX,info,0)
		{
			//print_tree();
		}
		virtual void operator()()
		{
			TreeSearch::operator()();
		}
		virtual bool onMatch();
		~PrefixSearch(){}
};
class Stemmer
{
	public:
		QString word;
		PrefixSearch* Prefix;
		StemSearch* Stem;
		SuffixSearch* Suffix;
		virtual bool on_match()
		{
			int count=0;
			int number=0;
			Prefix->fill_details();
			out<<"(";
			for (int i=0;i<Prefix->sub_positionsOFCurrentMatch.count();i++) //TODO: results with incorrect behaviour assuming more than 1 category works for any item
			{
				//out<<Prefix->sub_positionsOFCurrentMatch[i]<<" "<< getColumn("category","name",Prefix->catsOFCurrentMatch[i])<<" ";
				if (number>0)
					out<<" + ";
				number++;
				Search_by_item s(PREFIX,Prefix->idsOFCurrentMatch[i]);
				minimal_item_info prefix_info;
				while(s.retrieve(prefix_info))
				{
					if (prefix_info.category_id==Prefix->catsOFCurrentMatch[i])
					{
						if (count>0)
							out << " OR ";
						count++;
						out<</*Prefix->sub_positionsOFCurrentMatch[i]<<" "<<*/ prefix_info.description;
					}
				}
			}
			out <<")--(";

			//out<<Suffix->startingPos-1<<" "<<getColumn("category","name",Stem->category_of_currentmatch)<<" --- ";
			Search_by_item s(STEM,Stem->id_of_currentmatch);
			minimal_item_info stem_info;
			count=0;
			while(s.retrieve(stem_info))
			{
				if (stem_info.category_id==Stem->category_of_currentmatch)
				{
					if (count>0)
						out << " OR ";
					count++;
					out<</*Stem->startingPos-1<<" "<<*/ stem_info.description;
				}
			}
			out <<")--(";
			Suffix->fill_details();
			number=0;
			count=0;
			for (int i=0;i<Suffix->sub_positionsOFCurrentMatch.count();i++)
			{
				if (number>0)
					out<<" + ";
				number++;
				//out<<Suffix->sub_positionsOFCurrentMatch[i]<<" "<< getColumn("category","name",Suffix->catsOFCurrentMatch[i])<<" ";
				Search_by_item s(SUFFIX,Suffix->idsOFCurrentMatch[i]);
				minimal_item_info suffix_info;
				while(s.retrieve(suffix_info))
				{
					if (suffix_info.category_id==Suffix->catsOFCurrentMatch[i])
					{
						if (count>0)
							out << " OR ";
						count++;
						out<</*Suffix->sub_positionsOFCurrentMatch[i]<<" "<<*/ suffix_info.description;
					}
				}
			}
			out <<")\n";
			return true;
		}
		Stemmer(QString word)
		{
			this->word=word;
			Prefix=new PrefixSearch(this);
		}
		void start_stemming()
		{
			Prefix->operator ()();
		}
		~Stemmer()
		{
			if (Suffix)
				delete Suffix;
			if (Stem)
				delete Stem;
			if (Prefix)
				delete Prefix;
		}
};

void TreeSearch::traverse_text()
{
	int position=startingPos;
	QList <long> catsOFCurrentMatch;
	QList<int> sub_positionsOFCurrentMatch;
	QList<long> idsOFCurrentMatch;
	queue.clear();
#ifdef QUEUE
	QQueue<QList<int> > all_positions;
	QQueue<QList<long> > all_categories;
	QQueue<QList<long> > all_ids;
	all_categories.clear();
	all_positions.clear();
	all_ids.clear();
	all_categories.enqueue(catsOFCurrentMatch);
	all_positions.enqueue(sub_positionsOFCurrentMatch);
	all_ids.enqueue((idsOFCurrentMatch));
#endif
	queue.enqueue((letter_node*)Tree->getFirstNode());
	//show_queue_content();
#ifdef QUEUE
	QList  <int> temp_partition;
	QList  <long> temp_ids;
	QList <long> temp_categories;
#endif
	bool stop=false;
	int nodes_per_level=1;
	bool wait_for_dequeue=false;
	while (!queue.isEmpty() && !stop)
	{
		if (wait_for_dequeue)
			position++;
		wait_for_dequeue=false;
		node* current_node=queue.dequeue();
#ifdef QUEUE
		sub_positionsOFCurrentMatch=all_positions.dequeue();
		catsOFCurrentMatch =all_categories.dequeue();
		idsOFCurrentMatch=all_ids.dequeue();
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
#elif defined(PARENT)
				current_node=current_child;

#endif
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
#endif
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
TreeSearch::TreeSearch(item_types type,Stemmer* info,int position)
{
	this->info=info;
	startingPos=position;
	if (type==PREFIX)
		Tree=database_info.Prefix_Tree;
	else if (type==SUFFIX)
		Tree=database_info.Suffix_Tree;
	//number_of_matches=0;
}
bool SuffixSearch::shouldcall_onmatch(int position)//note that provided position is 1+last_letter
{
	if (position==info->word.length())
		if (database_info.rules_AC->operator ()(info->Prefix->resulting_category_idOFCurrentMatch,resulting_category_idOFCurrentMatch) && database_info.rules_BC->operator ()(info->Stem->category_of_currentmatch,resulting_category_idOFCurrentMatch))
			return true;
	return false;
}
bool SuffixSearch::onMatch()
{
	//out<<"S:"<<info->word.mid(startingPos)<<"\n";
	info->Suffix=this;
	return info->on_match();
}
int StemSearch::operator()()
{
	for (int i=starting_pos;i<=info->word.length();i++)
	{
		QString name=info->word.mid(starting_pos,i-starting_pos);
		long cat_id;
		Search_by_item s1(STEM,name);
		id_of_currentmatch=s1.ID();
		while(s1.retrieve(cat_id))
		{
			if (database_info.rules_AB->operator ()(info->Prefix->resulting_category_idOFCurrentMatch,cat_id))
			{
				category_of_currentmatch=cat_id;
				currentMatchPos=i-1;
				onMatch();
			}
		}
	}
	return 0;
}
bool StemSearch::onMatch()
{
	//out<<"s:"<<info->word.mid(starting_pos,currentMatchPos-starting_pos+1)<<"\n";
	info->Stem=this;
	SuffixSearch * Suffix= new SuffixSearch(info,currentMatchPos+1);
	Suffix->operator ()();
	return true;
}
bool PrefixSearch::onMatch()
{
	//out<<"p:"<<info->word.mid(0,sub_positionsOFCurrentMatch.last()+1)<<"\n";
	info->Prefix=this;
	StemSearch *Stem=new StemSearch(info,(sub_positionsOFCurrentMatch.count()>0?sub_positionsOFCurrentMatch.last()+1:0));
	Stem->operator ()();
	return true;
}

#endif // TREE_SEARCH_H

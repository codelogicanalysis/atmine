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
		long resulting_category_idOFCurrentMatch;
	protected:
		tree* Tree;
		Stemmer* info;
		QQueue<letter_node *> queue;
		QQueue<QList<int> > all_positions;
		QQueue<QList<long> > all_categories;
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
			for (int i=0;i<Prefix->sub_positionsOFCurrentMatch.count();i++)
				out<<Prefix->sub_positionsOFCurrentMatch[i]<<" "<< getColumn("category","name",Prefix->catsOFCurrentMatch[i])<<" ";
			out <<"--- ";
			out<<Suffix->startingPos-1<<" "<<getColumn("category","name",Stem->category_of_currentmatch)<<" --- ";
			for (int i=0;i<Suffix->sub_positionsOFCurrentMatch.count();i++)
				out<<Suffix->sub_positionsOFCurrentMatch[i]<<" "<< getColumn("category","name",Suffix->catsOFCurrentMatch[i])<<" ";
			out <<"\n";
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
			/*if (Suffix)
				delete Suffix;
			if (Stem)
				delete Stem;
			if (Prefix)
				delete Prefix;*/
		}
};

void TreeSearch::traverse_text()
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
	SuffixSearch Suffix(info,currentMatchPos+1);
	Suffix();
	return true;
}

bool PrefixSearch::onMatch()
{
	//out<<"p:"<<info->word.mid(0,sub_positionsOFCurrentMatch.last()+1)<<"\n";
	info->Prefix=this;
	StemSearch Stem(info,sub_positionsOFCurrentMatch.last()+1);
	Stem();
	return true;
}

#endif // TREE_SEARCH_H

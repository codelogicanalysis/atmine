#ifndef TREE_SEARCH_H
#define TREE_SEARCH_H

#include <QVector>
#include "tree.h"
#include "utilities.h"
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
#ifdef REDUCE_THRU_DIACRITICS
		QList<QString > raw_datasOFCurrentMatch;
	protected:
		bool a_branch_returned_false; //needed by get_all_possibilities() to stop when a false is retuned
		QList<QList <QString > > possible_raw_datasOFCurrentMatch;
		void get_all_possibilities(int i, QList<QString> raw_datas);
#endif
	protected:
		tree* Tree;
		Stemmer* info;
		item_types type;
		QQueue<letter_node *> queue;
#if defined(PARENT)
		node * reached_node;
#endif
		int position;//note that provided position is 1+last_letter after traversal
		//int number_of_matches;

		virtual bool shouldcall_onmatch(int)//re-implemented in case of SUFFIX search tree
		{
			return true;
		}
		inline bool on_match_helper();		
	public:
		TreeSearch(item_types type,Stemmer* info,int position);
		virtual bool operator()();
		void fill_details() //this function fills the public member functions such as QList<int> sub_positionsOFCurrentMatch & QList<long> catsOFCurrentMatch;
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
		/*virtual bool operator()()
		{
			return TreeSearch::operator()();
		}*/
		virtual bool shouldcall_onmatch(int position);
		virtual bool onMatch();
		~SuffixSearch()
		{
		}
};
class StemSearch /*: public Trie*/ //just a proof of concept
{
	public:
		int currentMatchPos;
		long category_of_currentmatch;
		long id_of_currentmatch;
		QString raw_data_of_currentmatch;
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
		bool operator()();
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
		/*virtual bool operator()()
		{
			TreeSearch::operator()();
		}*/
		virtual bool onMatch();
		~PrefixSearch(){}
};
class Stemmer
{
	protected:
		int total_matches_till_now;
		item_types type;
	public:
		QString word;
		QString diacritic_word;
		bool called_everything;
		PrefixSearch* Prefix;
		StemSearch* Stem;
		SuffixSearch* Suffix;

		virtual bool on_match_helper() //needed just to count matches till now
		{
			total_matches_till_now++;
			return on_match();
		}
		virtual bool on_match()
		{
			int count=0;
			int number=0;
			if (called_everything || type==PREFIX)
			{
#if !defined (REDUCE_THRU_DIACRITICS)
				Prefix->fill_details();
#endif
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
#ifdef REDUCE_THRU_DIACRITICS
						if (prefix_info.category_id==Prefix->catsOFCurrentMatch[i] && equal(prefix_info.raw_data,Prefix->raw_datasOFCurrentMatch[i]))
#else
						if (prefix_info.category_id==Prefix->catsOFCurrentMatch[i] )
#endif
						{
							if (count>0)
								out << " OR ";
							count++;
							out<</*Prefix->sub_positionsOFCurrentMatch[i]<<" "<<*/ prefix_info.description;
						}
					}
				}
				out <<")-";
			}
			minimal_item_info stem_info;
			if (called_everything || type==STEM)
			{
				out<< "-(";
				//out<<Suffix->startingPos-1<<" "<<getColumn("category","name",Stem->category_of_currentmatch)<<" --- ";
				Search_by_item s(STEM,Stem->id_of_currentmatch);
				count=0;
				while(s.retrieve(stem_info))
				{
#ifdef REDUCE_THRU_DIACRITICS
					if (stem_info.category_id==Stem->category_of_currentmatch && equal(stem_info.raw_data,Stem->raw_data_of_currentmatch))
#else
					if (stem_info.category_id==Stem->category_of_currentmatch)
#endif
					{
						if (count>0)
							out << " OR ";
						count++;
						out<</*Stem->startingPos-1<<" "<<*/ stem_info.description;
						out<<" [ ";
						for (unsigned int i=0;i<stem_info.abstract_categories.size();i++)
							if (stem_info.abstract_categories[i])
									out<<getColumn("category","name",abstract_category_ids[i])<< " ";
						out<<"]";
					}
				}
				out <<")-";
			}
			if (called_everything || type==SUFFIX)
			{
				out<< "-(";
#if !defined (REDUCE_THRU_DIACRITICS)
				Suffix->fill_details();
#endif
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
#ifdef REDUCE_THRU_DIACRITICS
						if (suffix_info.category_id==Suffix->catsOFCurrentMatch[i] && equal(suffix_info.raw_data,Suffix->raw_datasOFCurrentMatch[i]))
#else
						if (suffix_info.category_id==Suffix->catsOFCurrentMatch[i])
#endif
						{
							if (count>0)
								out << " OR ";
							count++;
							out<</*Suffix->sub_positionsOFCurrentMatch[i]<<" "<<*/ suffix_info.description;
						}
					}
				}
				out <<")";
			}
			out<<"\n";
			return true;
		}
		Stemmer(QString word)
		{
			this->diacritic_word=word;
			this->word=removeDiacritics(word);
			Prefix=new PrefixSearch(this);
			Stem=NULL;
			Suffix=NULL;
		}
		bool operator()()//if returns true means there was a match
		{
			called_everything=true;
			total_matches_till_now=0;
			Prefix->operator ()();
			return (total_matches_till_now>0);
		}
		bool operator()(item_types type)//if returns true means there was a match
		{
			called_everything=false;
			this->type=type;
			total_matches_till_now=0;
			if (type==PREFIX)
				Prefix->operator ()();
			else if (type==STEM)
				Stem->operator ()();
			else if (type==SUFFIX)
				Suffix->operator ()();
			return (total_matches_till_now>0);
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
	return info->on_match_helper();
}
bool StemSearch::operator()()
{
	bool false_returned=false;
	for (int i=starting_pos;i<=info->word.length();i++)
	{
		QString name=info->word.mid(starting_pos,i-starting_pos);
		//out<<name<<"\n";
		Search_by_item s1(STEM,name);
		id_of_currentmatch=s1.ID();
#ifdef REDUCE_THRU_DIACRITICS
		minimal_item_info inf;
		while(s1.retrieve(inf))
		{
			if (!false_returned)
			{
				if (database_info.rules_AB->operator ()(info->Prefix->resulting_category_idOFCurrentMatch,inf.category_id))
				{
					category_of_currentmatch=inf.category_id;
					raw_data_of_currentmatch=inf.raw_data;
					currentMatchPos=i-1;
					QString subword=getDiacriticword(i-1,starting_pos,info->diacritic_word);
					//out<<"subword:"<<subword<<"-"<<raw_data_of_currentmatch<<currentMatchPos<<"\n";
					if (equal(subword,raw_data_of_currentmatch))
					{
						//out<<"yes\n";
						if (info->called_everything)
						{
							if (!onMatch())
								false_returned=true;
						}
						else
						{
							if (!info->on_match_helper())
									false_returned=true;
						}
					}
				}
			}
		}
#else
		long cat_id;
		while(s1.retrieve(cat_id))
		{
			if (!false_returned)
			{
				if (database_info.rules_AB->operator ()(info->Prefix->resulting_category_idOFCurrentMatch,cat_id))
				{
					category_of_currentmatch=cat_id;
					currentMatchPos=i-1;
					if (info->called_everything)
					{
						if (!onMatch())
							false_returned=true;
					}
					else
					{
						if (!info->on_match_helper())
								false_returned=true;
					}
				}
			}
		}
#endif
	}
	return !false_returned;
}
bool StemSearch::onMatch()
{
	//out<<"s:"<<info->word.mid(starting_pos,currentMatchPos-starting_pos+1)<<"\n";
	info->Stem=this;
	SuffixSearch * Suffix= new SuffixSearch(info,currentMatchPos+1);
	return Suffix->operator ()();
}
bool PrefixSearch::onMatch()
{
	//out<<"p:"<<info->word.mid(0,position)<<"\n";
	info->Prefix=this;
	StemSearch *Stem=new StemSearch(info,position);
	return Stem->operator ()();
}

#endif // TREE_SEARCH_H

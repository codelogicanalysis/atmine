#ifndef TREE_SEARCH_H
#define TREE_SEARCH_H

#include "tree.h"

class TreeSearch: public tree
{
	protected:
		int startingPos;
		QString original_word;
		//int number_of_matches;

		QList<int> sub_positionsOFCurrentMatch;
		QList<long> catsOFCurrentMatch;
		long resulting_category_idOFCurrentMatch;

		virtual bool on_match_helper(QList<int> positions,QList<long> cats, long resulting_category_idOFCurrentMatch)
		{
			//number_of_matches++;
			sub_positionsOFCurrentMatch=positions;
			catsOFCurrentMatch=cats;
			this->resulting_category_idOFCurrentMatch=resulting_category_idOFCurrentMatch;
			return onMatch();
		}
	public:

		TreeSearch(item_types type):tree()
		{
			this->type=type;
			//number_of_matches=0;
		}
		void build_tree()
		{
			build_affix_tree(type);
		}
		virtual void operator()(QString original_word,int position)
		{
			this->original_word=original_word;
			startingPos=position;
			//number_of_matches=0;
			traverse_text(original_word,startingPos);
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
		SuffixSearch():TreeSearch(SUFFIX)
		{
			//print_tree();
		}
		virtual void operator()(QString original_word,int position,long prefix_cat_id,long stem_cat_id,QList<int> pos_prefix,QList<long> cat_prefix)
		{
			TreeSearch::operator()(original_word,position);
			this->pos_prefix=pos_prefix;
			this->cat_prefix=cat_prefix;
			this->prefix_cat_id=prefix_cat_id;
			this->stem_cat_id=stem_cat_id;
		}
		virtual bool shouldcall_onmatch(int position, long result_id)//note that provided position is 1+last_letter
		{
			if (position==original_word.length())
				if (areCompatible(AC,prefix_cat_id,result_id) && areCompatible(BC,stem_cat_id,result_id))
					return true;
			return false;
		}
		virtual bool onMatch()
		{
			out<<"S:"<<original_word.mid(startingPos)<<"\n";
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
SuffixSearch Suffix;
class StemSearch /*: public Trie*/
{
	private:
		QList<int> pos_prefix;
		QList<long> cat_prefix;

		QString original_word;
		//QList<QString> stems;//just a proof of concept
		//QList<long long> stem_ids;
		int currentMatchPos;
		long category_of_currentmatch;
		int starting_pos;
		long prefix_category;
	public:
		StemSearch()
		{
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
		int operator()(QString word,int pos,long prefix_category,QList<int> pos_prefix,QList<long> cat_prefix)
		{
			original_word=word;
			starting_pos=pos;
			this->prefix_category=prefix_category;
			this->pos_prefix=pos_prefix;
			this->cat_prefix=cat_prefix;
			for (int i=starting_pos;i<=original_word.length();i++)
			{
				QString name=original_word.mid(starting_pos,i-starting_pos);
				long cat_id;
				Search_by_item s1(STEM,name);
				while(s1.retrieve(cat_id))
				{
					if (areCompatible(AB,prefix_category,cat_id))
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
			out<<"s:"<<original_word.mid(starting_pos,currentMatchPos-starting_pos+1)<<"\n";
			Suffix(original_word,currentMatchPos+1,prefix_category,category_of_currentmatch,pos_prefix,cat_prefix);
			return true;
		}
};
StemSearch Stem;
class PrefixSearch : public TreeSearch
{
	public:

		PrefixSearch():TreeSearch(PREFIX)
		{
			print_tree();
		}
		virtual void operator()(QString original_word)
		{
			TreeSearch::operator()(original_word,0);
		}
		virtual bool onMatch()
		{
			out<<"p:"<<original_word.mid(0,sub_positionsOFCurrentMatch.last()+1)<<"\n";
			Stem(original_word,sub_positionsOFCurrentMatch.last()+1,resulting_category_idOFCurrentMatch,sub_positionsOFCurrentMatch,catsOFCurrentMatch);
			return true;
		}
};
PrefixSearch Prefix;




#endif // TREE_SEARCH_H

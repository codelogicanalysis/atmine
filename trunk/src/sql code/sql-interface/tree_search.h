#ifndef TREE_SEARCH_H
#define TREE_SEARCH_H

#include "tree.h"

class TreeSearch: public tree
{
	protected:
		int startingPos;
		QString original_word;
		int currentMatchPos;
		int number_of_matches;
		QList<long> catsOFCurrentMatch;

		virtual bool on_match_helper(int match_pos,QList<long> cats)//match_pos=position of last letter
		{
			number_of_matches++;
			currentMatchPos=match_pos;
			catsOFCurrentMatch=cats;
			return onMatch();
		}
	public:

		TreeSearch(item_types type, QString original_word,int position):tree()
		{
			build_affix_tree(type);
			original_word=original_word;
			startingPos=position;
			number_of_matches=0;
		}
		int operator()()
		{
			number_of_matches=0;
			traverse_text(original_word,startingPos);
			// TODO: also save the category and other info of the current match (access database)
			return number_of_matches;
		}
		/*int get_match_pos()
		{
			return currentMatchPos;
		}
		int get_number_of_matches()
		{
			return number_of_matches;
		}
		QList<long> get_categories_of_match()
		{
			return catsOFCurrentMatch;
		}*/
		virtual bool onMatch() = 0;// returns true to continue, false to abort
		bool isAffix(int pos)
		{
			//TODO: check if what is left of word is an affix (as a whole)
			return true;
		}
		virtual ~TreeSearch() {}
};

class SuffixSearch : public TreeSearch
{
	public:
		//....
		//i can access the whole data in TreeSearch...
		SuffixSearch(QString word, int pos):TreeSearch(SUFFIX,word,pos)
		{
		}
		virtual bool onMatch()
		{
			//do nothing I think
			return true;
		}
};

class StemSearch /*: public Trie*/
{
	private:
		QString original_word;
		int currentMatchPos;
	public:
		//....
		//i can access the whole data in TreeSearch...
		StemSearch(QString word,int pos)
		{

		}
		int operator()()
		{
			// TODO: do search also save the category and other info of the current match (access database)
			onMatch();
			return 0;
		}
		bool onMatch()
		{
			SuffixSearch sSrch(original_word,currentMatchPos+1);
			if (sSrch.isAffix(currentMatchPos+1))
			{
				//display result somehow or call some function that does this
			}
			return true;
		}
};

class PrefixSearch : public TreeSearch
{
	public:
		//....
		//i can access the whole data in TreeSearch...
		PrefixSearch(QString word):TreeSearch(PREFIX,word,0)
		{

		}
		virtual bool onMatch()
		{
			StemSearch sSrch(original_word,currentMatchPos+1);
			sSrch();
			return true;
		}
};




#endif // TREE_SEARCH_H

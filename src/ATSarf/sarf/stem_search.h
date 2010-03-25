#ifndef _STEM_SEARCH_H
#define	_STEM_SEARCH_H

//#include "trie.h"
#include "tree_search.h"
#include <QString>
class Stemmer;

class StemSearch :public TreeSearch/*: public Trie*/ //just a proof of concept
{
	public:
		int currentMatchPos;
		long category_of_currentmatch;
		long id_of_currentmatch;
		QString raw_data_of_currentmatch;
		int starting_pos;
	private:
		//Stemmer * info;
		//tree * Tree;
	public:
		StemSearch(Stemmer * info,int pos);
		virtual bool shouldcall_onmatch(int position);
		void fill_details();
		bool onMatch();
		~StemSearch();
};
#endif	/* _STEM_SEARCH_H */


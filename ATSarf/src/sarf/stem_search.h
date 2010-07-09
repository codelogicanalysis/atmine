#ifndef _STEM_SEARCH_H
#define	_STEM_SEARCH_H

#include "atmTrie.h"
#include "common.h"
#include <QString>
class Stemmer;

class StemSearch
{
	public:
		int currentMatchPos;
		long category_of_currentmatch;
		long id_of_currentmatch;
		QString raw_data_of_currentmatch;
		int starting_pos;
	private:
#ifdef USE_TRIE
		ATTrie * trie;
#endif
		Stemmer * info;
		bool stop;
	public:
		StemSearch(Stemmer * info,int pos);
		bool operator()();
#ifdef USE_TRIE_WALK
		bool check_for_terminal(int letter_index,ATTrie::Position pos);
#endif
		void traverse(int letter_index,ATTrie::Position pos);
		bool on_match_helper(int last_letter_index,Search_StemNode s1);
		bool onMatch();
		~StemSearch();
};

#endif	/* _STEM_SEARCH_H */


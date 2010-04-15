#ifndef _STEM_SEARCH_H
#define	_STEM_SEARCH_H

#include "../common_structures/atmTrie.h"
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
		ATTrie * trie;
		Stemmer * info;
	public:
		StemSearch(Stemmer * info,int pos);
		bool operator()();
		bool onMatch();
		~StemSearch();
};

#endif	/* _STEM_SEARCH_H */


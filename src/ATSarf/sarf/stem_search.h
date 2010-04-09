#ifndef _STEM_SEARCH_H
#define	_STEM_SEARCH_H

#include "atmTrie.h"
#include <QString>
class Stemmer;

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
		StemSearch(Stemmer * info,int pos);
		bool operator()();
		bool onMatch();
		~StemSearch();
};

#endif	/* _STEM_SEARCH_H */


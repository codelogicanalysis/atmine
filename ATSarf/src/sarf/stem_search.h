#ifndef _STEM_SEARCH_H
#define	_STEM_SEARCH_H

#include "atmTrie.h"
#include "common.h"
#include "database_info_block.h"
#include <QString>

class StemSearch
{
	public:
		int currentMatchPos;
		long category_of_currentmatch;
		long id_of_currentmatch;
		QString raw_data_of_currentmatch;
		int starting_pos;
	protected:
		text_info info;
	private:
#ifdef USE_TRIE
		ATTrie * trie;
#endif
		bool stop;
		bool reduce_thru_diacritics;
	public:
		StemSearch(QString * text,int start,bool reduce_thru_diacritics=false)
		{
			info.text=text;
			starting_pos=start;
			this->reduce_thru_diacritics=reduce_thru_diacritics;
		#ifdef USE_TRIE
			trie=database_info.Stem_Trie;
		#endif
		}
		bool operator()()
		{
			ATTrie::Position pos = trie->startWalk();
			stop=false;
			traverse(starting_pos,pos);
			trie->freePosition(pos);
			return !stop;
		}
#ifdef USE_TRIE_WALK
		bool check_for_terminal(int letter_index,ATTrie::Position pos);
#endif
		void traverse(int letter_index,ATTrie::Position pos);
		bool on_match_helper(int last_letter_index,Search_StemNode & s1);
		virtual bool shouldcall_onmatch(int)
		{
			return true;
		}
		virtual bool onMatch()=0;
		~StemSearch(){}
};

#endif	/* _STEM_SEARCH_H */


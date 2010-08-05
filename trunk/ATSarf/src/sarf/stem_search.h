#ifndef _STEM_SEARCH_H
#define	_STEM_SEARCH_H

#include "atmTrie.h"
#include "common.h"
#include "database_info_block.h"
#include <QString>

class StemSearch
{
	public:
		long prefix_category;
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
		StemSearch(QString * text,int start, long prefix_category,bool reduce_thru_diacritics=false)
		{
			info.text=text;
			starting_pos=start;
			this->prefix_category=prefix_category;
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
		bool isPrefixStemCompatible() const
		{//check rules AB
			compatibility_rules * cr= database_info.comp_rules;
			qDebug()<<"at check:"<<prefix_category;
			return ((*cr)(prefix_category,category_of_currentmatch));
		}
		virtual bool onMatch()=0;
		~StemSearch(){}
		/*static StemSearch * createStemSearch(QString * text,int start, long prefix_category)
		{
			StemSearch * me =new StemSearch(text,start);
			me->prefix_category=prefix_category;
			return me;
		}*/
};

#endif	/* _STEM_SEARCH_H */


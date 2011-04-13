#ifndef _STEM_SEARCH_H
#define	_STEM_SEARCH_H

#include "atmTrie.h"
#include "common.h"
#include "database_info_block.h"
#include "solution_position.h"
#include <QString>

class Stemmer;

class StemSearch
{
	public:
		long prefix_category;
		int currentMatchPos;
		long category_of_currentmatch;
		long id_of_currentmatch;
		minimal_item_info * solution;
	protected:
		text_info info;
		friend class Stemmer;
	private:
#ifdef USE_TRIE
		ATTrie * trie;
#endif
		multiply_params multi_p;
		bool stop;
		bool reduce_thru_diacritics;
		QVector<QString> possible_raw_datas;
	public:
		StemSearch(QString * text,int start, long prefix_category,bool reduce_thru_diacritics=true)
		{
			info.text=text;
			info.start=start;
			this->prefix_category=prefix_category;
			this->reduce_thru_diacritics=reduce_thru_diacritics;
		#ifdef USE_TRIE
			trie=database_info.Stem_Trie;
		#endif
			solution=NULL;
			multi_p=M_ALL;
		#if 0
			multi_p.description=false;
			multi_p.raw_data=false;
			multi_p.POS=false;
		#endif
		}
		bool operator()()
		{
			ATTrie::Position pos = trie->startWalk();
			stop=false;
			traverse(info.start,pos);
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
			return ((*cr)(prefix_category,category_of_currentmatch));
		}
		virtual bool onMatch()=0;
		~StemSearch(){}
		void setSolutionSettings(multiply_params params)
		{
			multi_p=params;
		}
	#ifdef MULTIPLICATION
		solution_position * computeFirstSolution();
		bool computeNextSolution(solution_position * current);//compute next posibility
	#endif
};

#endif	/* _STEM_SEARCH_H */


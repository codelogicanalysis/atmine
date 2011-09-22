#ifndef _SUFFIX_SEARCH_H
#define	_SUFFIX_SEARCH_H

#include "tree_search.h"

class SuffixSearch : public TreeSearch
{
    public:
		long prefix_category;
		long stem_category;
		SuffixSearch(QString * text,int start, long prefix_category, long stem_category):TreeSearch(SUFFIX,text,start) {
			this->prefix_category=prefix_category;
			this->stem_category=stem_category;
		}
		virtual bool onMatch()=0;
		virtual ~SuffixSearch(){}
		bool isPrefixStemSuffixCompatible() const {//check rules AC, BC
			compatibility_rules * cr= database_info.comp_rules;
			return ((*cr)(prefix_category,resulting_category_idOFCurrentMatch)) && ((*cr)(stem_category,resulting_category_idOFCurrentMatch));
		}
		virtual bool shouldcall_onmatch_ex(int position) {
			if (!isPrefixStemSuffixCompatible())
				return false;
			return shouldcall_onmatch(position);
		}
		virtual bool operator ()() {
			return TreeSearch::operator ()();
		}

		/*static SuffixSearch * createSuffixSearch(QString * text,int start, )
		{
			SuffixSearch * me =new SuffixSearch(text,start);
			me->prefix_category=prefix_category;
			me->stem_category=stem_category;
			return me;
		}*/
};

#endif	/* _SUFFIX_SEARCH_H */


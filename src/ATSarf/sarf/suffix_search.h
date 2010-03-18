#ifndef _SUFFIX_SEARCH_H
#define	_SUFFIX_SEARCH_H

#include "ATtree_search.h"

class Stemmer;

class SuffixSearch : public TreeSearch
{
    public:
        SuffixSearch(Stemmer * info,int position);
        virtual bool shouldcall_onmatch(int position);
        virtual bool onMatch();
		virtual ~SuffixSearch();
};

#endif	/* _SUFFIX_SEARCH_H */


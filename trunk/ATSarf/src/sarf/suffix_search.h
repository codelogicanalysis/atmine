#ifndef _SUFFIX_SEARCH_H
#define	_SUFFIX_SEARCH_H

#include "tree_search.h"

class SuffixSearch : public TreeSearch
{
    public:
		SuffixSearch(QString * text,int start);
		virtual bool onMatch()=0;
		virtual ~SuffixSearch();
};

#endif	/* _SUFFIX_SEARCH_H */


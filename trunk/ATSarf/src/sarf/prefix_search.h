#ifndef _PREFIX_SEARCH_H
#define	_PREFIX_SEARCH_H

#include "tree_search.h"

class PrefixSearch : public TreeSearch
{
    public:
		PrefixSearch(QString * text,int start);
		virtual bool onMatch()=0;
		virtual ~PrefixSearch();
};

#endif	/* _PREFIX_SEARCH_H */


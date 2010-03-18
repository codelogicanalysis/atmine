#ifndef _PREFIX_SEARCH_H
#define	_PREFIX_SEARCH_H

#include "ATtree_search.h"

class Stemmer;

class PrefixSearch : public TreeSearch
{
    public:
        PrefixSearch(Stemmer * info);
        virtual bool onMatch();
		virtual ~PrefixSearch();
};

#endif	/* _PREFIX_SEARCH_H */


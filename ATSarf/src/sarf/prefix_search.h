#ifndef _PREFIX_SEARCH_H
#define	_PREFIX_SEARCH_H

#include "tree_search.h"

class PrefixSearch : public TreeSearch
{
    public:
		virtual bool isPrefix() const{ return true;}
		PrefixSearch(QString * text,int start):TreeSearch(PREFIX,text,start) { }
		virtual bool onMatch()=0;
		virtual ~PrefixSearch(){}
		/*static PrefixSearch * createPrefixSearch(QString * text,int start)
		{
			return new PrefixSearch(text,start);
		}*/
};

#endif	/* _PREFIX_SEARCH_H */


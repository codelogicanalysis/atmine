#ifndef _PREFIX_ENUMERATE_H
#define	_PREFIX_ENUMERATE_H

#include "tree_enumerate.h"

class PrefixEnumerate : public TreeEnumerate
{
    public:
                virtual bool isPrefix() const{ return true;}

                PrefixEnumerate():TreeEnumerate(PREFIX) { }

                virtual bool onMatch()=0;
                virtual ~PrefixEnumerate(){}
};

#endif	/* _PREFIX_ENUMERATE_H */

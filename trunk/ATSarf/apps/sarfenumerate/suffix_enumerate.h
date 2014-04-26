#ifndef _SUFFIX_ENUMERATE_H
#define	_SUFFIX_ENUMERATE_H

#include "tree_enumerate.h"

class SuffixEnumerate : public TreeEnumerate
{
    public:
                /// Specify the category of the prefix
                long prefix_category;

                /// Specify the category of the stem
                long stem_category;

                SuffixEnumerate(long prefix_category, long stem_category):TreeEnumerate(SUFFIX) {

                        /// Setting the prefix_category to the input category
                        this->prefix_category=prefix_category;

                        /// Setting the stem category to the input category
                        this->stem_category=stem_category;
                }

                virtual bool onMatch()=0;

                virtual ~SuffixEnumerate(){}

                bool isPrefixStemSuffixCompatible() const {//check rules AC, BC
                        compatibility_rules * cr= database_info.comp_rules;
                        return ((*cr)(prefix_category,resulting_category_idOFCurrentMatch)) && ((*cr)(stem_category,resulting_category_idOFCurrentMatch));
                }

                virtual bool shouldcall_onmatch_ex() {
                        if (!isPrefixStemSuffixCompatible())
                                return false;
                        return true;
                }

                virtual bool operator ()() {
                        return TreeEnumerate::operator ()();
                }
};

#endif	/* _SUFFIX_ENUMERATE_H */

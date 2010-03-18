#ifndef _STEMMER_H
#define	_STEMMER_H

#include "prefix_search.h"
#include "stem_search.h"
#include "suffix_search.h"
#include "../common_structures/common.h"

class Stemmer
{
    protected:
        int total_matches_till_now;
        item_types type;
    public:
        QString word;
        QString diacritic_word;
        bool called_everything;
        PrefixSearch* Prefix;
        StemSearch* Stem;
        SuffixSearch* Suffix;

        virtual bool on_match_helper(); //needed just to count matches till now
        virtual bool on_match();
        Stemmer(QString word);
        bool operator()();//if returns true means there was a match
        bool operator()(item_types type);//if returns true means there was a match
		virtual ~Stemmer();
};

#endif	/* _STEMMER_H */


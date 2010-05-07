#ifndef _STEMMER_H
#define	_STEMMER_H

#include "prefix_search.h"
#include "stem_search.h"
#include "suffix_search.h"
#include "../common_structures/common.h"
#include <QVector>

class Stemmer
{
    protected:
        item_types type;
		bool get_all_details;
		//for use in on_match()
		QVector<minimal_item_info> * prefix_infos;
		minimal_item_info * stem_info;
		QVector<minimal_item_info> * suffix_infos;
    public:
		QString *text;
		QString *diacritic_text;
		long long start,finish;
        bool called_everything;
        PrefixSearch* Prefix;
        StemSearch* Stem;
        SuffixSearch* Suffix;

        virtual bool on_match_helper(); //needed just to count matches till now
        virtual bool on_match();
		Stemmer(QString *word,int start,bool get_all_details=true);
        bool operator()();//if returns true means there was a match
        bool operator()(item_types type);//if returns true means there was a match
		virtual ~Stemmer();
};

#endif	/* _STEMMER_H */


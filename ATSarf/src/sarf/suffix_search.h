/**
  * @file   suffix_search.h
  * @brief  this header file contains the definition of the SuffixSearch class which searches for the possible suffixes in an input word
  * @author Jad Makhlouta
  */
#ifndef _SUFFIX_SEARCH_H
#define	_SUFFIX_SEARCH_H

#include "tree_search.h"

/**
  * @class  SuffixSearch
  * @author Jad Makhlouta
  * @brief  This class implements the functionalities searching for the possible suffixes of an input word. To do so, it
  * inherets from the class TreeSearch.
  */
class SuffixSearch : public TreeSearch
{
    public:
                /// Specify the category of the prefix
                long prefix_category;

                /// Specify the category of the stem
                long stem_category;

                /**
                  * This method implements the constructor of the SuffixSearch class
                  * @param  text input string of the user to be processed
                  * @param  start   index of the input string indicating where to start searching for the suffix from
                  * @param  prefix_category indicates the category of the possible prefix found for compatibility testing
                  * @param  stem_category   indicates the category of the possible stem found for compatibility testing
                  */
		SuffixSearch(QString * text,int start, long prefix_category, long stem_category):TreeSearch(SUFFIX,text,start) {

                        /// Setting the prefix_category to the input category
                        this->prefix_category=prefix_category;

                        /// Setting the stem category to the input category
                        this->stem_category=stem_category;
		}

                /**
                  * This method implements the function that is triggered upon finiding a suffix match, and it is declared as virtual
                  * so it must be implemented by the programmer upon inhereting from this class.
                  */
		virtual bool onMatch()=0;

                /**
                  * This method defines the destructor of the SuffixSearch class
                  */
		virtual ~SuffixSearch(){}

                /**
                  * This method checks if the triplet prefix-stem-suffix is compatible based on compatibility rules
                  * extracted from arabic morphological rules.
                  * @return It returns true if the triplet is compatible, else returns false.
                  */
		bool isPrefixStemSuffixCompatible() const {//check rules AC, BC
                        /** This function initializes an instance of the compatibility_rules class, then checks the
                          * compatibility of current suffix match with prefix and stem and returns the product of the
                          * output of both tests.
                          */
                        compatibility_rules * cr= database_info.comp_rules;
                        return ((*cr)(prefix_category,resulting_category_idOFCurrentMatch)) && ((*cr)(stem_category,resulting_category_idOFCurrentMatch));
		}

                /**
                  * This method triggers the compatibility function upon finding a candidate match.
                  * @return It returns false if compatibility test is false, else returns true -- ask about!?!?!
                  */
		virtual bool shouldcall_onmatch_ex(int position) {
			if (!isPrefixStemSuffixCompatible())
				return false;
			return shouldcall_onmatch(position);
		}

                /**
                  * This function implements the bracket operator where it fires the TreeSearch bracket operator as well
                  * @return It returns a boolean refering to the output of the bracket operator function of TreeSearch
                  */
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

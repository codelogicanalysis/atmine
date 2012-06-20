/**
  * @file   prefix_search.h
  * @brief  this header file contains the definition of the PrefixSearch class which searches for the possible prefixes in an input word
  * @author Jad Makhlouta
  */
#ifndef _PREFIX_SEARCH_H
#define	_PREFIX_SEARCH_H

#include "tree_search.h"

/**
  * @class  PrefixSearch
  * @author Jad Makhlouta
  * @brief  This class implements the functionalities searching for the possible prefixes of an input word. To do so, it
  * inherets from the class TreeSearch.
  */
class PrefixSearch : public TreeSearch
{
    public:
                /**
                  * This method checks whether a string is a prefix or not
                  * @return The method returns true if this string is a prefix, else returns false
                  */
		virtual bool isPrefix() const{ return true;}

                /**
                  * This method implements the constructor of the PrefixSearch class
                  * @param  text pointer to the input string to be processed
                  * @param  start   index of the input to start processing from
                  */
		PrefixSearch(QString * text,int start):TreeSearch(PREFIX,text,start) { }

                /**
                  * This method in called when we have a prefix match, however it is defined as virtual and the implementation is
                  * done in the inhereting class PrefixMachine.
                  * @return This method returns a boolean with true value if a match is found else false.
                  */
		virtual bool onMatch()=0;
                /**
                  * This method defines the destructor of the PrefixSearch class
                  */
		virtual ~PrefixSearch(){}
		/*static PrefixSearch * createPrefixSearch(QString * text,int start)
		{
			return new PrefixSearch(text,start);
		}*/
};

#endif	/* _PREFIX_SEARCH_H */

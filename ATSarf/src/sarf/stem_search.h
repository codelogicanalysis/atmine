/**
  * @file   stem_search.h
  * @brief  this header file contains the definition of the StemSearch class which extracts the possible stems for an input string
  * @author Jad Makhlouta
  */
#ifndef STEM_SEARCH_H
#define STEM_SEARCH_H

#include "atmTrie.h"
#include "common.h"
#include "database_info_block.h"
#include "solution_position.h"
#include <QString>

class Stemmer;

/**
  * @class  StemSearch
  * @author Jad Makhlouta
  * @brief  This class implements the required functionalities in order to extract the possible stems
  */
class StemSearch {
    public:
        /// Specifies the category of the possible prefix extracted
        long prefix_category;

        /// Specifies the position of the current match
        int currentMatchPos;

        /// Holds the category of the stem possible match
        long category_of_currentmatch;

        /// Holds the id of the node stem possible match
        long id_of_currentmatch;

        /// This structure holds the information of the possible stem solution found
        minimal_item_info *solution;
        //protected:
        /// This type holds the input string from the user with the start/finish index specifying stem search starting index
        text_info info;
        friend class Stemmer;
    private:
        /// Delcares a pointer to the trie structure that stores the root arabic words in the lexer
        ATTrie *trie;
        multiply_params multi_p;
        bool stop;

        /// Boolean indicating whether to reduce the possible solutions using diacritics
        bool reduce_thru_diacritics;
        QVector<QString> possible_raw_datas;
    public:
        /**
          * This method implements the constructor of the StemSearch class
          * @param  text    pointer to the input string to be analyzed
          * @param  start   integer to indicate the start index to search for the stem from where the previous part is a possible prefix
          * @param  prefix_category indicates the category of the possible match prefix
          * @param  reduce_thru_diacritics  boolean indicating the use of diacritics to reduce possible stems which is initially set to true
          */
        StemSearch(QString *text, int start, long prefix_category, bool reduce_thru_diacritics = true) {
            info.text = text;
            info.start = start;
            this->prefix_category = prefix_category;
            this->reduce_thru_diacritics = reduce_thru_diacritics;
            // Setting the trie pointer to the databased extracted data
            trie = database_info.Stem_Trie;
            solution = NULL;
            multi_p = M_ALL;
        }

        /**
          * This method implements the bracket operator for the stem_search starting the stem search routine
          * @return returns a true/false value
          */
        bool operator()() {
            ATTrie::Position pos = trie->startWalk();
            stop = false;
            traverse(info.start, pos);
            trie->freePosition(pos);
            return !stop;
        }
        bool check_for_terminal(int letter_index, ATTrie::Position pos);
        void traverse(int letter_index, ATTrie::Position pos);
        bool on_match_helper(int last_letter_index, Search_StemNode &s1);

        /**
          * This method checks the compatibility between the prefix and stem found
          * @return It returns true if the prefix and stem are compatible, else false
          */
        bool isPrefixStemCompatible() const {
            //check rules AB
            compatibility_rules *cr = database_info.comp_rules;
            return ((*cr)(prefix_category, category_of_currentmatch));
        }
        virtual bool onMatch() = 0;

        /**
          * This method implements the destructor of the StemSearch class
          */
        ~StemSearch() {
            if (solution != NULL) {
                delete solution;
            }
        }

        /**
          * This method sets the settings of the solutions that we require the stem_search to return
          */
        void setSolutionSettings(multiply_params params) {
            multi_p = params;
        }
        #ifdef MULTIPLICATION
        solution_position *computeFirstSolution();
        bool computeNextSolution(solution_position *current); //compute next posibility
        #endif
};

#endif 

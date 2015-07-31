/**
  * @file tree_search.h
  * @author Jad Makhoulta and documented by Ameen Jaber
  * @brief  This file includes the declaration of the TreeSearch class which is the base class for prefix
  * and suffix search
  */
#ifndef _TREE_SEARCH_H
#define _TREE_SEARCH_H

#include <assert.h>
#include <QVector>
#include <QString>
#include <QList>
#include <QQueue>
#include "tree.h"
#include "database_info_block.h"
#include "common.h"
#include "node_info.h"
#include "solution_position.h"
#include "node.h"
#include "diacritics.h"
//#include "Search_by_item_locally.h"

class Stemmer;

/**
  * @class  TreeSearch
  * @author Jad Makhlouta
  * @brief  This class implements the structure and functions used for affix matching
  */
class TreeSearch {
    public://private
        bool filled_details;

        /// This structure holds the settings required for the final result returned
        multiply_params multi_p;
        QList<int> sub_positionsOFCurrentMatch;//end of last split
        /// Vector containing the affix solutions found
        AffixSolutionVector affix_info;

        /// List of the result nodes of the affix solutions found
        QList<result_node *> *result_nodes;
    private:
        /// Pointer to a triplet hash table that takes id/category/raw_data and returns abstract/description/POS of an affix solution
        ItemCatRaw2AbsDescPosMap *map;
        friend class Stemmer;
    public:
        /// Holds the category_id of the current affix match
        long resulting_category_idOFCurrentMatch;
    public:
        /**
          * This method checks if the obtanined solution is a prefix
          * @return Returns true if solution is a prefix, else returns false
          */
        virtual bool isPrefix() const {
            return false;
        }

        /**
          * This method returns a reference to the affix solution vector
          * @return Returns a reference to the affix solution vector
          */
        const AffixSolutionVector &getSolution() const {
            return affix_info;   //make sure affix_info is not null!!
        }

        /**
          * This method returns the previous node of a current input node in the trie structure
          * @param  current Pointer to a node_info structure
          * @return This method returns a node_info pointer to the previous node
          */
        node_info *previousNode(node_info *current) { //seems to have memory leak but not used
            node *head = Tree->getFirstNode();

            if (current->node == NULL) {
                return NULL;
            }

            node_info *previous = new node_info;
            //previous->node=current->node->getPreviousResultNode();
            int pos_in_tree = current->pos_in_tree, finish = current->start, start = current->start;
            //pos_in_tree-=(current->finish-current->start+1);
            node *pre = current->node;
            pre = pre->getPrevious();

            while (pos_in_tree >= -1 && pre != head && pre->isLetterNode()) {
                pos_in_tree--;
                start--;
                pre = pre->getPrevious();

                //check if diacritic and ignore
                while (start >= 0 && isDiacritic(info.text->at(start))) {
                    start--;
                }

                assert(start >= 0);
            }

            if (pos_in_tree == -1) {
                assert(pre == head && start == info.start);
            }

            previous->start = start;
            previous->finish = finish;
            previous->pos_in_tree = pos_in_tree;
            previous->node = (result_node *)pre;
            assert(start >= -1);
            assert(start <= finish);
            return previous;
        }
        node_info *lastNode() { //seems to have memory leak but not used
            node_info *last = new node_info;
            last->node = reached_node->getPreviousResultNode();
            last->pos_in_tree = position - 1;
            last->finish = info.finish;
            last->start = (sub_positionsOFCurrentMatch.size() > 1 ? sub_positionsOFCurrentMatch[sub_positionsOFCurrentMatch.size() -
                           2] : info.start);
            return last;
        }
        QList<result_node *> *getSplitList() { //seems to have memory leak
            QList<result_node *> *list = new QList<result_node *>();
            result_node *node = reached_node;

            while (node != NULL) {
                list->prepend(node);
                node = node->getPreviousResultNode();
            }

            return list;
        }
        QList<int> &getSplitPositions() {
            return sub_positionsOFCurrentMatch;
        }
        long getFinalResultingCategory() {
            return resulting_category_idOFCurrentMatch;
        }
    private:
        void initializeAffixInfo(solution_position *sol_pos,
                                 int start_index); //zero and initialize solutions till 'last_index' exclusive
        bool increment(solution_position *info, int index);
    public:
        solution_position *computeFirstSolution() {
            if (type == PREFIX) {
                map = database_info.map_prefix;
            } else if (type == SUFFIX) {
                map = database_info.map_suffix;
            }

            affix_info.clear();

            if (result_nodes != NULL) {
                delete result_nodes;
            }

            result_nodes = getSplitList();
            solution_position *first = new solution_position();
            initializeAffixInfo(first, 0);
            //first->print_positions();
            return first;
        }
        bool computeNextSolution(solution_position *current) { //compute next posibility
            bool val = increment(current, sub_positionsOFCurrentMatch.count() - 1);
            //current->print_positions();
            return val;
        }
        QList<QList <RawData > > possible_raw_datasOFCurrentMatch;
        tree *Tree;

        /// variable of type text_info that holds input text information
        text_info info;

        /// Holds the type of the required solution search
        item_types type;
        result_node *reached_node;
        int position;//note that provided position is 1+last_letter after traversal

        /// boolean to specify whether reduction trhough diacritics is required
        bool reduce_thru_diacritics;
        virtual bool shouldcall_onmatch_ex(int) {
            return true;
        }
        virtual bool shouldcall_onmatch(int) {
            return true;
        }
        bool on_match_helper();

        /**
          * This method sets the settings for the solution required to be extracted
          * @param  params
          */
        void setSolutionSettings(multiply_params params) {
            multi_p = params;
        }
    public:
        /**
          * This method implements the constructor of the TreeSearch class
          * @param  type    Specifies the type of solution required being prefix, or suffix
          * @param  text    Pointer to the input text
          * @param  start   Specify the starting index
          * @param  reduce_thru_diacritics  boolean to whether a solution reduced using diacretics is required
          * or not. It is initially set to true
          */
        TreeSearch(item_types type, QString *text, int start, bool reduce_thru_diacritics = true) {
            info.text = text;
            this->type = type;
            info.start = start;
            info.finish = start;
            this->reduce_thru_diacritics = reduce_thru_diacritics;

            // load the tree structure based on the input type whether it is a prefix or suffix
            if (type == PREFIX) {
                Tree = database_info.Prefix_Tree;
            } else if (type == SUFFIX) {
                Tree = database_info.Suffix_Tree;
            }

            multi_p = M_ALL;
            result_nodes = NULL;
        }
        virtual bool operator()();
        /*virtual*/ void
        fill_details(); //this function fills the public member functions such as QList<int> sub_positionsOFCurrentMatch & QList<long> catsOFCurrentMatch;
        virtual bool onMatch() = 0;// returns true to continue, false to abort
        virtual ~TreeSearch() {

            if (result_nodes != NULL) {
                delete result_nodes;
            }

        }
};

#endif // TREE_SEARCH_H

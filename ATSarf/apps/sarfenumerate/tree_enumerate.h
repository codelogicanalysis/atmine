#ifndef TREE_ENUMERATE_H
#define TREE_ENUMERATE_H

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

class Enumerator;

class TreeEnumerate {
    public://private
        bool filled_details;

        /// This structure holds the settings required for the final result returned
        multiply_params multi_p;

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

        virtual bool isPrefix() const {
            return false;
        }

        const AffixSolutionVector &getSolution() const {
            return affix_info;   //make sure affix_info is not null!!
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
            bool val = increment(current, possible_raw_datasOFCurrentMatch.count() - 1);
            return val;
        }
        QList<QList <RawData > > possible_raw_datasOFCurrentMatch;
        tree *Tree;

        /// Holds the type of the required solution search
        item_types type;
        result_node *reached_node;

        /// boolean to specify whether reduction trhough diacritics is required
        virtual bool shouldcall_onmatch_ex()    {
            return true;
        }
        bool on_match_helper();

        void setSolutionSettings(multiply_params params) {
            multi_p = params;
        }
    public:
        TreeEnumerate(item_types type) {
            this->type = type;

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
        /*virtual*/ void fill_details();
        virtual bool onMatch() = 0;// returns true to continue, false to abort
        virtual ~TreeEnumerate() {

            if (result_nodes != NULL) {
                delete result_nodes;
            }

        }
};

#endif

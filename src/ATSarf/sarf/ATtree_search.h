#ifndef _TREE_SEARCH_H
#define _TREE_SEARCH_H

#include <QVector>
#include <QString>
#include <QList>
#include <QQueue>
#include "../common_structures/tree.h"
#include "../caching_structures/database_info_block.h"
#include "../common_structures/common.h"

class Stemmer;

class TreeSearch
{
    public:
        int startingPos;
        QList<int> sub_positionsOFCurrentMatch;
        QList<long> catsOFCurrentMatch;
        QList<long> idsOFCurrentMatch;
        long resulting_category_idOFCurrentMatch;
#ifdef REDUCE_THRU_DIACRITICS
        QList<QString > raw_datasOFCurrentMatch;
    protected:
        bool a_branch_returned_false; //needed by get_all_possibilities() to stop when a false is retuned
        QList<QList <QString > > possible_raw_datasOFCurrentMatch;
        void get_all_possibilities(int i, QList<QString> raw_datas);
#endif
    protected:
        tree* Tree;
        Stemmer* info;
        item_types type;
        QQueue<letter_node *> queue;
#if defined(PARENT)
        node * reached_node;
#endif
        int position;//note that provided position is 1+last_letter after traversal
        //int number_of_matches;

        virtual bool shouldcall_onmatch(int);
        inline bool on_match_helper();
    public:
        TreeSearch(item_types type,Stemmer* info,int position);
        virtual bool operator()();
        void fill_details(); //this function fills the public member functions such as QList<int> sub_positionsOFCurrentMatch & QList<long> catsOFCurrentMatch;
        virtual bool onMatch() = 0;// returns true to continue, false to abort
        virtual ~TreeSearch();
};

#endif // TREE_SEARCH_H

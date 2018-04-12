/**
  * @file   database_info_block.h
  * @author Jad Makhlouta and documented by Ameen Jaber
  * @brief  This file implements the database_info_block class as well as some other types required for database extraction
  * and data manipulation
  */
#ifndef DATABASE_INFO_BLOCK_H
#define DATABASE_INFO_BLOCK_H

#include "compatibility_rules.h"
#include "tree.h"
#include "atmTrie.h"
#include "common.h"
#include <QString>
#include <QDateTime>
#include "Triplet.h"
#include "dbitvec.h"
#include "ATMProgressIFC.h"

/// Input triplet to hash table (item_id,category_id,raw_data)
typedef Triplet<long long, long, QString> ItemEntryKey;
/// Output triplet from the hash table (abstract_categories or reverse_description,description_id,POS)
typedef Triplet<dbitvec, long, QString> ItemEntryInfo;
/// Hash table that takes ItemEntry triplet and returns ItemEntryInfo triplet
typedef QMultiHash<ItemEntryKey, ItemEntryInfo > ItemCatRaw2AbsDescPosMap;
/// Type definition of a pointer to a triplet to triplet hash function
typedef ItemCatRaw2AbsDescPosMap *ItemCatRaw2AbsDescPosMapPtr;
/// Type definition of an iterator over a hash function
typedef ItemCatRaw2AbsDescPosMap::iterator ItemCatRaw2AbsDescPosMapItr;

/**
  * @class  database_info_block
  * @brief  This class implements the methods required to read the lexer data from the database and build the required
  * structure (trie) out of them to be used later
  * @author Jad Makhlouta and documented by Ameen Jaber
  */
class database_info_block {
    private:
        void readTrieFromDatabaseAndBuildFile();
        void buildTrie();
        void buildDescriptions();
        void readDescriptionsFromDatabaseAndBuildFile();
        void buildMap(item_types type, ItemCatRaw2AbsDescPosMap *map);
        void fillMap(item_types type, ItemCatRaw2AbsDescPosMap *map);
    public:
        ATMProgressIFC *prgsIFC;

        tree *Prefix_Tree;
        tree *Suffix_Tree;
        ATTrie *Stem_Trie;
        StemNodesList *trie_nodes;
        compatibility_rules *comp_rules;

        /// Triplet Hash for the prefix
        ItemCatRaw2AbsDescPosMapPtr map_prefix;
        /// Triplet Hash for the stem
        ItemCatRaw2AbsDescPosMapPtr map_stem;
        /// Triplet Hash for the suffix
        ItemCatRaw2AbsDescPosMapPtr map_suffix;

        QVector<QString> *descriptions;

        database_info_block();
        void fill(ATMProgressIFC *p_ifc);
        ~database_info_block();
};

extern database_info_block database_info;
extern QDateTime executable_timestamp;

#endif


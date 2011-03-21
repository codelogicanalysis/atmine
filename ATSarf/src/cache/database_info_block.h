#ifndef DATABASE_INFO_H
#define DATABASE_INFO_H

#include "compatibility_rules.h"
#include "tree.h"
#include "atmTrie.h"
#include "common.h"
#include <QString>
#include <QDateTime>
#include "Triplet.h"
#include "dbitvec.h"
#include "ATMProgressIFC.h"

typedef Triplet<long long, long, QString> ItemEntryKey;  //(item_id,category_id,raw_data)
typedef Triplet<dbitvec,long,QString> ItemEntryInfo; //(abstract_categories,description_id,POS)

typedef QMultiHash<ItemEntryKey,ItemEntryInfo > ItemCatRaw2PosDescAbsMap;//change all uses of this map
typedef ItemCatRaw2PosDescAbsMap * ItemCatRaw2PosDescAbsMapPtr;
typedef ItemCatRaw2PosDescAbsMap::iterator ItemCatRaw2PosDescAbsMapItr;

class database_info_block
{
    public:
        tree* Prefix_Tree;
        tree* Suffix_Tree;
#ifdef USE_TRIE
		ATTrie * Stem_Trie;
		QVector<StemNode> * trie_nodes;
#endif
		compatibility_rules * comp_rules;

		ItemCatRaw2PosDescAbsMapPtr map_prefix;
		ItemCatRaw2PosDescAbsMapPtr map_stem;
		ItemCatRaw2PosDescAbsMapPtr map_suffix;

		QVector<QString>* descriptions;

        database_info_block();
		void fill(ATMProgressIFC *p_ifc);
        ~database_info_block();
};

extern database_info_block database_info;
extern QString trie_path;
extern QString trie_list_path;
extern QDateTime executable_timestamp;

#endif // DATABASE_INFO_H

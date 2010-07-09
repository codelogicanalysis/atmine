#ifndef DATABASE_INFO_H
#define DATABASE_INFO_H

#include "compatibility_rules.h"
#include "tree.h"
#include "atmTrie.h"
#include "common.h"
#include <QString>
#include <QDateTime>
#include "Triplet.h"
#include "ATMProgressIFC.h"

typedef Triplet<long long, long, QString> Map_key;
typedef Triplet<bitset<max_sources>,QString,QString> Map_entry;
class database_info_block
{
    public:
        tree* Prefix_Tree;
        tree* Suffix_Tree;
#ifdef USE_TRIE
		ATTrie * Stem_Trie;
		QVector<StemNode> * trie_nodes;
#endif
        compatibility_rules * rules_AA;
        compatibility_rules * rules_AB;
        compatibility_rules * rules_AC;
        compatibility_rules * rules_BC;
        compatibility_rules * rules_CC;

		QHash<Map_key,Map_entry > * map_prefix;
		QHash<Map_key,Map_entry > * map_stem;
		QHash<Map_key,Map_entry > * map_suffix;

        database_info_block();
		void fill(ATMProgressIFC *p_ifc);
        ~database_info_block();
};

extern database_info_block database_info;
extern QString trie_path;
extern QString trie_list_path;
extern QDateTime executable_timestamp;

#endif // DATABASE_INFO_H

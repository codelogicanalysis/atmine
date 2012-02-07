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
typedef Triplet<dbitvec,long,QString> ItemEntryInfo; //(abstract_categories or reverse_description,description_id,POS)

typedef QMultiHash<ItemEntryKey,ItemEntryInfo > ItemCatRaw2AbsDescPosMap;//change all uses of this map
typedef ItemCatRaw2AbsDescPosMap * ItemCatRaw2AbsDescPosMapPtr;
typedef ItemCatRaw2AbsDescPosMap::iterator ItemCatRaw2AbsDescPosMapItr;

class database_info_block {
	private:
		void readTrieFromDatabaseAndBuildFile();
		void buildTrie();
		void buildDescriptions();
		void readDescriptionsFromDatabaseAndBuildFile();
		void buildMap(item_types type,ItemCatRaw2AbsDescPosMap * map);
		void fillMap(item_types type,ItemCatRaw2AbsDescPosMap * map);
    public:
		ATMProgressIFC *prgsIFC;

        tree* Prefix_Tree;
        tree* Suffix_Tree;
#ifdef USE_TRIE
		ATTrie * Stem_Trie;
		StemNodesList * trie_nodes;
#endif
		compatibility_rules * comp_rules;

		ItemCatRaw2AbsDescPosMapPtr map_prefix;
		ItemCatRaw2AbsDescPosMapPtr map_stem;
		ItemCatRaw2AbsDescPosMapPtr map_suffix;

		QVector<QString>* descriptions;

        database_info_block();
		void fill(ATMProgressIFC *p_ifc);
        ~database_info_block();
};

extern database_info_block database_info;
extern QDateTime executable_timestamp;

#endif // DATABASE_INFO_H

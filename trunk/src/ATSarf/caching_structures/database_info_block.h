#ifndef DATABASE_INFO_H
#define DATABASE_INFO_H

#include "compatibility_rules.h"
#include "../common_structures/tree.h"

class database_info_block
{
    public:
        tree* Prefix_Tree;
        tree* Suffix_Tree;
		//tree * Stem_Tree;
        compatibility_rules * rules_AA;
        compatibility_rules * rules_AB;
        compatibility_rules * rules_AC;
        compatibility_rules * rules_BC;
        compatibility_rules * rules_CC;
        database_info_block();
        void fill();
        ~database_info_block();
};

extern database_info_block database_info;

#endif // DATABASE_INFO_H

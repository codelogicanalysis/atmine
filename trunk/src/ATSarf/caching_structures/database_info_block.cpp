#include "database_info_block.h"

database_info_block::database_info_block()
{
    Prefix_Tree=new tree();
    Suffix_Tree=new tree();
    rules_AA=new compatibility_rules(AA);
    rules_AB=new compatibility_rules(AB);
    rules_AC=new compatibility_rules(AC);
    rules_BC=new compatibility_rules(BC);
    rules_CC=new compatibility_rules(CC);
}

void database_info_block::fill()
{
    Prefix_Tree->build_affix_tree(PREFIX);
    Suffix_Tree->build_affix_tree(SUFFIX);
    rules_AA->fill();
    rules_AB->fill();
    rules_AC->fill();
    rules_BC->fill();
    rules_CC->fill();
}

database_info_block::~database_info_block()
{
    delete Prefix_Tree;
    delete Suffix_Tree;
    delete rules_AA;
    delete rules_AB;
    delete rules_AC;
    delete rules_BC;
    delete rules_CC;
}


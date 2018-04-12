#include "solution_position.h"

#include "tree_search.h"
#include "stem_search.h"

int solution_position::getAffixSolutionLength(TreeSearch * affixMachine)
{
	return affixMachine->affix_info.size();
}
minimal_item_info & solution_position::getIthAffixSolution(TreeSearch * affixMachine, int i)
{
	return affixMachine->affix_info[i];
}
AffixSolutionVector & solution_position::getAffixSolution(TreeSearch * affixMachine)
{
	return affixMachine->affix_info;
}
minimal_item_info & solution_position::getStemSolution(StemSearch * stemMachine)
{
	return *stemMachine->solution;
}

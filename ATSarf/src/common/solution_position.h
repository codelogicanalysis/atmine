#ifndef SOLUTION_POSITION_H
#define SOLUTION_POSITION_H

#include "common.h"
#include <QVector>
#include <QPair>
#include "database_info_block.h"

typedef QPair<int,ItemCatRaw2AbsDescPosMapItr> AffixPosition;//index of raw_data and of corresponding entry in map corresponding for this raw_data
typedef QVector<AffixPosition> InternalPositions;
typedef QVector<minimal_item_info> StoredInfo;
typedef QVector<minimal_item_info>  AffixSolutionVector;

class TreeSearch;
class StemSearch;

class solution_position
{
/*public:
	void print_positions() {
		qDebug()<<"--------";
		for (int i=0;i<indexes.size();i++) {
			ItemCatRaw2PosDescAbsMapItr itr=indexes.at(i).second;
			qDebug()<<indexes.at(i).first<<"=>"
					<<itr.key().first<<" "<<itr.key().second<<" "<<itr.key().third<<" "
					<<(long)&(itr.value().first)<<" "<<itr.value().second<<" "<<itr.value().third;
		}
	}*/
public:
	//int current_index;
	InternalPositions indexes;
	StoredInfo previous_solutions;//solutions stored for only current index
	void store_solution(const minimal_item_info & inf) {
		previous_solutions.append(inf);
	}
	void clear_stored_solutions() {
		previous_solutions.clear();
	}
	int getAffixSolutionLength(TreeSearch * affixMachine);
	minimal_item_info & getIthAffixSolution(TreeSearch * affixMachine, int i);
	AffixSolutionVector & getAffixSolution(TreeSearch * affixMachine);
	minimal_item_info & getStemSolution(StemSearch * stemMachine);

};

class SolutionsCompare
{
private:
	multiply_params params;

public:
	SolutionsCompare(multiply_params params=M_ALL)
	{
		this->params=params;
	}
	void setParams(multiply_params params)
	{
		this->params=params;
	}
	bool equal (minimal_item_info & sol1,minimal_item_info & sol2)
	{
		if (params.raw_data && sol1.raw_data!=sol2.raw_data)
			return false;
		if (params.abstract_category && sol1.abstract_categories!=sol2.abstract_categories)
			return false;
		if (params.description && sol1.description_id()!=sol2.description_id())
			return false;
		if (params.POS && sol1.POS!=sol2.POS)
			return false;
		return true;
	}
	bool found(solution_position * info, minimal_item_info & current_solution)//does not compare category_id
	{
		if (params.NONE())
			return true;
		if (params.ALL())
			return false;
		int count=info->previous_solutions.count();
		for (int i=0;i<count;i++)
			if (equal(current_solution,info->previous_solutions[i]))
				return true;
		return false;
	}

};
#endif // SOLUTION_POSITION_H

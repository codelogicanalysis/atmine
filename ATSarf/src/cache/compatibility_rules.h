#ifndef _COMPATIBILITY_RULES_H
#define	_COMPATIBILITY_RULES_H


#include <QPair>
#include <QVector>
#include <QHash>
#include "dbitvec.h"
#include "common.h"

typedef QVector<dbitvec> bitvec2d;
typedef QVector<long> index2id_map;
typedef QPair<long,long> ResultingCategoryKey;
typedef QHash< ResultingCategoryKey,long> ResultingCategoryMap;

class compatibility_rules
{
    private:
        rules rule;
        bitvec2d rules_info;
        index2id_map  bitorder[2];
        ResultingCategoryMap resulting_category;

		void generate_bit_order(rules rule);
		long get_bitindex(int order,long id);
    public:
        compatibility_rules(rules rule);
        void fill();
        bool operator()(int id1,int id2);
        bool operator()(int id1,int id2, long & id_r);//-1 is invalid
};

#endif	/* _COMPATIBILITY_RULES_H */


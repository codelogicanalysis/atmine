
#include "compatibility_rules.h"

#include <QString>
#include <QList>
#include <QVector>
#include <QHash>
#include <QtAlgorithms>
#include "../sql-interface/sql_queries.h"
#include "../utilities/dbitvec.h"
#include "../sql-interface/Retrieve_Template.h"

using namespace std;

void compatibility_rules::generate_bit_order(rules rule)
{
    item_types t[2];
    get_types_of_rule(rule,t[0],t[1]);
    for (int i=0;i<2;i++)
    {
		Retrieve_Template order(QString("category"),QString("id"),QString("abstract=0 AND type=%1 ORDER BY id ASC").arg((int)(t[i])));
		int j=0;
		while (order.retrieve())
		{
			bool ok;
			//bitorder[i].append(order.get(0).toLongLong(&ok));
			bitorder[i].insert(order.get(0).toLongLong(&ok),j);
			if (ok==false)
			{
					error << "Unexpected Error: Non-integer ID\n";
					return; //must not reach here
			}
			j++;
		}
    }
}
long compatibility_rules::get_bitindex(int order,long id)
{
	/*QVector<long>::iterator i = qBinaryFind(bitorder[order].begin(), bitorder[order].end(), id);
	if (*i==id)
		return i-bitorder[order].begin();
	else
	{
		error<<"Unexpected Error: id "<<id<< " is not part of the ids array\n";
		throw 1; //Not found
	}*/
	return bitorder[order].value(id,-1);
}
compatibility_rules::compatibility_rules(rules rule)
{
        this->rule=rule;
}
void compatibility_rules::fill()
{
        generate_bit_order(rule);
        int length[2];
		length[0]=bitorder[0].count();
        length[1]=bitorder[1].count();
        for (int i = 0; i < length[0]; i++) {
                rules_info.push_back( dbitvec() );
                rules_info[i].resize(length[1]);
        }
		Retrieve_Template order("compatibility_rules", "category_id1", "category_id2", "resulting_category",QString("type=%1").arg((int)(rule)));
		while (order.retrieve())
        {
				int i1=this->get_bitindex(0,order.get(0).toLongLong());
				int i2=this->get_bitindex(1,order.get(1).toLongLong());
                if (rule==AA || rule==CC)
                {
						bool isValueNull = order.get(2).isNull();
                        resulting_category[ResultingCategoryKey(i1,i2)]= isValueNull ?
																						  order.get(1).toLongLong() :
																						  order.get(2).toLongLong();
                }
                rules_info[i1][i2]=true;
                /*if (ok==false)
                {
                        error << "Unexpected Error: Non-integer ID\n";
                        return; //must not reach here
                }*/
        }
}
bool compatibility_rules::operator()(int id1,int id2)
{
        try{
                int i1=get_bitindex(0,id1);
                int i2=get_bitindex(1,id2);
                return rules_info[i1][i2];
        }catch(int i) {
                return false;
        }
}
bool compatibility_rules::operator()(int id1,int id2, long & id_r)//-1 is invalid
{
        try{
                int i1=get_bitindex(0,id1);
                int i2=get_bitindex(1,id2);
                if (rule==AA || rule==CC)
                {
                        ResultingCategoryKey k(i1,i2);
                        if (resulting_category.contains(k))
                                 id_r=resulting_category[k];
                }
                return rules_info[i1][i2];
        }catch(int i) {
                return false;
        }
}

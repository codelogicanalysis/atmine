#ifndef DATABASE_INFO_H
#define DATABASE_INFO_H

#include <QList>
#include <QVector>
#include <QMap>
#include "sql-interface.h"
#include <QtAlgorithms>
#include "tree.h"
#include "dbitvec.h"


class compatibility_rules
{
	private:
		rules rule;
		typedef QVector<dbitvec> bitvec2d;
		bitvec2d rules_info;
		typedef QVector<long> index2id_map;
		index2id_map  bitorder[2];
		typedef QPair<long,long> ResultingCategoryKey;
		typedef QMap<ResultingCategoryKey,long> ResultingCategoryMap;
		ResultingCategoryMap resulting_category;

		inline void generate_bit_order(rules rule)
		{
			item_types t[2];
			get_types_of_rule(rule,t[0],t[1]);
			for (int i=0;i<2;i++)
			{
				QString stmt=QString( "SELECT id FROM category WHERE abstract=0 AND type=%1 ORDER BY id ASC").arg((int)(t[i]));
				if (!execute_query(stmt))
					return;
				bool ok;
				while (query.next())
				{
					bitorder[i].append(query.value(0).toLongLong(&ok));
					if (ok==false)
					{
						error << "Unexpected Error: Non-integer ID\n";
						return; //must not reach here
					}
				}
			}
		}
		inline long get_bitindex(int order,long id)
		{
			QVector<long>::iterator i = qBinaryFind(bitorder[order].begin(), bitorder[order].end(), id);
			if (*i==id)
				return i-bitorder[order].begin();
			else
			{
				error<<"Unexpected Error: id "<<id<< " is not part of the ids array\n";
				throw 1 /*Not found*/;
			}
		}
	public:
		compatibility_rules(rules rule)
		{
			this->rule=rule;
		}
		void fill()
		{
			generate_bit_order(rule);
			int length[2];
			length[0]=bitorder[0].count();
			length[1]=bitorder[1].count();
			for (int i = 0; i < length[0]; i++) {
				rules_info.push_back( dbitvec() );
				rules_info[i].resize(length[1]);
			}
			QString stmt=QString( "SELECT category_id1, category_id2, resulting_category FROM compatibility_rules WHERE type=%1").arg((int)(rule));
			if (!execute_query(stmt))
				return;
			while (query.next())
			{
				int i1=this->get_bitindex(0,query.value(0).toLongLong());
				int i2=this->get_bitindex(1,query.value(1).toLongLong());
				if (rule==AA || rule==CC)
				{
					bool isValueNull = query.value(2).isNull();
					resulting_category[ResultingCategoryKey(i1,i2)]= isValueNull ?
													  query.value(1).toLongLong() :
													  query.value(2).toLongLong();
				}
				rules_info[i1][i2]=true;
				/*if (ok==false)
				{
					error << "Unexpected Error: Non-integer ID\n";
					return; //must not reach here
				}*/
			}
		}
		bool operator()(int id1,int id2)
		{
			try{
				int i1=get_bitindex(0,id1);
				int i2=get_bitindex(1,id2);
				return rules_info[i1][i2];
			}catch(int i) {
				return false;
			}
		}
		bool operator()(int id1,int id2, long & id_r)//-1 is invalid
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
};

class database_info_block
{
	public:
		tree* Prefix_Tree;
		tree* Suffix_Tree;
		//trie * Stem_Trie;
		compatibility_rules * rules_AA;
		compatibility_rules * rules_AB;
		compatibility_rules * rules_AC;
		compatibility_rules * rules_BC;
		compatibility_rules * rules_CC;
		database_info_block()
		{
			Prefix_Tree=new tree();
			Suffix_Tree=new tree();
			rules_AA=new compatibility_rules(AA);
			rules_AB=new compatibility_rules(AB);
			rules_AC=new compatibility_rules(AC);
			rules_BC=new compatibility_rules(BC);
			rules_CC=new compatibility_rules(CC);
		}
		void fill()
		{
			Prefix_Tree->build_affix_tree(PREFIX);
			Suffix_Tree->build_affix_tree(SUFFIX);
			rules_AA->fill();
			rules_AB->fill();
			rules_AC->fill();
			rules_BC->fill();
			rules_CC->fill();
		}
		~database_info_block()
		{
			delete Prefix_Tree;
			delete Suffix_Tree;
			delete rules_AA;
			delete rules_AB;
			delete rules_AC;
			delete rules_BC;
			delete rules_CC;
		}
};

database_info_block database_info;

#endif // DATABASE_INFO_H

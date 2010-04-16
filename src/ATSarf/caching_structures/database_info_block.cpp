#include <QFile>
#include "database_info_block.h"
#include "../common_structures/atmTrie.h"
#include "../common_structures/common.h"
#include "../sql-interface/sql_queries.h"
#include "../sql-interface/Search_by_item.h"
#include <assert.h>

#ifdef USE_TRIE
void buildTrie(ATTrie* trie)
{
	/*QFile input(trie_path);
	if (input.open(QIODevice::ReadOnly))
	{
		delete database_info.Stem_Trie;
		input.close();
		ATTrie * temp=new ATTrie(trie_path.toStdString().data());
		database_info.Stem_Trie=temp;
		return;
	}*/
	QSqlQuery query(db);
	QString stmt=QString("SELECT id, name FROM stem");
	QString name;
	unsigned long long  stem_id;
	if (!execute_query(stmt,query))
		return;
	while (query.next())
	{
			name=query.value(1).toString();
			stem_id=query.value(0).toULongLong();
			Search_by_item s1(STEM,stem_id);
#ifdef MEMORY_EXHAUSTIVE
			minimal_item_info inf;
			while(s1.retrieve(inf))
			{
					StemNode * node=new StemNode();
					node->category_id=inf.category_id;
					node->key=name;
					node->raw_datas.append(inf.raw_data);
					node->stem_id=stem_id;
					node->description=inf.description;//must be changed later
			}
#elif defined(REDUCE_THRU_DIACRITICS)
			minimal_item_info inf;
			while(s1.retrieve(inf))
			{
				StemNode * node = NULL;
				trie->retreive(name,&node);
				if (node == NULL){
					node=new StemNode();
					node->add_info(inf.category_id,inf.raw_data);
					node->key=name;
					node->stem_id=stem_id;
					trie->store(name,node);
				}
				else
				{
					assert(node->key==name);
					assert(node->stem_id==stem_id);
					node->add_info(inf.category_id,inf.raw_data);
				}
			}
#else
			long cat_id;
			while(s1.retrieve(cat_id))
			{
				StemNode * node = NULL;
				trie->retreive(name,&node);
				if (node == NULL){
					node=new StemNode();
					node->add_info(inf.category_id);
					node->key=name;
					node->stem_id=stem_id;
					trie.store(name,&node);
				}
				else
				{
					assert(node->key==name);
					assert(node->stem_id==stem_id);
					node->add_info(inf.category_id);
				}
			}

#endif
	}
	database_info.Stem_Trie->save(trie_path.toStdString().data());
}
#endif
database_info_block::database_info_block()
{
    Prefix_Tree=new tree();
    Suffix_Tree=new tree();
#ifdef USE_TRIE
	Stem_Trie= new ATTrie();
#endif
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
#ifdef USE_TRIE
	buildTrie(Stem_Trie);
#endif
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
#ifdef USE_TRIE
	delete Stem_Trie;
#endif
    delete rules_AA;
    delete rules_AB;
    delete rules_AC;
    delete rules_BC;
    delete rules_CC;
}

database_info_block database_info;

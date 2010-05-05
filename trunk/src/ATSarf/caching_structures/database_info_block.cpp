#include <QFile>
#include <QDateTime>
#include "database_info_block.h"
#include "../common_structures/atmTrie.h"
#include "../common_structures/common.h"
#include "../sql-interface/sql_queries.h"
#include "../sql-interface/Search_by_item.h"
#include <assert.h>
#include <QDebug>


#ifdef USE_TRIE

void buildfromfile()
{
	QSqlQuery query(db);
#ifdef REDUCE_THRU_DIACRITICS
	QString stmt=QString("SELECT stem.id, stem.name, stem_category.category_id, stem_category.raw_data FROM stem, stem_category WHERE stem.id=stem_category.stem_id ORDER BY stem.id ASC");
#else
	QString stmt=QString("SELECT stem.id, stem.name, stem_category.category_id FROM stem, stem_category WHERE stem.id=stem_category.stem_id ORDER BY stem.id ASC");
#endif
	QString name,raw_data;
	long category_id;
	long long  stem_id, last_id;
	if (!execute_query(stmt,query))
		return;
	out<<QDateTime::currentDateTime().time().toString()<<"\n";
	StemNode * node = NULL;
	int index=0;
	last_id=-1;
	while (query.next())
	{
		stem_id=query.value(0).toULongLong();
		if (last_id!=stem_id)
		{
			if (last_id!=-1)
			{
				database_info.trie_nodes->insert(index,*node);
				delete node;
				database_info.Stem_Trie->store(name,index);
				index++;
			}
			last_id=stem_id;
			node=new StemNode();
			node->key=name;
			node->stem_id=stem_id;
		}
		name=query.value(1).toString();
		category_id=query.value(2).toLongLong();
#ifdef REDUCE_THRU_DIACRITICS
		raw_data=query.value(3).toString();
		node->add_info(category_id,raw_data);
#else
		node->add_info(category_id);
#endif
	}
	out<<QDateTime::currentDateTime().time().toString()<<"\n";
	database_info.Stem_Trie->save(trie_path.toStdString().data());
	QFile file(trie_list_path.toStdString().data());
	if (file.open(QIODevice::WriteOnly))
	{
		QDataStream out(&file);   // we will serialize the data into the file
		out << *(database_info.trie_nodes);
		file.close();
	}
	else
		error <<"Unexpected Error: Unable to write TRIE to file\n";
}

void buildTrie()
{
	out<<QDateTime::currentDateTime().time().toString()<<"\n";
#ifndef TRIE_FROM_FILE
	buildfromfile();
#else
	QFile file(trie_list_path.toStdString().data());
	if (file.open(QIODevice::ReadOnly))
	{
		QDataStream in(&file);    // read the data serialized from the file
		in >> *(database_info.trie_nodes);
		file.close();
		QFile input(trie_path);
		if (input.open(QIODevice::ReadOnly))
		{
			delete database_info.Stem_Trie;
			input.close();
			database_info.Stem_Trie=new ATTrie(trie_path.toStdString().data());
			out<<QDateTime::currentDateTime().time().toString()<<"\n";
		}
	}
	else
		buildfromfile();
#endif
}
#endif

database_info_block::database_info_block()
{
    Prefix_Tree=new tree();
    Suffix_Tree=new tree();
#ifdef USE_TRIE
	Stem_Trie= new ATTrie();
	trie_nodes=new QVector<StemNode>();
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
	buildTrie();
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
	delete trie_nodes;
#endif
    delete rules_AA;
    delete rules_AB;
    delete rules_AC;
    delete rules_BC;
    delete rules_CC;
}

database_info_block database_info;

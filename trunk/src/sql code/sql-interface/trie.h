#ifndef TRIE_H
#define TRIE_H

#include "sql-interface.h"
#include "tree.h"

tree* sample_tree =new tree();

node * add_to_trie(item_types , QString content,long id2, long id_r, node * current)
{
	//out << content<<"\n";
	return sample_tree->addElement(content,id2,id_r,current);
}

int helper(long cat_id1, int size,node * current)
{
	if (size<=0)
		return 0;
	long long prefix_id;
	long cat_id2,cat_r_id;
	Search_Compatibility s2(AA,cat_id1);
	while (s2.retrieve(cat_id2,cat_r_id))
	{
		Search_by_category s3(cat_id2);
		while(s3.retrieve(prefix_id))
		{
			QString name= getColumn("prefix","name",prefix_id);
			node * next=add_to_trie(PREFIX,name,cat_id2,cat_r_id,current);
			helper(cat_r_id,size-name.length(),next);
		}
	}
	return 0;
}

int generate_all_prefixes()
{
	QSqlQuery query(db);
	QString stmt("SELECT id, name FROM prefix");
	QString name;
	unsigned long prefix_id1;
	bool ok;
	node * first_node=sample_tree->getFirstNode();
	if (!execute_query(stmt,query))
		return -1;;
	while (query.next())
	{
		name=query.value(1).toString();
		prefix_id1=query.value(0).toLL();
		long cat_id;
		Search_by_item s1(PREFIX,prefix_id1);
		while(s1.retrieve(cat_id))
		{
			node * next=add_to_trie(PREFIX,name,cat_id,cat_id,first_node);
			helper(prefix_id1,6-name.length(),next);
		}
		//helper(prefix_id1,6-name.length(),first_node);
	}
	return 0;
}
#endif // TRIE_H

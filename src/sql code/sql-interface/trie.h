#ifndef TRIE_H
#define TRIE_H

#include "sql-interface.h"

typedef struct node_
{

} node;


node * add_to_trie(item_types type, QString content, node * current)
{
	out << content<<"\n";
	return current;
}

int helper(long long prefix_id1, int size,node * current)
{
	if (size==0)
		return 0;
	long long prefix_id2,prefix_id_continuation;
	Search_by_item s1(PREFIX,prefix_id1);
	//QString name=s1.Name(); //not name of first prefix but second is what we need to add here
	long cat_id1,cat_id2,cat_r_id;
	while(s1.retrieve(cat_id1))
	{
		Search_Compatibility s2(AA,cat_id1);
		while (s2.retrieve(cat_id2,cat_r_id))
		{
			Search_by_category s3(cat_id2);
			while(s3.retrieve(prefix_id2))
			{
				QString name= getColumn("prefix","name",prefix_id2);
				node * next=add_to_trie(PREFIX,name,current);
				Search_by_category s4(cat_r_id);
				while (s4.retrieve(prefix_id_continuation))
				{
					helper(prefix_id_continuation,size-name.length(),next);
				}
			}
		}
	}
	return 0;
}

int generate_all_prefixes()
{
	QSqlQuery query(db);
	QString stmt("SELECT id, name FROM prefix");
	QString name;
	long prefix_id1;
	bool ok;
	node * first_node=NULL;
	if (!execute_query(stmt,query))
		return -1;;
	while (query.next())
	{
		name=query.value(1).toString();
		prefix_id1=query.value(0).toLL();
		node * next=add_to_trie(PREFIX,name,first_node);
		helper(prefix_id1,6-name.length(),next);
	}
	return 0;
}
#endif // TRIE_H

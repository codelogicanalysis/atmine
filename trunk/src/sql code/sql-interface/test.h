#ifndef TEST_H
#define TEST_H

//#include "insert_scripts.h"
#include "sql-interface.h"
#include "tree.h"
#include "tree_search.h"

//starting point
int start(QString input_str, QString &output_str, QString &error_str)
{

	out.setString(&output_str);
	out.setCodec("utf-8");
	in.setString(&input_str);
	in.setCodec("utf-8");
	displayed_error.setString(&error_str);
	displayed_error.setCodec("utf-8");
	/*if (insert_buckwalter()<0)//generate_all_prefixes()
		return -1;
	if (insert_propernames()<0)
		return -1;
	if (insert_placenames()<0)
		return -1;*/
	int source_id=insert_source("Just for testing","","");
	insert_item(PREFIX,"ABCFG","","C9_2",source_id);
	insert_item(PREFIX,"ABC","","C7",source_id);
	insert_item(PREFIX,"FG","","C9_1",source_id);
	insert_item(PREFIX,"DE","","C8",source_id);
	insert_item(PREFIX,"AB","","C5",source_id);
	insert_item(PREFIX,"CD","","C6",source_id);
	insert_item(PREFIX,"A","","C1",source_id);
	insert_item(PREFIX,"B","","C2",source_id);
	insert_item(PREFIX,"CD","","C3",source_id);
	insert_item(PREFIX,"CD","","C10",source_id);
	insert_category("rc2",PREFIX,source_id, false);
	insert_category("rc3",PREFIX,source_id, false);
	insert_category("rc10",PREFIX,source_id, false);
	insert_category("rc6",PREFIX,source_id, false);
	insert_category("rc8",PREFIX,source_id, false);
	insert_category("rc9",PREFIX,source_id, false);
	insert_compatibility_rules(AA,"C1","C2","rc2",source_id);
	insert_compatibility_rules(AA,"rc2","C3","rc3",source_id);
	insert_compatibility_rules(AA,"rc3","C10","rc10",source_id);
	insert_compatibility_rules(AA,"C5","C6","rc6",source_id);
	insert_compatibility_rules(AA,"C7","C8","rc8",source_id);
	insert_compatibility_rules(AA,"C7","C9_1","rc9",source_id);
	QString word;
	in >>word;
	/*out << removeLastDiacritic(word)<<"\n";*/
	/*for (int i=0;i<word.length();i++)
		out<<((bitset<16>((int)word[word.length()-i-1].toAscii())).to_string().data());
	out <<"\n";
	for (int i=0;i<word.length();i++)
		out<<(int)word[word.length()-i-1].unicode();
	out<<"\n"<<bitset_to_string(string_to_bitset(word))<<"\n";*/
	//display_table("stem_category");
	/*tree* sample_tree =new tree();
	sample_tree->sample();
	sample_tree->print_tree();
	out<<"-----------------------------------------------\n";
	sample_tree->addElement(word,10,30);
	sample_tree->print_tree();*/
	tree* sample_tree=new tree();
	/*sample_tree->sample();
	sample_tree->print_tree();
	sample_tree->reset();
	sample_tree->print_tree();
	//sample_tree->sample();*/
	sample_tree->build_affix_tree(PREFIX);
	sample_tree->traverse_text(word,0);
	sample_tree->print_tree();
	return 0;
}

#endif // TEST_H

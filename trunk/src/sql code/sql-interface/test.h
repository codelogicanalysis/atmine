#ifndef TEST_H
#define TEST_H

#include "insert_scripts.h"
#include "sql-interface.h"
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
	/*if (insert_buckwalter()<0)
		return -1;*/
	/*if (insert_propernames()<0)
		return -1;
	if (insert_placenames()<0)
		return -1;*/
	/*int source_id=insert_source("Just for testing","","");
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

	insert_item(STEM,"BC","","S1",source_id);
	insert_item(STEM,"CD","","S2",source_id);
	insert_item(STEM,"FG","","S3",source_id);
	insert_item(STEM,"DE","","S4",source_id);
	insert_item(STEM,"AB","","S5",source_id);
	insert_compatibility_rules(AB,"C1","S1",source_id);
	insert_compatibility_rules(AB,"rc2","S2",source_id);
	insert_compatibility_rules(AB,"rc3","S3",source_id);
	insert_compatibility_rules(AB,"C5","S3",source_id);
	insert_compatibility_rules(AB,"C7","S3",source_id);
	insert_compatibility_rules(AB,"rc2","S1",source_id);
	insert_compatibility_rules(AB,"rc3","S2",source_id);
	insert_compatibility_rules(AB,"rc10","S4",source_id);
	insert_compatibility_rules(AB,"rc6","S4",source_id);
	insert_compatibility_rules(AB,"rc8","S5",source_id);
	insert_compatibility_rules(AB,"rc9","S5",source_id);

	insert_item(SUFFIX,"ABCFG","","SC9_2",source_id);
	insert_item(SUFFIX,"ABC","","SC7",source_id);
	insert_item(SUFFIX,"FG","","SC9_1",source_id);
	insert_item(SUFFIX,"DE","","SC8",source_id);
	insert_item(SUFFIX,"AB","","SC5",source_id);
	insert_item(SUFFIX,"CD","","SC6",source_id);
	insert_item(SUFFIX,"A","","SC1",source_id);
	insert_item(SUFFIX,"B","","SC2",source_id);
	insert_item(SUFFIX,"CD","","SC3",source_id);
	insert_item(SUFFIX,"CD","","SC10",source_id);
	insert_category("Src2",SUFFIX,source_id, false);
	insert_category("Src3",SUFFIX,source_id, false);
	insert_category("Src10",SUFFIX,source_id, false);
	insert_category("Src6",SUFFIX,source_id, false);
	insert_category("Src8",SUFFIX,source_id, false);
	insert_category("Src9",SUFFIX,source_id, false);
	insert_compatibility_rules(CC,"SC1","SC2","Src2",source_id);
	insert_compatibility_rules(CC,"Src2","SC3","Src3",source_id);
	insert_compatibility_rules(CC,"Src3","SC10","Src10",source_id);
	insert_compatibility_rules(CC,"SC5","SC6","Src6",source_id);
	insert_compatibility_rules(CC,"SC7","SC8","Src8",source_id);
	insert_compatibility_rules(CC,"SC7","SC9_1","Src9",source_id);

	insert_compatibility_rules(BC,"S1","Src2",source_id);
	insert_compatibility_rules(BC,"S2","Src3",source_id);
	insert_compatibility_rules(BC,"S3","SC7",source_id);
	insert_compatibility_rules(BC,"S4","Src6",source_id);
	insert_compatibility_rules(BC,"S4","Src8",source_id);
	insert_compatibility_rules(BC,"S1","Src9",source_id);

	insert_compatibility_rules(AC,"C1","Src2",source_id);
	insert_compatibility_rules(AC,"rc2","Src2",source_id);
	insert_compatibility_rules(AC,"rc3","Src2",source_id);
	insert_compatibility_rules(AC,"C5","Src2",source_id);
	insert_compatibility_rules(AC,"C7","SC7",source_id);
	insert_compatibility_rules(AC,"C7","Src3",source_id);
	insert_compatibility_rules(AC,"rc2","Src3",source_id);
	insert_compatibility_rules(AC,"rc3","Src3",source_id);
	insert_compatibility_rules(AC,"rc10","Src6",source_id);
	insert_compatibility_rules(AC,"rc6","Src10",source_id);
	insert_compatibility_rules(AC,"rc8","Src8",source_id);
	insert_compatibility_rules(AC,"rc9","Src9",source_id);*/
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
	//tree* sample_tree=new tree();
	/*sample_tree->sample();
	sample_tree->print_tree();
	sample_tree->reset();
	sample_tree->print_tree();
	//sample_tree->sample();*/
	//sample_tree->build_affix_tree(PREFIX);
	//sample_tree->traverse_text(word,0);
	//out<<"---\n";
	//sample_tree->print_tree();
	PrefixSearch search(word);
	search();
	return 0;
}

#endif // TEST_H
#ifndef TEST_H
#define TEST_H

//#include "insert_scripts.h"
#include "sql-interface.h"
#include "tree.h"
#include "trie.h"

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
	generate_all_prefixes();
	sample_tree->print_tree();
	return 0;
}

#endif // TEST_H

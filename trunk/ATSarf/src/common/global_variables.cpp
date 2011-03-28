#include <QString>
#include <bitset>
#include <QSqlDatabase>
#include <QTextStream>
#include <QDateTime>
#include "hadith.h" //just to propagate Directives such as SUBMISSION
#include "database_info_block.h"
#include "letters.h"
#include "sql_queries.h"

dbitvec INVALID_BITSET(max_sources);

QSqlDatabase db;

QTextStream out;
QTextStream in;
QTextStream displayed_error;
QTextStream hadith_out;
//QTextStream chainOut;

bool KEEP_OLD=true;
bool warn_about_automatic_insertion =false;
bool display_errors=true;
bool display_warnings=true;

int source_ids[max_sources+1]={0};//here last element stores number of filled entries in the array
int abstract_category_ids[max_sources+1]={0};//here last element stores number of filled entries in the array

QString databaseFileName;

#ifdef LOAD_FROM_FILE
QString trie_path;
QString trie_list_path;
QString compatibility_rules_path;
QString prefix_tree_path;
QString suffix_tree_path;
QString description_path;
QString prefix_info_path;
QString suffix_info_path;
QString stem_info_path;
#endif

QDateTime executable_timestamp;

QVector<QChar> alefs(5);
QString delimiters,punctuation,nonconnectingLetters;

void initialize_variables()
{
#ifndef SUBMISSION
	databaseFileName="../../src/sql design/atm_filled.sql";
#else
	databaseFileName=".atm_filled.sql";
#endif
#ifdef LOAD_FROM_FILE
	trie_path=".stem_trie.dat";
	trie_list_path=".stem_list.dat";
	compatibility_rules_path= ".compatibility.dat";
	prefix_tree_path=".prefix_tree.dat";
	suffix_tree_path=".suffix_tree.dat";
	description_path=".descriptions.dat";
	prefix_info_path=".prefix_info.dat";
	suffix_info_path=".suffix_info.dat";
	stem_info_path=".stem_info.dat";
#endif


	alefs[0]=alef;
	alefs[1]=alef_hamza_above;
	alefs[2]=alef_hamza_below;
	alefs[3]=alef_madda_above;
	alefs[4]=alef_wasla;
	nonconnectingLetters= QString("")+alef+alef_hamza_above+alef_hamza_below+alef_madda_above+alef_wasla+aleft_superscript+
						  waw_hamza_above+waw+hamza+zain+ra2+thal+dal+ta2_marbouta+alef_maksoura;
	delimiters=QString("[ :\\.,()-><{}\\/|'\"\n")+fasila+question_mark+semicolon+full_stop+full_stop2+full_stop3+"]";
	punctuation=QString(":\\.,()-'\n\"")+fasila+question_mark+semicolon+full_stop+full_stop2+full_stop3;

	generate_bit_order("source",source_ids);
	generate_bit_order("category",abstract_category_ids,"abstract");
	INVALID_BITSET.reset();
	INVALID_BITSET.setBit(max_sources-1,true);
	//INVALID_BITSET.show();
}

bool filling=true;

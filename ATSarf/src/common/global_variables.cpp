#include <QString>
#include <bitset>
#include <QSqlDatabase>
#include <QTextStream>
#include <QDateTime>
#include "hadith.h" //just to propagate Directives such as SUBMISSION
#include "database_info_block.h"
#include "letters.h"
#include "sql_queries.h"
#include "common.h"

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

#ifdef LOAD_FROM_FILE
QStringList cacheFileList;
#endif

QDateTime executable_timestamp;

void initialize_variables()
{
#ifdef LOAD_FROM_FILE
	cacheFileList<<trie_path<<trie_list_path<<compatibility_rules_path<<prefix_tree_path
				 <<suffix_tree_path<<description_path<<prefix_info_path<<suffix_info_path<<stem_info_path;
#endif
	//generate_bit_order's are last 2 statements that need database but are not used except in statements that need the database, so they dont hurt to remain
	generate_bit_order("source",source_ids);
	generate_bit_order("category",abstract_category_ids,"abstract");
	INVALID_BITSET.reset();
	INVALID_BITSET.setBit(max_sources-1,true);
	//INVALID_BITSET.show();
}

bool filling=true;
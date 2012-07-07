//#include <QFile>
#include <QString>
#include <bitset>
#include <QSqlDatabase>
#include <QTextStream>
#include <QTextCodec>
#include <QDateTime>
#include "hadith.h" //just to propagate Directives such as SUBMISSION
#include "database_info_block.h"
#include "letters.h"
#include "sql_queries.h"
#include "common.h"
#include "morphemes.h"

dbitvec INVALID_BITSET(max_sources);

//QSqlDatabase db;

//QTextStream out;
//QTextStream in;
//QTextStream displayed_error;
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

QString non_punctuation_delimiters;

void initialize_variables()
{
	QTextCodec *codec = QTextCodec::codecForName("utf-8");
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForTr(codec);
	QTextCodec::setCodecForLocale(codec);
#ifdef LOAD_FROM_FILE
	cacheFileList<<trie_path<<trie_list_path<<compatibility_rules_path<<prefix_tree_path
				 <<suffix_tree_path<<description_path<<prefix_info_path<<suffix_info_path<<stem_info_path;
#endif
	non_punctuation_delimiters=delimiters;
	non_punctuation_delimiters.remove(QRegExp(QString("[")+punctuation+"]")).remove('-').remove('[').remove(']').remove(':');

	INVALID_BITSET.reset();
	INVALID_BITSET.setBit(max_sources-1,true);
	//INVALID_BITSET.show();
}

void initialize_other() {
	morpheme_initialize();
}

bool filling=true;

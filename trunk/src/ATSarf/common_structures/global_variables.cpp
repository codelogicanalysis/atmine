#include <bitset>
#include <QSqlDatabase>
#include <QTextStream>
#include <QDateTime>
#include "../caching_structures/database_info_block.h"
#include "../utilities/letters.h"
#include "../sql-interface/sql_queries.h"

bitset<max_sources> INVALID_BITSET;

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

QString trie_path="stem_trie.dat";
QString trie_list_path="stem_list.dat";
QDateTime executable_timestamp;

QString delimiters(QString("[ :.,()-><{}\\/|'\"")+fasila+"]");

QVector<QChar> alefs(5);


void initialize_variables()
{
	alefs[1]=alef_hamza_above;
	alefs[0]=alef;
	alefs[2]=alef_hamza_below;
	alefs[3]=alef_madda_above;
	alefs[4]=alef_wasla;
	INVALID_BITSET.reset();
	INVALID_BITSET.set(max_sources-1);
}

bool filling=true;

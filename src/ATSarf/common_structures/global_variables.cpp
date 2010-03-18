#include <bitset>
#include <QSqlDatabase>
#include <QTextStream>
#include "../caching_structures/database_info_block.h"

bitset<max_sources> INVALID_BITSET;

QSqlDatabase db;

QTextStream out;
QTextStream in;
QTextStream displayed_error;

bool KEEP_OLD=true;
bool warn_about_automatic_insertion =false;
bool display_errors=true;
bool display_warnings=true;

database_info_block database_info;

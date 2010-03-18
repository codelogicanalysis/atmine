#ifndef SQL_INTERFACE_H
#define SQL_INTERFACE_H

#include <bitset>
#include <QString>
#include <QSqlDatabase>
#include <QTextStream>
#include <QDebug>
#include <QSqlQuery>
#include <QVariant>
#include "logger/logger.h"
#include "../common_structures/common.h"

using namespace std;

extern QSqlDatabase db;

#define perform_query(stmt)  \
	if (!execute_query(stmt)) \
		return -1;

inline bool execute_query(QString stmt);
inline bool execute_query(QString stmt, QSqlQuery &query);

inline QString interpret_type(item_types t);
inline QString interpret_type(rules r);
inline bitset<max_sources> bigint_to_bitset(unsigned long long ll);
inline bitset<max_sources> bigint_to_bitset(char * val);
inline bitset<max_sources> bigint_to_bitset(QVariant val);
inline bitset<max_sources> string_to_bitset(QString val);
inline bitset<max_sources> string_to_bitset(QVariant val);
inline bool start_connection(); //and do other initializations
inline void close_connection();
inline long get_abstractCategory_id(int bit);
inline long get_source_id(int bit);
inline bool existsID(QString table,unsigned long long id,QString additional_condition ="");
inline long long getID(QString table, QString name, QString additional_condition="", QString column_name="name");
inline QString getColumn(QString table, QString column_name, long long id, QString additional_condition="",bool has_id=true);
inline bool existsSOURCE(int source_id);
inline bitset<max_sources> getSources(QString table,unsigned long long id=-1, QString additional_condition ="", bool has_id=true);
inline int get_type_of_category(long category_id, item_types & type);
inline bool get_types_of_rule(rules rule, item_types &t1, item_types &t2);
inline bitset<max_sources> addSource(QString table, int source_id, long long id=-1 , QString additional_condition ="",bool has_id=true);
inline bitset<max_sources> addAbstractCategory(QString table, int abstract_category_id, long long id=-1 , QString additional_condition ="",bool has_id=true);
inline long insert_category(QString name, item_types type, bitset<max_sources> sources, bool isAbstract=false);//returns its id if already present and if names are equal but others are not, -1 is returned
inline long insert_category(QString name, item_types type, int source_id, bool isAbstract=false);//returns its id if already present
inline long long insert_description(QString name,item_types type);
//TODO: change the order of the parameters to have those related only to stems last; but dont forget to change also the calls to this function accordingly
inline long insert_item(item_types type,QString name, QString raw_data, QString category, int source_id, QList<long> abstract_ids =QList<long>(), QString description="", QString POS="",QString grammar_stem="",QString lemma_ID="");
//let dispay table return the number of rows in the table
inline long display_table(QString table); //TODO: has some error in producing sources for example may result in "3,0,0" and also in rules type may result in "AA" always
inline int insert_source(QString name, QString normalization_process, QString creator); //returns current number of sources
inline int insert_compatibility_rules(rules rule, long id1,long id2, long result_id, int source_id);
inline int insert_compatibility_rules(rules rule,QString category1,QString category2, QString category_result, int source_id);
inline int insert_compatibility_rules(rules rule,QString category1,QString category2, int source_id);
inline int insert_compatibility_rules(rules rule, long id1,long id2, int source_id);
inline bool areCompatible(rules rule,long category1,long category2, long& resulting_category);
inline bool areCompatible(rules rule,long category1,long category2);


#endif

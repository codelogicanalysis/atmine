#ifndef SQL_INTERFACE_H
#define SQL_INTERFACE_H

#include "logger.h"
#include "common.h"

#include "dbitvec.h"
#include <QString>
#include <QSqlDatabase>
#include <QTextStream>
#include <QDebug>
#include <QSqlQuery>
#include <QVariant>
#include "ATMProgressIFC.h"

using namespace std;

extern QSqlDatabase db;

#define perform_query(stmt)  \
	if (!execute_query(stmt)) \
		return -1;

extern int source_ids[max_sources+1];//here last element stores number of filled entries in the array
extern int abstract_category_ids[max_sources+1];//here last element stores number of filled entries in the array

bool execute_query(QString stmt);
bool execute_query(QString stmt, QSqlQuery &query);

QString interpret_type(item_types t);
QString interpret_type(rules r);
dbitvec bigint_to_bitset(unsigned long long ll);
dbitvec bigint_to_bitset(char * val);
dbitvec bigint_to_bitset(QVariant val);
dbitvec string_to_bitset(QString val);
dbitvec string_to_bitset(QVariant val);
QString bitset_to_string(dbitvec b);
int get_bitindex(int id,int array[]);//very slow, implement differently later
bool start_connection(ATMProgressIFC *); //and do other initializations
void close_connection();
int generate_bit_order(QString table,int array[],QString filter_column="");
long get_abstractCategory_id(int bit);
int get_abstractCategory_id(QString abstract_category);
long get_source_id(int bit);
bool existsID(QString table,unsigned long long id,QString additional_condition ="");
long long getID(QString table, QString name, QString additional_condition="", QString column_name="name");
QString getColumn(QString table, QString column_name, long long id, QString additional_condition="",bool has_id=true);
bool existsSOURCE(int source_id);
dbitvec getSources(QString table,unsigned long long id=-1, QString additional_condition ="", bool has_id=true);
int get_type_of_category(long category_id, item_types & type);
bool get_types_of_rule(rules rule, item_types &t1, item_types &t2);
dbitvec addSource(QString table, int source_id, long long id=-1 , QString additional_condition ="",bool has_id=true);
dbitvec addAbstractCategory(QString primary_condition, int abstract_category_id);
bool addAbstractCategory(QString name, QString raw_data, QString category, int source_id, QList<long> * abstract_ids, QString description="", QString POS="");
long insert_category(QString name, item_types type, dbitvec sources, bool isAbstract=false);//returns its id if already present and if names are equal but others are not, -1 is returned
long insert_category(QString name, item_types type, int source_id, bool isAbstract=false);//returns its id if already present
long long insert_description(QString name,item_types type);
//TODO: change the order of the parameters to have those related only to stems last; but dont forget to change also the calls to this function accordingly
long insert_item(item_types type,QString name, QString raw_data, QString category, int source_id, QList<long> * abstract_ids, QString description="", QString POS="",QString grammar_stem="",QString lemma_ID="");
//let dispay table return the number of rows in the table
long display_table(QString table); //TODO: has some error in producing sources for example may result in "3,0,0" and also in rules type may result in "AA" always
int insert_source(QString name, QString normalization_process, QString creator); //returns current number of sources
int insert_compatibility_rules(rules rule, long id1,long id2, long result_id, int source_id);
int insert_compatibility_rules(rules rule,QString category1,QString category2, QString category_result, int source_id);
int insert_compatibility_rules(rules rule,QString category1,QString category2, int source_id);
int insert_compatibility_rules(rules rule, long id1,long id2, int source_id);
bool areCompatible(rules rule,long category1,long category2, long& resulting_category);
bool areCompatible(rules rule,long category1,long category2);
#endif

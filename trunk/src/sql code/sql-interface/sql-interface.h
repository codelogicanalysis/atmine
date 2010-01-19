#ifndef SQL_INTERFACE_H
#define SQL_INTERFACE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bitset>
#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QStringList>
#include <QDir>
#include <QStringList>
#include <QRegExp>
#include <QList>

#define max_sources 256
using namespace std;

enum rules { AA,AB,AC,BC,CC };
enum item_types { PREFIX, STEM, SUFFIX};

typedef struct minimal_item_info_
{
	item_types type;
	long category_id;
	bitset<max_sources> abstract_categories; //only for STEMS
	QString raw_data;
	QString description;
	QString POS;
} minimal_item_info;
typedef struct all_item_info_
{

	unsigned long long item_id;
	long category_id;
	bitset<max_sources> abstract_categories; //only for STEMS
	bitset<max_sources> sources;
	QString raw_data;
	QString description;
	QString POS;
	QString lemma_ID; //only for STEMs
} all_item_info;

QTextStream out;
QTextStream in;
QTextStream displayed_error;
int source_ids[max_sources]={0};
int abstract_category_ids[max_sources]={0};

bitset<max_sources> INVALID_BITSET;
bool KEEP_OLD=true;
bool warn_about_automatic_insertion =false;
bool display_errors=true;
bool display_warnings=true;

QSqlDatabase db;
QSqlQuery query;

#define error \
	if (display_errors) displayed_error << "ERROR! "
			//... pay attention to put such statements between {..} in if-else structures, otherwise next else would be to this if and not as intended
#define warning \
	if (display_errors && display_warnings) displayed_error << "WARNING! "
		//...
#define perform_query(stmt)  \
	if (!execute_query(stmt)) \
		return -1;


#include <stdio.h>
#include <execinfo.h>
#include <stdlib.h>
#include <assert.h>

void backtrace(void)
{
	void *addresses[20];
	char **strings;

	int size = backtrace(addresses, 20);
	strings = backtrace_symbols(addresses, size);
	qDebug()<<QString("Stack frames: %1").arg( size);
	for(int i = 0; i < size; i++)
	{
		qDebug()<<QString("%1: %2").arg(i).arg( (int)addresses[i]);
		qDebug()<< strings[i];
	}
	free(strings);
}



inline QString interpret_type(item_types t)
{
	switch(t)
	{
	case PREFIX:
		return "prefix";
	case STEM:
		return "stem";
	case SUFFIX:
		return "suffix";
	default:
		error << "UNDEFINED item_type!\n";
		return "--";
	}
}
inline QString interpret_type(rules r)
{
	switch(r)
	{
	case AA:
		return "AA";
	case AB:
		return "AB";
	case AC:
		return "AC";
	case BC:
		return "BC";
	case CC:
		return "CC";
	default:
		error << "UNDEFINED compatibility rule!\n";
		return "--";
	}
}
inline bool execute_query(QString stmt, QSqlQuery &query=query)
{
	if (!query.exec(stmt))
	{
		error <<query.lastError().text()<<"\n"<<"STATEMENT WAS: "<<stmt<<"\n";
		return false;
	}
	return true;
}
inline int generate_bit_order(QString table,int array[max_sources],QString filter_column ="")
{
	QString stmt=QString( "SELECT id FROM %1 %2").arg(table).arg((filter_column==""?QString(""):QString("WHERE %1=1").arg(filter_column)));
	perform_query(stmt);
	int i=0;
	bool ok;
	while (query.next())
	{
		array[i]=query.value(0).toInt(&ok);
		if (ok==false)
		{
			error << "Unexpected Error: Non-integer ID\n";
			return -2; //must not reach here
		}
		i=i+1;
	}
	return i;
}
int get_bitindex(int id,int array[max_sources])
{
	for (int i=0;i<max_sources;i++)
		if (array[i]==id)
			return i;
	error<<"Unexpected Error: id is not part of the ids array\n";
	qDebug()<<"Requested id="<<id;
	backtrace();
	assert(1);
	return max_sources-1;
}
inline bitset<max_sources> bigint_to_bitset(unsigned long long ll)
{
	unsigned long long mask=0x1;
	bitset<max_sources> b;
	for (int i=0 ; i<max_sources; i++)
	{
		b[i]=(mask & ll)!=0;
		mask=mask <<1;
	}
	return b;
}
inline bitset<max_sources> bigint_to_bitset(char * val)
{
	unsigned long long ll;
	sscanf(val,"%llu",&ll);
	return bigint_to_bitset(ll);
}
inline bitset<max_sources> bigint_to_bitset(QVariant val)
{
	bool ok;
	if (val.canConvert(QVariant::ULongLong))
		return bigint_to_bitset(val.toULongLong(&ok));
	return bigint_to_bitset((char *)val.toString().toStdString().data());
}
inline bitset<max_sources> string_to_bitset(QString val)///edit
{
	ushort mask=0x1;
	bitset<max_sources> b;
	b.reset();
	for (int i=0 ; i<max_sources && i<val.length()<<4; i++)
	{
		b[i]=(mask & (ushort)val[i>>4].unicode())!=0;
		mask=mask <<1;
		if (mask==0x0)
			mask=0x1;
	}
	return b;
}
inline bitset<max_sources> string_to_bitset(QVariant val)
{
	return string_to_bitset(val.toString());
}
inline QString bitset_to_string(bitset<max_sources> b)///edit
{
	ushort shift=0;
	QChar val[max_sources>>4];
	for (int i=0 ; i<max_sources>>4; i++)
		val[i]=0;
	for (int i=0 ; i<max_sources; i++)
	{
		val[i>>4]=(val[i>>4].unicode()+((ushort)b[i] << shift));
		shift=shift +1;
		if (shift==16)
			shift=0;
	}
	return QString(val,max_sources>>4);
}
inline bool start_connection() //and do other initializations
{
	/*qDebug()<<db.isOpen()<<db.databaseName();
	if (!db.isOpen())
	{
		if (db.databaseName()!="atm")
		{*/
			db = QSqlDatabase::addDatabase("QMYSQL");
			db.setHostName("localhost");
			db.setDatabaseName("atm");
			db.setUserName("root");
			db.setPassword("");
		//}
		bool ok = db.open();
		if (ok)
		{
			db.exec("SET NAMES 'utf8'");
			QSqlQuery temp(db);
			query=temp;
			INVALID_BITSET.reset();
			INVALID_BITSET.set(max_sources-1);
			generate_bit_order("source",source_ids);
			generate_bit_order("category",abstract_category_ids,"abstract");
			return 0;
		}
		else
		{
			error <<db.lastError().text()<<"\n";
			return 1;
		}
	//}
	return 0;
}
inline void close_connection()
{
		db.close();
		//TODO: must destroy the db before calling the following
		//QSqlDatabase::removeDatabase("atm");

}
inline bool existsID(QString table,unsigned long long id,QString additional_condition ="")
{
	QString stmt( "SELECT * FROM %1 WHERE id ='%2' %3");
	stmt=stmt.arg(table).arg(id).arg((additional_condition==""?additional_condition:"AND "+additional_condition));
	perform_query(stmt);
	return (query.size()>0);
}
inline long long getID(QString table, QString name, QString additional_condition="", QString column_name="name")
{
	bool ok;
	QString stmt( "SELECT id FROM %1 WHERE %4 =\"%2\" %3");
	stmt=stmt.arg(table).arg(name).arg((additional_condition==QString("")?additional_condition:"AND "+additional_condition)).arg(column_name);
	perform_query(stmt);
	if (query.next())
		return query.value(0).toULongLong(&ok);
	else
		return -1;
}
inline QString getColumn(QString table, QString column_name, long long id, QString additional_condition="",bool has_id=true)
{
	QString stmt( "SELECT %4 FROM %1 WHERE %2 %3");
	stmt=stmt.arg(table).arg((has_id?QString("id ='%1'").arg(id):"")).arg((additional_condition==QString("")?additional_condition:(has_id==true?"AND ":"")+additional_condition)).arg(column_name);
	if (!execute_query(stmt))
		return QString::null;
	if (query.next())
		return query.value(0).toString();
	else
	{
		if (has_id) //just not to show this warning for checkCompatibility
		{
			warning << QString("Name not found for given id '%1'\n").arg(id);
		}
		return "";
	}
}
inline bool existsSOURCE(int source_id)
{
	if (source_id<0 || source_id>=max_sources || !existsID("source",source_id))
	{
		error << "INVALID source_id! Unable to perform operation...\n";
		return false;
	}
	else
		return true;
}
inline bitset<max_sources> get_bitset_column(QString table,QString column,unsigned long long id=-1, QString additional_condition ="", bool has_id=true)
{
	QString stmt( "SELECT %4 FROM %1 WHERE %2 %3");
	stmt=stmt.arg(table).arg((has_id?QString("id ='%1'").arg(id):QString(""))).arg((additional_condition==""?additional_condition:(has_id?"AND ":"")+additional_condition)).arg(column);
	if (!execute_query(stmt))
		return INVALID_BITSET; //must not reach here
	if (query.next())
		return string_to_bitset(query.value(0));
	else
		return INVALID_BITSET;
}
inline bitset<max_sources> getSources(QString table,unsigned long long id=-1, QString additional_condition ="", bool has_id=true)
{
	return get_bitset_column(table,"sources",id,additional_condition,has_id);
}
inline int get_type_of_category(long category_id, item_types & type)
{
	bool ok;
	QString stmt=QString("SELECT cast(type as unsigned) from category WHERE id=%1").arg(category_id);
	perform_query(stmt);
	if (query.next())
	{
		type=(item_types)query.value(0).toInt(&ok);
		if (!ok)
		{
			error << "Unexpected Error: Non-integer type\n";
			return -2;
		}
		return 0;
	}
	return -1;
}
inline bool get_types_of_rule(rules rule, item_types &t1, item_types &t2)
{
	switch (rule)
	{
	case AA:
		t1=PREFIX;
		t2=PREFIX;
		break;
	case AB:
		t1=PREFIX;
		t2=STEM;
		break;
	case AC:
		t1=PREFIX;
		t2=SUFFIX;
		break;
	case BC:
		t1=STEM;
		t2=SUFFIX;
		break;
	case CC:
		t1=SUFFIX;
		t2=SUFFIX;
		break;
	default:
		warning << QString("INVALID compatibility rule type!\n");
		return false; //must not reach here
	}
	return true;
}
inline bool update_dates(int source_id)
{
	QString stmt= QString("UPDATE source SET date_last = NOW() WHERE id = '%1'").arg( source_id);
	perform_query(stmt);
	return 0;
}
inline bitset<max_sources> set_index_bitset(QString table,QString column_name, int index, long long id=-1 , QString additional_condition ="",bool has_id=true)
{
	//precondition index is valid
	long long num=0x1;
	num=num<<index;
	QString stmt("UPDATE %3 SET %5= %5 | %2 WHERE %1 %4");
	stmt=stmt.arg((has_id==true?QString("id ='%1'").arg(id):QString(""))).arg(num).arg(table).arg((additional_condition==""?additional_condition:(has_id?"AND ":"")+additional_condition)).arg(column_name);
	if (!execute_query(stmt))
		return INVALID_BITSET; //must not reach here
	return get_bitset_column(table,column_name, id,additional_condition,has_id);
}
inline bitset<max_sources> addSource(QString table, int source_id, long long id=-1 , QString additional_condition ="",bool has_id=true)
{
	if (!existsSOURCE(source_id))
		return INVALID_BITSET;
	int bit_index=get_bitindex(source_id,source_ids);
	if (bit_index>=0 && bit_index<max_sources)
		return set_index_bitset(table,"sources",bit_index,id,additional_condition,has_id);
	else
	{
		error << "Unexpected Error: source_id ="<<source_id<<"\n";
		return INVALID_BITSET;
	}
}
inline bitset<max_sources> addAbstractCategory(QString table, int abstract_category_id, long long id=-1 , QString additional_condition ="",bool has_id=true)
{
	if (!existsID("category",abstract_category_id,QString("abstract=1 AND type =%1").arg((int)(STEM))))
		return INVALID_BITSET;
	int bit_index=get_bitindex(abstract_category_id,abstract_category_ids);
	if (bit_index>=0 && bit_index<max_sources)
		return set_index_bitset(table,"abstract_categories",get_bitindex(abstract_category_id,abstract_category_ids),id,additional_condition,has_id);
	else
	{
		error << "Unexpected Error: abstract_category_id ="<<abstract_category_id<<"\n";
		return INVALID_BITSET;
	}

}
/*inline long long getGrammarStem_id(QString table,int stem_id)
{
	bool ok;
	QString stmt( "SELECT grammar_stem_id FROM %1 WHERE id =%2");
	stmt=stmt.arg(table).arg(stem_id);
	perform_query(stmt);
	if (query.next() && !query.value(0).isNull()) //else null is casted "cleanly" to 0
	{
		long long val=query.value(0).toULongLong(&ok);
		if (ok)
			return val;
		else
			return -1;
	}
	else
		return -1;
}*/
inline int resolve_conflict(QString table, QString column_name, QVariant new_value, QString primary_key_condition, int source_id, bool nonvague_sources=true/*, QString additional_SET_condition=""*/)
{
/*	long long old_value_long;
	QString old_value_string;
#define old_value (longlong?old_value_long:old_value_string) //to be able to use same variable consistently
*/
	QVariant old_value;
	//bool longlong;
	bool isNull=false;
	QString stmt( "SELECT %1 FROM %2 WHERE %3");
	stmt=stmt.arg(column_name).arg(table).arg(primary_key_condition);
	//qDebug() << stmt;
	perform_query(stmt);
	//I assume that such an entry exists
	if (query.size()==0)
	{
		error << "Unexpected Error: No conflict is present since row does not even exist\n";
		return -2;
	}
	if (query.next() && !query.value(0).isNull()) //else null is casted "cleanly" to 0
	{
		/*long long val=query.value(0).toULongLong(&longlong);
		if (longlong)
			old_value= val;
		else
			old_value=query.value(0).toString();*/
		old_value=query.value(0);
	}
	else
		isNull=true;
	if (old_value==new_value)
		addSource(table,source_id,-1,primary_key_condition,false);//just to be generic we assume no column id exist, but even if it is no problem condition can solve this
	else
	{
		if ((KEEP_OLD && !isNull) || (!KEEP_OLD && new_value.isNull()))
		{
			warning << QString("CONFLICT with '%1' in table '%2' at entry satisfying the following condition (%3). KEPT %1 %4 instead of %5\n").arg(column_name).arg(table).arg(primary_key_condition).arg(old_value.toString()).arg(new_value.toString());
		}
		else
		{
			QString additional_SET_condition;
			if (!nonvague_sources)
			{
				long long num=0x1;
				int bit_index=get_bitindex(source_id,source_ids);
				if (bit_index>=0 && bit_index<max_sources)
				{
					num=num<<bit_index;
					additional_SET_condition=QString(",sources= sources | %1").arg(num);
				}
				else
				{
					error << "Unexpected Error: source_id ="<<source_id<<"\n";
					return -4;
				}
			}
			else
			{
				bitset<max_sources> sources;
				int bit_index=get_bitindex(source_id,source_ids);
				if (bit_index>=0 && bit_index<max_sources)
				{
					sources.set(bit_index);
					additional_SET_condition=QString(",sources= '%1'").arg(bitset_to_string(sources));
				}
				else
				{
					error << "Unexpected Error: source_id ="<<source_id<<"\n";
					return -4;
				}
			}
			/*if (additional_SET_condition!="")
				additional_SET_condition=QString(",%1").arg(additional_SET_condition);*/
			stmt= QString("UPDATE %1 SET %2='%3' %4 WHERE %5").arg(table).arg(column_name).arg(new_value.toString()).arg( additional_SET_condition).arg(primary_key_condition);
			//qDebug() << stmt;
			perform_query(stmt);
			if (!isNull)
			{
				warning << QString("CONFLICT with '%1' in table '%2' at entry satisfying the following condition (%3). Replaced %1 %5 instead of %4\n").arg(column_name).arg(table).arg(primary_key_condition).arg(old_value.toString()).arg(new_value.toString());
			}
			else
			{
				warning << QString("UPDATED '%1' in table '%2' at entry satisfying the following condition (%3), from NULL to %1 = '%4'\n").arg(column_name).arg(table).arg(primary_key_condition).arg(new_value.toString());
			}
		}
	}
	return 0;
}
inline long insert_category(QString name, item_types type, bitset<max_sources> sources, bool isAbstract=false)//returns its id if already present
{
	long id=getID("category",name,QString("abstract=%1 AND type=%2").arg((isAbstract?"1":"0")).arg((int)type));
	if (id>=0)
		return id;
	QString stmt( "INSERT INTO category(name,type,sources,abstract) VALUES('%1',%2,'%3',%4)");
	stmt=stmt.arg(name).arg((int)type).arg(bitset_to_string(sources)).arg((isAbstract?"1":"0"));
	perform_query(stmt);
	if (isAbstract)
		if (generate_bit_order("category",abstract_category_ids,"abstract")<0)
		{
			error <<"Unexpected Error: Error generating abstract category array\n";
			return -3;//must not reach here
		}
	return getID("category",name);//get id of inserted
	//maybe must update dates, but I think no need here
}
inline long insert_category(QString name, item_types type, int source_id, bool isAbstract=false)//returns its id if already present
{
	bitset<max_sources> sources;
	int bit_index=get_bitindex(source_id,source_ids);
	if (bit_index>=0 && bit_index<max_sources)
	{
		sources.set(bit_index);
		return insert_category(name,type,sources, isAbstract);
	}
	else
	{
		error << "Unexpected Error: source_id ="<<source_id<<"\n";
		return -4;
	}
}
inline long long insert_description(QString name,item_types type)
{
	QString stmt( "INSERT INTO description(name,type) VALUES('%1',%2)");
	stmt=stmt.arg(name).arg((int)type);
	query.exec(stmt);
	return getID("description",name,QString("type=%1").arg((int)type));//get id of inserted
}
inline long insert_item(item_types type,QString name, QString raw_data, QString category, int source_id, QList<long> abstract_ids=QList<long>(), QString description="", QString POS="",QString grammar_stem="",QString lemma_ID="")
{
	QString table=interpret_type(type);
	QString item_category=QString("%1_category").arg(table);
	if (table=="--")
		return -3; //must not reach here
	if (!existsSOURCE(source_id))
		return -2;
	for (int i=0; i<abstract_ids.count();i++)
		if (abstract_ids[i]==-1 || !existsID("category",abstract_ids[i],QString("abstract=1 AND type =%1").arg((int)(STEM))))
		{
			if (abstract_ids[i]!=-1)
				warning<< QString("Undefined Abstract Category Provided '%1'. Will be ignored\n").arg(abstract_ids[i]);
			abstract_ids.removeAt(i);
			i--;
		}
	QString stmt;
	bitset<max_sources> cat_sources,sources,abstract_categories;
	long long item_id=getID(table,name);
	long long grammar_stem_id =-1;
	if (type==STEM && grammar_stem!="")
		grammar_stem_id=getID(table,grammar_stem);
	else if (type==STEM && grammar_stem=="")
		grammar_stem_id=-1;
	long category_id =getID("category",category, QString("type=%1").arg((int)(type)));
	long long description_id=getID("description",description,QString("type=%1").arg((int)(type)));
	//update sources of category or insert it altogether if not there
	if (category_id==-1)
	{
		int bit_index=get_bitindex(source_id,source_ids);
		if (!(bit_index>=0 && bit_index<max_sources))
		{
			error << "Unexpected Error: source_id ="<<source_id<<"\n";
			return -4;
		}
		cat_sources.set(bit_index);
		category_id=insert_category(category,type,cat_sources);
		if (category_id==-1)
			return -1;
		if (warn_about_automatic_insertion)
			warning << QString("New %2 Category Automatically inserted: '%1'").arg(category).arg(table.toUpper())<<"\n";
	}
	else
	{
		cat_sources=addSource("category",source_id,category_id);
		if (cat_sources==INVALID_BITSET)
		{
			error << "Unexpected Error: Category must exist but this seems not the case\n";
		}
	}
	// if item is there or not
	if (item_id==-1)
	{
		if (type==STEM)
		{
			bitset<max_sources> grammar_stem_sources;
			if (grammar_stem_id!=-1)
			{
				int bit_index=get_bitindex(source_id,source_ids);
				if (!(bit_index>=0 && bit_index<max_sources))
				{
					error << "Unexpected Error: source_id ="<<source_id<<"\n";
					return -4;
				}
				grammar_stem_sources.set(bit_index);
			}
			stmt = "INSERT INTO stem(name, grammar_stem_id,sources)  VALUES('%1',%2, '%3')";
			stmt=stmt.arg(name).arg((grammar_stem_id==-1?QString("NULL"):QString("'%1'").arg(grammar_stem_id))).arg(bitset_to_string(grammar_stem_sources));
		}
		else
			stmt = QString("INSERT INTO %1(name)  VALUES('%2')").arg(table).arg(name);
		perform_query(stmt);
		item_id=getID(table,name);
		int bit_index=get_bitindex(source_id,source_ids);
		if (!(bit_index>=0 && bit_index<max_sources))
		{
			error << "Unexpected Error: source_id ="<<source_id<<"\n";
			return -4;
		}
		sources.set(bit_index);
		for (int i=0; i<abstract_ids.count();i++)
		{
			int bit_index=get_bitindex(abstract_ids[i],abstract_category_ids);
			if (!(bit_index>=0 && bit_index<max_sources))
			{
				error << "Unexpected Error: abstract_id ="<<abstract_ids[i]<<"\n";
				return -4;
			}
			abstract_categories.set(bit_index);
		}
		//later would be updated
	}
	else
	{
		if (type==STEM && grammar_stem_id!=-1)
		{
			if (resolve_conflict("stem","grammar_stem_id",grammar_stem_id,QString("id = %1").arg(item_id),source_id,true)<0)
				return -1;
		}
		QString primary_condition=QString("%1_id = '%2' AND category_id = '%3' AND raw_data='%4' AND description_id = %5 AND POS=\"%6\"").arg(table).arg(item_id).arg(category_id).arg(raw_data).arg(description_id).arg(POS);
		sources=addSource(item_category,source_id,-1,primary_condition,false);
		for (int i=0; i<abstract_ids.count();i++)
			abstract_categories=addAbstractCategory(item_category,abstract_ids[i],-1,primary_condition,false);
		if (sources!=INVALID_BITSET || abstract_categories!=INVALID_BITSET) //assumed to mean row was modified
		{
			//check for conflict in lemmaID only since description_id and POS became now part of the primary key
			if (resolve_conflict(item_category,"lemma_ID",lemma_ID,primary_condition,source_id,false)<0)
				return -1;
			update_dates(source_id);
			return item_id;
		}
		else //row does not exist
		{
			//check if this works ???
			int bit_index=get_bitindex(source_id,source_ids);
			if (!(bit_index>=0 && bit_index<max_sources))
			{
				error << "Unexpected Error: source_id ="<<source_id<<"\n";
				return -4;
			}
			sources.reset();
			sources.set(bit_index);
			abstract_categories.reset();
			for (int i=0; i<abstract_ids.count();i++)
			{
				int bit_index=get_bitindex(abstract_ids[i],abstract_category_ids);
				if (!(bit_index>=0 && bit_index<max_sources))
				{
					error << "Unexpected Error: abstract_id ="<<abstract_ids[i]<<"\n";
					return -4;
				}
				abstract_categories.set(bit_index);
			}
		}
	}
	//add a description entry first
	description_id=insert_description(description,type);
	if (type==STEM)
	{
		stmt="INSERT INTO stem_category(stem_id, category_id, abstract_categories, sources, raw_data, description_id, POS,lemma_ID)  VALUES('%1','%2','%3','%4','%5','%6',\"%7\",\"%8\")";
		stmt=stmt.arg( item_id).arg( category_id).arg( bitset_to_string(abstract_categories)).arg( bitset_to_string(sources)).arg( raw_data).arg( description_id).arg(POS).arg(lemma_ID);
	}
	else
	{
		stmt="INSERT INTO %1_category(%1_id, category_id, sources, raw_data, description_id, POS)  VALUES('%2','%3','%4','%5','%6','%7')";
		stmt=stmt.arg(table).arg( item_id).arg( category_id).arg( bitset_to_string(sources)).arg( raw_data).arg( description_id).arg(POS);
	}
	perform_query(stmt);
	update_dates(source_id);
	return item_id;
}
inline long display_table(QString table)
{

	out<<"--------------------------------------------------\n"<<table<<":\n";
	QString stmt("SHOW COLUMNS FROM %1");
	stmt=stmt.arg(table);
	perform_query(stmt);

	int source_column=-1,type_column=-1,abstract_categories_column=-1, col=-1;
	int columns=query.size();
	while ( query.next())
	{
		col++;
		//qDebug()<<query.value(0).toString();
		out<<(!query.value(0).isNull() ? query.value(0).toString() : "NULL");
		if (query.value(0).toString()=="sources")
			source_column=col;
		if (query.value(0).toString()=="type")
			type_column=col;
		if (query.value(0).toString()=="abstract_categories")
			abstract_categories_column=col;
		out << "\t";
	}
	out<<"\n";

	stmt=QString("SELECT * FROM %1").arg(table);
	perform_query(stmt);
	QSqlRecord row;
	while (query.next())
	{
		row=query.record();
		for (int i=0; i< columns;i++)
		{
			if (i==source_column)
			{
				bitset<max_sources> b=string_to_bitset(row.value(i));
				if (b.count()==0)
					out<<"--------";
				else
					for (int k=0; k<max_sources;k++)
						if (b[k]==1)
							out<<source_ids[k]<<",";
				out<<"\t";
			}
			else if(i==type_column)
			{
				bool ok;
				if (table=="category")
					out<<interpret_type((item_types)row.value(i).toInt(&ok)).toUpper()<<"\t";
				if (table=="compatibility_rules")
					out<<interpret_type((rules)row.value(i).toInt(&ok))<<"\t";
			}
			else if(i==abstract_categories_column)
			{
				bitset<max_sources> b=string_to_bitset(row.value(i));
				if (b.count()==0)
					out<<"--------";
				else
					for (int k=0; k<max_sources;k++)
						if (b[k]==1)
							out<<abstract_category_ids[k]<<",";
				out<<"\t";
			}
			else
				out<<QString("%1\t").arg(!row.value(i).isNull() ? (!(row.value(i).toString()==QString(""))?row.value(i).toString():QString("    ")) : QString("NULL"));
		}
		out<<"\n";
	}
	out<<QString("Rows: %1\n").arg((long)query.size());
	return 0;
}
inline int insert_source(QString name, QString normalization_process, QString creator) //returns current number of sources
{
	QString stmt( "SELECT id FROM source WHERE description =\"%1\"");
	stmt=stmt.arg(name);
	perform_query(stmt);
	if (query.next())
	{
		warning<<"INSERT Operation ignored, since a source with same description exists!\n";
		bool ok;
		int val=query.value(0).toInt(&ok);
		if (ok)
			return val;
		else
		{
			error <<"Unexpected Error: Non-integer ID\n";
			return -1;
		}
	}
	stmt=QString("INSERT INTO source(description, normalization_process, creator, date_start,date_last) VALUES(\"%1\", \"%2\", \"%3\", NOW(), NOW())").arg(name,normalization_process,creator);
	perform_query(stmt);
	if (generate_bit_order("source",source_ids)<1)
		return -1;
	return getID("source",name,"","description");
}
inline int insert_compatibility_rules(rules rule, long id1,long id2, long result_id, int source_id)
{
	QString stmt;
	item_types t1,t2;
	if (!get_types_of_rule(rule,t1,t2))
			return -1;;
	if (id1<0 || !existsID("category",id1,QString("type=%1").arg((int)t1)))
	{
		error<< QString("INVALID %2 Category ID-1 '%1'\n").arg(id1).arg(interpret_type(t1));
		return -1;
	}
	if (id2<0 || !existsID("category",id2,QString("type=%1").arg((int)t2)))
	{
		error<< QString("INVALID %2 Category ID-2 '%1'\n").arg(id2).arg(interpret_type(t2));
		return -1;
	}
	if (t1==t2) //resulting category id is ignored otherwise
	{
		if (result_id<0 || !existsID("category",result_id,QString("type=%1").arg((int)t1)))
		{
			error<< QString("INVALID %2 resulting Category ID '%1'\n").arg(result_id).arg(interpret_type(t1));
			return -1;
		}
	}
	else
		result_id=-1; //-1 will translate to NULL later
	stmt =QString("SELECT resulting_category FROM compatibility_rules WHERE category_id1=%1 AND category_id2=%2 AND type=%3").arg(id1).arg(id2).arg((int)rule);
	perform_query(stmt);
	if (query.next()) //already present
	{
		bool ok;
		long old_result_id=query.value(0).toLongLong(&ok);
		if (query.isNull(0))
			old_result_id=-1;
		if (old_result_id==result_id)
			addSource("compatibility_rules",source_id,-1,QString("category_id1=%1 AND category_id2=%2").arg(id1).arg(id2),false);
		else
		{
			if (KEEP_OLD)
			{
				warning << QString("RESULTING CATEGORY CONFLICT at rule=(%1,%2). KEPT resulting_category_id %3 instead of %4\n").arg(id1).arg(id2).arg(old_result_id).arg(result_id);
			}
			else
			{
				bitset<max_sources> sources;
				int bit_index=get_bitindex(source_id,source_ids);
				if (!(bit_index>=0 && bit_index<max_sources))
				{
					error << "Unexpected Error: source_id ="<<source_id<<"\n";
					return -4;
				}
				sources.set(bit_index);
				stmt= QString("UPDATE compatibility_rules SET resulting_category='%1' ,sources='%2' WHERE category_id1 = '%3' AND category_id2 = '%4'").arg(result_id).arg(bitset_to_string(sources)).arg( id1).arg(id2);
				//qDebug() << stmt;
				perform_query(stmt);
				warning << QString("RESULTING CATEGORY CONFLICT at rule=(%1,%2). REPLACED resulting_category_id %4 by %3\n").arg(id1).arg(id2).arg(old_result_id).arg(result_id);
			}
		}
	}
	else
	{
		bitset<max_sources> sources;
		int bit_index=get_bitindex(source_id,source_ids);
		if (!(bit_index>=0 && bit_index<max_sources))
		{
			error << "Unexpected Error: source_id ="<<source_id<<"\n";
			return -4;
		}
		sources.set(bit_index);
		stmt="INSERT INTO compatibility_rules(category_id1, category_id2, type, sources, resulting_category)  VALUES(%1,%2,%3,'%4',%5)";
		stmt=stmt.arg(id1).arg( id2).arg( (int)(rule)).arg( bitset_to_string(sources)).arg( (result_id==-1?QString("NULL"):QString("%1").arg(result_id)));
		perform_query(stmt);
	}
	update_dates(source_id);
	return 0;
}
inline int insert_compatibility_rules(rules rule,QString category1,QString category2, QString category_result, int source_id)
{
	//maybe better later to check for category type, else error will appear later for an id which the user did not enter
	long id1=getID("category",category1), id2=getID("category",category2),id_r=getID("category",category_result);
	if (id1==-1)
	{
		error << QString("Category 1 Undefined: '%1'\n").arg(category1);
		return -1;
	}
	if (id2==-1)
	{
		error << QString("Category 2 Undefined: '%1'\n").arg(category2);
		return -1;
	}
	if (id_r==-1)
	{
		error << QString("Resulting Category Undefined: '%1'\n").arg(category_result);
		return -1;
	}
	return insert_compatibility_rules(rule,id1,id2,id_r,source_id);
}
inline int insert_compatibility_rules(rules rule,QString category1,QString category2, int source_id)
{
	return insert_compatibility_rules(rule, category1,category2,category2,source_id);
}
inline int insert_compatibility_rules(rules rule, long id1,long id2, int source_id)
{
	return insert_compatibility_rules(rule,id1,id2,id2,source_id);
}
class Search_by_item
{
// do I make a new connection with database for each such class??
private:
	QSqlQuery query;
	item_types type;
	long long id;
	QString name;
	inline bool retrieve_internal(long category_id) //returns just a category but can contain redundancy
	{
		bool ok;
		category_id =query.value(1).toLongLong(&ok); //1=category_id
		if (!ok)
		{
			error << "Unexpected Error: Non-integer category_id's\n";
			return false;
		}
		return true;
	}
	inline bool retrieve_internal(all_item_info &info)
	{
#define toLL() toLongLong(&ok); if (!ok) { error << "Unexpected Error: Non-integer field\n"; return false; }


		bool ok;
		info.item_id=query.value(0).toLL();
		info.category_id =query.value(1).toLL();
		info.sources=string_to_bitset(query.value(2));
		info.raw_data=query.value(3).toString();
		info.POS=query.value(4).toString();
		if (query.value(5).isNull())
			info.description="";
		else
		{
			long long description_id=query.value(5).toLL();
			info.description=getColumn("description","name",description_id);  //uses global query
		}
		if (type==STEM)
		{
			info.abstract_categories=string_to_bitset(query.value(6));
			info.lemma_ID=query.value(7).toString();
		}
		else
		{
			info.abstract_categories.reset();
			info.lemma_ID="";
		}
		return true;
	}
	inline bool retrieve_internal(minimal_item_info &minimal)
	{
		all_item_info all;
		if (!retrieve_internal(all))
			return false;
		minimal.abstract_categories=all.abstract_categories;
		minimal.category_id=all.category_id;
		minimal.description=all.description;
		minimal.POS=all.POS;
		minimal.raw_data=all.raw_data;
		return true;
	}
	/*inline bool execute_query(QString stmt) //just a copy of the global one, bc we need to use the local query and not the global
	{
		if (!query.exec(stmt))
		{
			error <<query.lastError().text()<<"\n";
			return false;
		}
		return true;
	}*/
public:
	Search_by_item(item_types type,long long id)
	{
		QSqlQuery temp(db);
		query=temp;
		this->type=type;
		this->id=id;
		QString table = interpret_type(type);
		if (id!=-1)
		{
			name=getColumn(table,"name",id); //will use the global query
			//QString stmt( "SELECT DISTINCT category_id FROM %1_category WHERE %1_id ='%2' ORDER BY category_id ASC");
			QString stmt( "SELECT %1_id, category_id, sources, raw_data, POS, description_id %3 FROM %1_category WHERE %1_id ='%2' ORDER BY category_id ASC");
			stmt=stmt.arg(table).arg(id).arg((type==STEM?", abstract_categories, lemma_ID":""));
			if (!execute_query(stmt,query)) //will use the local query
				id=-1; //not really, but because an error took place
		}
	}
	Search_by_item(item_types type,QString name)
	{
		QSqlQuery temp(db);
		query=temp;
		this->type=type;
		this->name=name;
		QString table = interpret_type(type);
		//maybe better here get information about GrammarStem, and its sources and save it, but not a problem, or not??
		id=getID(table,name); //will use the global query
		if (id!=-1)
		{
			//QString stmt( "SELECT DISTINCT category_id FROM %1_category WHERE %1_id ='%2' ORDER BY category_id ASC");
			QString stmt( "SELECT %1_id, category_id, sources, raw_data, POS, description_id %3 FROM %1_category WHERE %1_id ='%2' ORDER BY category_id ASC");
			stmt=stmt.arg(table).arg(id).arg((type==STEM?", abstract_categories, lemma_ID":""));
			if (!execute_query(stmt,query)) //will use the local query
				id=-1; //not really, but because an error took place
		}
	}
	inline long long ID()
	{
		return id;
	}
	inline QString Name()
	{
		if (id!=-1)
			return name;
		else
			return QString::null;
	}
	inline int size() //total size and not what is left
	{
		return query.size();
	}
	inline bool retrieve(long &category_id) //returns just a category but can contain redundancy
	{
		if (id !=-1 && query.next())
			return retrieve_internal(category_id);
		else
			return false;
	}
	inline bool retrieve(all_item_info & info)
	{
		if (id !=-1 && query.next())
			return retrieve_internal(info);
		else
			return false;
	}
	inline bool retrieve(minimal_item_info &info)
	{
		if (id !=-1 && query.next())
			return retrieve_internal(info);
		else
			return false;
	}
	inline int retrieve(long category_ids[],int size_of_array)
	{
		if (size_of_array<=0 || id ==-1)
			return 0;
		int i;
		for (i=0; i<size_of_array && query.next();i++)
		{
			if (!retrieve_internal(category_ids[i]))
				return -1;
		}
		return i;
	}
	#define retrieve_info()  \
	{\
		if (size_of_array<=0 || id ==-1) \
				return 0; \
		int i; \
		for (i=0; i<size_of_array && query.next();i++) \
		{ \
			if (!retrieve_internal(info[i])) \
				return -1; \
		} \
		return i; \
	}
	inline int retrieve(minimal_item_info info[], int size_of_array)
	{
		retrieve_info();
	}
	inline int retrieve(all_item_info info[], int size_of_array)
	{
		retrieve_info();
	}
	inline QString getGrammarStem() //just for stems
	{
		if (type==STEM)
		{
			bool ok;
			long long grammar_stem_id= getColumn("stem","grammar_stem_id",id).toLongLong(&ok);
			if (!ok)
			{
				error << "Non-integer Grammar Stem ID\n";
				return "";
			}
			return getColumn("stem","name",grammar_stem_id);
		}
		else
		{
			warning << "getGrammarStem() does not work for PREFIX or SUFFIX\n";
			return "";
		}
	}
	inline bitset<max_sources> getGrammarStem_sources() //just for stems
	{
		if (type==STEM)
			return getSources("stem",id);
		else
		{
			warning << "getGrammarStem_sources() does not work for PREFIX or SUFFIX\n";
			return INVALID_BITSET;
		}
	}
};
class Search_by_category
{
private:
	QSqlQuery query;
	item_types type;
	bool err;
	inline bool retrieve_internal(long long item_id)
	{
		bool ok;
		item_id =query.value(0).toLongLong(&ok);
		if (!ok)
		{
			error << "Unexpected Error: Non-integer item_id's\n";
			return false;
		}
		return true;
	}
public:
	Search_by_category(long category_id)
	{
		err=false;
		QSqlQuery temp(db);
		query=temp;
		get_type_of_category(category_id,type);
		QString table = interpret_type(type);
		QString stmt( "SELECT %1_id FROM %1_category WHERE category_id ='%2' ORDER BY %1_id ASC");
		stmt=stmt.arg(interpret_type(type)).arg(category_id);
		if (!execute_query(stmt,query)) //will use the local query
			err=true;
	}
	inline int size() //total size and not what is left
	{
		return query.size();
	}
	inline bool retrieve(long long &item_id)
	{
		if (!err && query.next())
			return retrieve_internal(item_id);
		else
			return false;
	}
	inline int retrieve(long long item_ids[],int size_of_array)
	{
		if (size_of_array<=0 || !err)
			return 0;
		int i;
		for (i=0; i<size_of_array && query.next();i++)
		{
			if (!retrieve_internal(item_ids[i]))
				return -1;
		}
		return i;
	}
};
class Search_Compatibility
{
private:
	QSqlQuery query;
	rules rule;
	bool err;
	inline bool retrieve_internal(long &category2, long &resulting_category)
	{
		bool ok;
		category2 =query.value(0).toLongLong(&ok);
		if (!ok)
		{
			error << "Unexpected Error: Non-integer category_id's\n";
			return false;
		}
		if (query.value(2).isNull())
			resulting_category=-1;
		else
		{
			resulting_category=query.value(2).toLongLong(&ok);
			if (!ok)
			{
				error << "Unexpected Error: Non-integer category_id's\n";
				return false;
			}
		}
		return true;
	}
public:
	Search_Compatibility(rules rule, long category_id1)
	{
		err=false;
		QSqlQuery temp(db);
		query=temp;
		this->rule=rule;
		item_types t1,t2,t_cat;
		if (!get_types_of_rule(rule,t1,t2))
			err=true;
		get_type_of_category(category_id1,t_cat);
		if (t1!=t_cat)
			err=true;
		QString stmt( "SELECT category_id2, resulting_category FROM compatibility_rules WHERE category_id1 ='%1' AND type=%2");
		stmt=stmt.arg(category_id1).arg((int)rule);
		if (!execute_query(stmt,query)) //will use the local query
			err=true;
	}
	inline int size() //total size and not what is left
	{
		return query.size();
	}
	inline bool retrieve(long &category2, long &resulting_category)
	{
		if (!err && query.next())
			return retrieve_internal(category2,resulting_category);
		else
			return false;
	}
	inline bool retrieve(long &category2)
	{
		long resulting_category;
		return retrieve(category2, resulting_category);
	}
	inline int retrieve(long category2_ids[],long resulting_categorys[],int size_of_array)
	{
		if (size_of_array<=0 || !err)
			return 0;
		int i;
		for (i=0; i<size_of_array && query.next();i++)
		{
			if (!retrieve_internal(category2_ids[i],resulting_categorys[i]))
				return -1;
		}
		return i;
	}
	inline int retrieve(long category2_ids[],int size_of_array)
	{
		long resulting_categorys[size_of_array];
		return retrieve(category2_ids,resulting_categorys,size_of_array);
	}
};
inline bool areCompatible(rules rule,long category1,long category2, long& resulting_category)
{
	QString resulting=getColumn("compatibility_rules","resulting_category",-1,QString("category_id1=%1 AND category_id2=%2 AND type =%3").arg(category1).arg(category2).arg((int)rule),false);
	if (resulting=="")
	{
		resulting_category=-1;
		return false;
	}
	else
	{
		bool ok;
		long result_id;
		if (rule==AA || rule==CC)
		{
			result_id=resulting.toLongLong(&ok);
			if (ok && existsID("category",result_id,QString("type=%1").arg((rule==AA?(int)PREFIX:(int)SUFFIX))))
				resulting_category=result_id;
			else
			{
				warning << "Unknown resulting Category\n";
				resulting_category=-1;
			}
		}
		else
			resulting_category=-1;
		return true;
	}
}
inline bool areCompatible(rules rule,long category1,long category2)
{
	long resulting_id;
	return areCompatible(rule,category1,category2,resulting_id);
}


#endif

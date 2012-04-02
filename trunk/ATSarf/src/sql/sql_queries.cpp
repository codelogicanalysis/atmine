#include <QFile>
#include <QFileInfo>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTextStream>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QStringList>
#include <QList>
#include <assert.h>
#include <iostream>

#include "sql_queries.h"
#include "database_info_block.h"
#include "diacritics.h"
#include "dbitvec.h"

QSqlQuery query;
#ifdef LOAD_FROM_FILE
extern QStringList cacheFileList;
#endif

#if 0
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
#endif

QString interpret_type(item_types t) //TODO: switch into table
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
QString interpret_type(rules r)
{
	static QString typeNameTable[RULES_LAST_ONE]={"AA","AB","AC","BC","CC"};
#if 0
	if (r<0 || r>=RULES_LAST_ONE)
	{
		error << "UNDEFINED compatibility rule!\n";
		return "--";
	}
#endif
	return typeNameTable[r];
}
bool execute_query(QString stmt, QSqlQuery &query)
{
#if 0
	if (stmt.contains('\"'))
	{
		error <<"INVALID QUERY, contains '\"'\n"<<"STATEMENT WAS: "<<stmt<<"\n";
		return false;
	}
#endif
#if 0
	if (!filling)
		qDebug()<<stmt;
#endif
	if (!query.exec(stmt))
	{
		error <<query.lastError().text()<<"\n"<<"STATEMENT WAS: "<<stmt<<"\n";
		return false;
	}
	return true;
}
bool execute_query(QString stmt)
{
	return execute_query(stmt,query);
}
int generate_bit_order(QString table,int array[],QString filter_column)
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
	array[max_sources]=i;
	return i;
}
int append_to_bit_order(int array[],int id)
{
	if ((++array[max_sources])>max_sources)
	{
		error<< "Unexpected Error: Number of elements in the array has passed the limit!\n";
		array[max_sources]=max_sources;
		return max_sources;
	}
	array[array[max_sources]-1]=id;
	return array[max_sources];
}
int get_bitindex(int id,int array[])//very slow, implement differently later
{
	for (int i=0;i<max_sources && i<array[max_sources];i++)//array[max_sources] stores by convention the number of filled entries in the array
		if (array[i]==id)
			return i;
	error<<"Unexpected Error: id is not part of the ids array\n";
        /*qDebug()<<"Requested id="<<id;
	backtrace();
        assert(1);*/
	return max_sources-1;
}
long get_abstractCategory_id(int bit)//-1 is invalid
{
	if (bit<abstract_category_ids[max_sources] && bit>=0)
		return abstract_category_ids[bit];
	else
		return -1;
}
long get_source_id(int bit)
{
	if (bit<source_ids[max_sources] && bit>=0)
		return source_ids[bit];
	else
		return -1;
}
dbitvec bigint_to_bitset(unsigned long long ll)
{
	unsigned long long mask=0x1;
	dbitvec b(max_sources);
	for (int i=0 ; i<max_sources; i++)
	{
		b[i]=(mask & ll)!=0;
		mask=mask <<1;
	}
	return b;
}
dbitvec bigint_to_bitset(char * val)
{
	unsigned long long ll;
	sscanf(val,"%llu",&ll);
	return bigint_to_bitset(ll);
}
dbitvec bigint_to_bitset(QVariant val)
{
	bool ok;
	if (val.canConvert(QVariant::ULongLong))
		return bigint_to_bitset(val.toULongLong(&ok));
	return bigint_to_bitset((char *)val.toString().toStdString().data());
}
dbitvec string_to_bitset(QString val)
{
	ushort mask=0x1;
	dbitvec b(max_sources);
	b.reset();
	int num_bits=val.length()<<4;
	for (int i=0 ; i<max_sources && i<num_bits; i++)
	{
		b.setBit(i,(mask & (ushort)val[i>>4].unicode())!=0);
		mask=mask <<1;
		if (mask==0x0)
			mask=0x1;
	}
#if 0
	if (b.NothingSet())
		qDebug()<<"Nothing Set";
	else
		qDebug()<<"--";
#endif
	return b;
}
dbitvec string_to_bitset(QVariant val)
{
	return string_to_bitset(val.toString());
}
QString bitset_to_string(dbitvec b)
{
	ushort shift=0;
	QString result;
	int num_characters=max_sources>>4;
	QChar val[num_characters];
	for (int i=0 ; i<num_characters; i++)
		val[i]=0;
	int j=0;
	for (int i=0 ; i<max_sources; i++)
	{
		j=j | ((ushort)b[i] << shift);
		shift=shift +1;
		if (shift==16)
		{
			shift=0;
			val[(i>>4)]=j;
			j=0;
		}
	}
	return QString(val,num_characters);
}
bool tried_once=false;
void check_for_staleness()
{
#ifdef LOAD_FROM_FILE
	QFileInfo temp(trie_list_path);
	QDateTime cache_time;
	if (temp.exists())
		cache_time=temp.lastModified();
	else
		return;
	execute_query("SHOW TABLE STATUS");
	while (query.next())
	{
		QDateTime d=query.value(12).toDateTime();
#ifndef IGNORE_EXEC_TIMESTAMP
		if (d>executable_timestamp || d>cache_time)
#else
		if (d>cache_time)
#endif
		{
			QString s,files;
			foreach ( s,cacheFileList)
				files+=s+" ";
			system(QString(QString("rm ") +files).toStdString().data());
			return;
		}
	}
#endif
}
bool start_connection(ATMProgressIFC * p_ifc) //and do other initializations
{
	db = QSqlDatabase::addDatabase("QMYSQL");
	db.setHostName("localhost");
	db.setDatabaseName("atm");
	db.setUserName("root");
	db.setPassword("");
	bool ok = db.open();
	if (ok)
	{
		db.exec("SET NAMES 'utf8'");
		QSqlQuery temp(db);
		query=temp;
		check_for_staleness();
		return 0;
	}
	else
	{
		if (!tried_once)
		{
			system("mysql --user=\"root\" --password=\"\" -e \"create database atm\"");
			system(string("mysql --user=\"root\" --password=\"\" atm <\""+databaseFileName.toStdString()+"\"").c_str());
			tried_once=true;
			start_connection(p_ifc);
		}
		else
		{
			std::cout <<"Unable to build databases. Reason: "<<db.lastError().text().toStdString()<<"\n";
			return 1;
		}
	}
	return 0;
}
void close_connection()
{
		db.close();
		//TODO: must destroy the db before calling the following
		//QSqlDatabase::removeDatabase("atm");

}
bool existsID(QString table,unsigned long long id,QString additional_condition)
{
	QString stmt( "SELECT * FROM %1 WHERE id ='%2' %3");
	stmt=stmt.arg(table).arg(id).arg((additional_condition==""?additional_condition:"AND "+additional_condition));
	perform_query(stmt);
	return (query.size()>0);
}
long long getID(QString table, QString name, QString additional_condition, QString column_name)
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
QString getColumn(QString table, QString column_name, long long id, QString additional_condition,bool has_id)
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
bool existsSOURCE(int source_id)
{
	if (source_id<0 || source_id>=max_sources || !existsID("source",source_id))
	{
		error << "INVALID source_id! Unable to perform operation...\n";
		return false;
	}
	else
		return true;
}
bool existsEntry(QString table,unsigned long long id=-1, QString additional_condition ="", bool has_id=true)
{
	QString stmt( "SELECT count(*) FROM %1 WHERE %2 %3");
	stmt=stmt.arg(table).arg((has_id?QString("id ='%1'").arg(id):QString(""))).arg((additional_condition==""?additional_condition:(has_id?"AND ":"")+additional_condition));
	if (!execute_query(stmt))
		return false; //must not reach here
	if (query.next())
		return query.value(0).toInt()>0;
	else
	{
		qDebug() <<stmt;
		return false;
	}
}
dbitvec get_bitset_column(QString table,QString column,unsigned long long id=-1, QString additional_condition ="", bool has_id=true)
{
	QString stmt( "SELECT %4 FROM %1 WHERE %2 %3");
	stmt=stmt.arg(table).arg((has_id?QString("id ='%1'").arg(id):QString(""))).arg((additional_condition==""?additional_condition:(has_id?"AND ":"")+additional_condition)).arg(column);
	if (!execute_query(stmt))
		return INVALID_BITSET; //must not reach here
	if (query.next())
		return string_to_bitset(query.value(0));
	else
	{
		qDebug() <<stmt;
		return INVALID_BITSET;
	}
}
dbitvec getSources(QString table,unsigned long long id, QString additional_condition, bool has_id)
{
	return get_bitset_column(table,"sources",id,additional_condition,has_id);
}
int get_type_of_category(long category_id, item_types & type)
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
bool get_types_of_rule(rules rule, item_types &t1, item_types &t2)
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
bool update_dates(int source_id)
{
	QString stmt= QString("UPDATE source SET date_last = NOW() WHERE id = '%1'").arg( source_id);
	perform_query(stmt);
	return 0;
}
dbitvec set_index_bitset(QString table,QString column_name, int index, long long id=-1 , QString additional_condition ="",bool has_id=true)
{
	//precondition index is valid
	dbitvec old_bitset(max_sources);
	old_bitset.reset();
	old_bitset=get_bitset_column(table,column_name,id,additional_condition,has_id);
	assert(old_bitset!=INVALID_BITSET);
	old_bitset.setBit(index);//it became new now
	QString str1=bitset_to_string(old_bitset);
	assert (string_to_bitset(str1)==old_bitset);
	QString stmt("UPDATE ");
	stmt.append(table).append(" SET ").append(column_name).append("= '");
	//stmt.append(QString("CAST(b'%1' AS BINARY)").arg(old_bitset.to_string().data()));
	stmt.append(str1);
	stmt.append("' WHERE ").append((has_id==true?QString("id ='%1'").arg(id):QString(""))).append((additional_condition==""?additional_condition:(has_id?" AND ":"")+additional_condition));
	QString where;
	where.append("' WHERE ").append((has_id==true?QString("id ='%1'").arg(id):QString(""))).append((additional_condition==""?additional_condition:(has_id?" AND ":"")+additional_condition));
	if (!execute_query(stmt))
		return INVALID_BITSET; //must not reach here
	dbitvec new_bitset(max_sources);
	new_bitset.reset();
	new_bitset=get_bitset_column(table,column_name,id,additional_condition,has_id);
	assert (new_bitset!=INVALID_BITSET);
#if 0
	if (new_bitset!=old_bitset)
	{
		qDebug() <<where;
		qDebug()<<"Conflict:";
		/*QString s= bitset_to_string(old_bitset);
		QString t= bitset_to_string(new_bitset);
		for (int i=0;i<s.size();i++)
			qDebug()<<s[i];
		for (int i=0;i<t.size();i++)
			qDebug()<<t[i];*/
		assert (bitset_to_string(new_bitset)!=bitset_to_string(old_bitset));
		qDebug()<<"O: "<<old_bitset.to_string().data();
		qDebug()<<"N: "<<new_bitset.to_string().data();
		stmt="SHOW WARNINGS";
		if (!execute_query(stmt))
			return INVALID_BITSET;
		query.next();
		qDebug()<<query.value(2).toString();
	}
#endif
	return old_bitset;//it is actually new now
}
dbitvec addSource(QString table, int source_id, long long id , QString additional_condition,bool has_id)
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
dbitvec addAbstractCategory(QString primary_condition,int abstract_category_id)
{
	if (!existsID("category",abstract_category_id,QString("abstract=1 AND type =%1").arg((int)(STEM))))
		return INVALID_BITSET;
	int bit_index=get_bitindex(abstract_category_id,abstract_category_ids);
	if (bit_index>=0 && bit_index<max_sources)
		return set_index_bitset("stem_category","abstract_categories",bit_index,-1,primary_condition,false);
	else
	{
		error << "Unexpected Error: abstract_category_id ="<<abstract_category_id<<"\n";
		return INVALID_BITSET;
	}

}
bool addAbstractCategory(QString name, QString raw_data, QString category, int source_id, QList<long> * abstract_ids, QString description, QString POS)
{
#if 1 //if alef is first word and different form of alef the rawdata dont insert
	if (raw_data.length()<name.length())
		raw_data=name;
	int i=0;
	if (name.length()>0)
		do
		{
			//qDebug()<<"o:"<<name;
			if (alefs.contains(name[i]))
			{
				QString corresponding_part=getDiacriticword(i,i,raw_data);//must be one letter
				assert(corresponding_part.length()>0);
				if (name[i]!=corresponding_part[0])
				{
					warning<<"Modified '"<<name <<"' with raw data '"<<raw_data<<"' to be strictly equal\n";
					if(!alefs.contains(corresponding_part[0])) {
						qDebug()<<corresponding_part[0]<<" "<<alefs;
						return -1;
					}
					name[i]=corresponding_part[0];
				}
			}
			//qDebug()<<"n:"<<name;
		}while ((i=name.indexOf(' ',i)+1)>0 && i<name.length());
#endif
	QString table="stem";
	QString item_category=QString("%1_category").arg(table);
	if (table=="--")
		return false; //must not reach here
	if (!existsSOURCE(source_id))
		return false;
	if (abstract_ids!=NULL)
	{
		for (int i=0; i<abstract_ids->count();i++)
			if (abstract_ids->operator [](i)==-1 || !existsID("category",abstract_ids->operator [](i),QString("abstract=1 AND type =%1").arg((int)(STEM))))
			{
				if (abstract_ids->operator [](i)!=-1)
					warning<< QString("Undefined Abstract Category Provided '%1'. Will be ignored\n").arg(abstract_ids->at(i));
				abstract_ids->removeAt(i);
				i--;
			}
	}
	QString stmt;
	dbitvec cat_sources(max_sources),sources(max_sources),abstract_categories(max_sources);
	cat_sources.reset();
	sources.reset();
	abstract_categories.reset();
	long long item_id=getID(table,name);
	long category_id =getID("category",category, QString("type=%1").arg((int)(STEM)));
	long long description_id=getID("description",description,QString("type=%1").arg((int)(STEM)));
	//update sources of category or insert it altogether if not there
	//bool new_category=false;
	if (category_id==-1)
	{
		error << "Category does not exist\n";
		return false;
	}
	else
	{
		cat_sources=addSource("category",source_id,category_id);
		if (cat_sources==INVALID_BITSET)
		{
			error << "Unexpected Error: Category must exist but this seems not the case\n";
			return false;
		}
	}

	int bit_index=get_bitindex(source_id,source_ids);
	sources.setBit(bit_index);

	// if item is there or not
	if (item_id==-1)
	{
		error << "Item does not exist\n";
		return false;
	}
	else
	if (abstract_ids!=NULL)
	{
		for (int i=0; i<abstract_ids->count();i++)
		{
			int bit_index=get_bitindex(abstract_ids->operator [](i),abstract_category_ids);
			if (!(bit_index>=0 && bit_index<max_sources))
			{
				error << "Unexpected Error: abstract_id ="<<abstract_ids->operator [](i)<<"\n";
				return false;
			}
			abstract_categories.setBit(bit_index);
		}
	}
	else
	{
		error<< "No categories provided\n";
		return false;
	}

	QString primary_condition=QString("%1_id = '%2' AND category_id = '%3' AND raw_data='%4' AND description_id = %5 AND POS=\"%6\"").arg(table).arg(item_id).arg(category_id).arg(raw_data).arg(description_id).arg(POS);
	if (existsEntry(item_category,-1,primary_condition,false))
	{
		sources=addSource(item_category,source_id,-1,primary_condition,false);
		for (int i=0; i<abstract_ids->count();i++)
			abstract_categories=addAbstractCategory(primary_condition,abstract_ids->at(i));
		assert (sources!=INVALID_BITSET /*|| abstract_categories!=INVALID_BITSET*//*check this change*/); //assumed to mean row was modified
		{
			addSource(item_category,source_id,-1,primary_condition,false);
			update_dates(source_id);
			return true;
		}
	}
	else //row does not exist
	{
		error<< "Entry does not exist\n";
		return false;
	}
	//add a description entry first
	description_id=insert_description(description,STEM);
	perform_query(stmt);
	update_dates(source_id);
	return true;
}
/*long long getGrammarStem_id(QString table,int stem_id)
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
int resolve_conflict(QString table, QString column_name, QVariant new_value, QString primary_key_condition, int source_id, bool nonvague_sources=true/*, QString additional_SET_condition=""*/)
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
	//TODO: change null condition to include empty
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
			addSource(table,source_id,-1,primary_key_condition,false);
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
				dbitvec sources(max_sources);
				sources.reset();
				int bit_index=get_bitindex(source_id,source_ids);
				if (bit_index>=0 && bit_index<max_sources)
				{
					sources.setBit(bit_index);
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
long insert_category(QString name, item_types type, dbitvec sources, bool isAbstract)//returns its id if already present and if names are equal but others are not, -1 is returned
{
	long id=getID("category",name,QString("abstract=%1 AND type=%2").arg((isAbstract?"1":"0")).arg((int)type));
	if (id>=0)
	{
		//TODO: in case the category is found sources must be updated,but for this to take place there must exist a function that adds groups of sources and not just one
		return id;
	}
	QString stmt( "INSERT INTO category(name,type,sources,abstract) VALUES('%1',%2,'%3',%4)");
	stmt=stmt.arg(name).arg((int)type).arg(bitset_to_string(sources)).arg((isAbstract?"1":"0"));
	perform_query(stmt);
	id=getID("category",name);//get id of inserted
	if (isAbstract)
		if (append_to_bit_order(abstract_category_ids,id)<0)
		{
			error <<"Unexpected Error: Error generating abstract category array\n";
			return -3;//must not reach here
		}
	return id;
	//maybe must update dates, but I think no need here
}
long insert_category(QString name, item_types type, int source_id, bool isAbstract)//returns its id if already present
{
	dbitvec sources(max_sources);
	sources.reset();
	int bit_index=get_bitindex(source_id,source_ids);
	if (bit_index>=0 && bit_index<max_sources)
	{
		sources.setBit(bit_index);
		return insert_category(name,type,sources, isAbstract);
	}
	else
	{
		error << "Unexpected Error: source_id ="<<source_id<<"\n";
		return -4;
	}
}
long long insert_description(QString name,item_types type)
{
	if (!existsEntry("description",-1,QString("name= \"%1\" AND type=%2").arg(name).arg((int)type),false))
	{
		QString stmt( "INSERT INTO description(name,type) VALUES('%1',%2)");
		stmt=stmt.arg(name).arg((int)type);
		query.exec(stmt);
	}
	return getID("description",name,QString("type=%1").arg((int)type));//get id of inserted
}
//TODO: change the order of the parameters to have those related only to stems last; but dont forget to change also the calls to this function accordingly
//note: lemmaID for prefix and suffix means 'reverse_description' understood as 0 or 1
long insert_item(item_types type,QString name, QString raw_data, QString category, int source_id, QList<long> * abstract_ids, QString description, QString POS,QString grammar_stem,QString lemma_ID)
{
#if 1 //if alef is first word and different form of alef the rawdata dont insert
	if (raw_data.length()<name.length())
		raw_data=name;
	int i=0;
	//QString reducedName;
	if (type==STEM && name.length()>0) {
		do {
			//qDebug()<<"o:"<<name;
			if (alefs.contains(name[i])) {
				QString corresponding_part=getDiacriticword(i,i,raw_data);//must be one letter
				assert(corresponding_part.length()>0);
				if (name[i]!=corresponding_part[0]){
					warning<<"Modified '"<<name <<"' with raw data '"<<raw_data<<"' to be strictly equal\n";
					assert(alefs.contains(corresponding_part[0]));
					name[i]=corresponding_part[0];
				}
			}
			/*if (i>0 && isNonConnectingLetter(name[i-1])) {
				reducedName=name.remove(i,1);
				//TODO: remove also from raw_data then later call insert_item again
			}*/
			//qDebug()<<"n:"<<name;
		}while ((i=name.indexOf(' ',i)+1)>0 && i<name.length());
	}
#endif
#if 1
	if(!equal(name,raw_data)) {
		error<<"Conflict Lexicon:\t"<<name<<"\t"<<raw_data;
		if (raw_data.endsWith(' ')) {
			do {
				raw_data=raw_data.remove(raw_data.size()-1,1);
			} while (raw_data.endsWith(' '));
			assert(equal(raw_data,name));
		} else {
			name=removeDiacritics(raw_data);
		}
		displayed_error <<" Corrected to:\t"<<name<<"\t"<<raw_data<<"\n";
	}
#endif
	QString table=interpret_type(type);
	QString item_category=QString("%1_category").arg(table);
	if (table=="--") {
		error<<"Table Invalid\n";
		return -3; //must not reach here
	}
	if (!existsSOURCE(source_id)) {
		error<<"Source does not exist\n";
		return -2;
	}
	if (abstract_ids!=NULL)
	{
		for (int i=0; i<abstract_ids->count();i++)
			if (abstract_ids->operator [](i)==-1 || !existsID("category",abstract_ids->operator [](i),QString("abstract=1 AND type =%1").arg((int)(type))))
			{
				if (abstract_ids->operator [](i)!=-1)
					warning<< QString("Undefined Abstract Category Provided '%1'. Will be ignored\n").arg(abstract_ids->at(i));
				abstract_ids->removeAt(i);
				i--;
			}
	}
	QString stmt;
	dbitvec cat_sources(max_sources),sources(max_sources),abstract_categories(max_sources);
	cat_sources.reset();
	sources.reset();
	abstract_categories.reset();
	long long item_id=getID(table,name);
	long long grammar_stem_id =-1;
	if (type==STEM && grammar_stem!="")
		grammar_stem_id=getID(table,grammar_stem);
	else if (type==STEM && grammar_stem=="")
		grammar_stem_id=-1;
	long category_id =getID("category",category, QString("type=%1").arg((int)(type)));
	long long description_id=getID("description",description,QString("type=%1").arg((int)(type)));
	//update sources of category or insert it altogether if not there
	//bool new_category=false;
	if (category_id==-1)
	{
		int bit_index=get_bitindex(source_id,source_ids);
		if (!(bit_index>=0 && bit_index<max_sources))
		{
			error << "Unexpected Error: source_id ="<<source_id<<"\n";
			return -4;
		}
		cat_sources.setBit(bit_index);
		category_id=insert_category(category,type,cat_sources);
		if (category_id==-1) {
			error << "Category was not inserted sucessfully\n";
			return -1;
		}
		if (warn_about_automatic_insertion)
			warning << QString("New %2 Category Automatically inserted: '%1'").arg(category).arg(table.toUpper())<<"\n";
		//new_category=true;
	}
	else
	{
		cat_sources=addSource("category",source_id,category_id);
		if (cat_sources==INVALID_BITSET)
		{
			error << "Unexpected Error: Category must exist but this seems not the case\n";
		}
	}

	int bit_index=get_bitindex(source_id,source_ids);
	sources.setBit(bit_index);

	// if item is there or not
	if (item_id==-1)
	{
		if (type==STEM)
		{
			dbitvec grammar_stem_sources(max_sources);
			grammar_stem_sources.reset();
			if (grammar_stem_id!=-1)
			{
				int bit_index=get_bitindex(source_id,source_ids);
				if (!(bit_index>=0 && bit_index<max_sources))
				{
					error << "Unexpected Error: source_id ="<<source_id<<"\n";
					return -4;
				}
				grammar_stem_sources.setBit(bit_index);
			}
			stmt = "INSERT INTO stem(name, grammar_stem_id,sources)  VALUES('%1',%2, '%3')";
			stmt=stmt.arg(name).arg((grammar_stem_id==-1?QString("NULL"):QString("'%1'").arg(grammar_stem_id))).arg(bitset_to_string(grammar_stem_sources));
		}
		else
			stmt = QString("INSERT INTO %1(name)  VALUES('%2')").arg(table).arg(name);
		perform_query(stmt);
		item_id=getID(table,name);

		if (!(bit_index>=0 && bit_index<max_sources))
		{
			error << "Unexpected Error: source_id ="<<source_id<<"\n";
			return -4;
		}
	}
	else
	{
		if (type==STEM && grammar_stem_id!=-1)
		{
			if (resolve_conflict("stem","grammar_stem_id",grammar_stem_id,QString("id = %1").arg(item_id),source_id,true)<0) {
				error<< "Could not resolve conflict\n";
				return -1;
			}
		}
	}
	if (abstract_ids!=NULL)
	{
		for (int i=0; i<abstract_ids->count();i++)
		{
			int bit_index=get_bitindex(abstract_ids->operator [](i),abstract_category_ids);
			if (!(bit_index>=0 && bit_index<max_sources))
			{
				error << "Unexpected Error: abstract_id ="<<abstract_ids->operator [](i)<<"\n";
				return -4;
			}
			abstract_categories.setBit(bit_index);
		}
	}
	//later would be updated

	QString primary_condition=QString("%1_id = '%2' AND category_id = '%3' AND raw_data='%4' AND description_id = %5 AND POS=\"%6\"").arg(table).arg(item_id).arg(category_id).arg(raw_data).arg(description_id).arg(POS);
	if (existsEntry(item_category,-1,primary_condition,false))
	{
		sources=addSource(item_category,source_id,-1,primary_condition,false);
#if 0
		if (type==STEM)
#endif
			for (int i=0; i<abstract_ids->count();i++)
				abstract_categories=addAbstractCategory(primary_condition,abstract_ids->at(i));
		assert (sources!=INVALID_BITSET /*|| abstract_categories!=INVALID_BITSET*//*check this change*/); //assumed to mean row was modified
		{
			if (type==STEM)
			{
				//check for conflict in lemmaID only since description_id and POS became now part of the primary key
				if (resolve_conflict(item_category,"lemma_ID",lemma_ID,primary_condition,source_id,false)<0) {
					error<< "Could not resolve conflict\n";
					return -1;
				}
			}
			else
			{//TODO: check if what follows is correct
				addSource(item_category,source_id,-1,primary_condition,false);
				//check for conflict in reverse_description only since description_id and POS became now part of the primary key
				if (resolve_conflict(item_category,"reverse_description",QVariant((bool)lemma_ID.toInt()),primary_condition,source_id,false)<0) {
					error<< "Could not resolve conflict\n";
					return -1;
				}
			}
			update_dates(source_id);
			return item_id;
		}
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
		stmt="INSERT INTO %1_category(%1_id, category_id, abstract_categories, sources, raw_data, description_id, POS,reverse_description)  VALUES('%2','%3','%4','%5','%6','%7','%8',%9)";
		stmt=stmt.arg(table).arg( item_id).arg( category_id).arg( bitset_to_string(abstract_categories)).arg( bitset_to_string(sources)).arg( raw_data).arg( description_id).arg(POS).arg(lemma_ID.toInt());
	}
	perform_query(stmt);
	update_dates(source_id);
	return item_id;
}
int remove_item(item_types type,long item_id, QString raw_data, long category_id, long description_id, QString POS)
{
	QString stmt( QString("DELETE FROM %1_category ")+
				  "WHERE %1_id=%2 AND category_id=%3 AND raw_data=\"%4\" "+
						"AND description_id=%5 AND POS=\"%6\"");
	stmt=stmt.arg(interpret_type(type)).arg(item_id).arg(category_id)
			 .arg(raw_data).arg(description_id).arg(POS);
	perform_query(stmt);
	if (query.numRowsAffected()==0)
	{
		//try finding a different id for description (eg the id given is for a prefix while we are looking for suffix)
		QString stmt2( QString("SELECT id FROM description ")+
					  "WHERE name IN (SELECT name FROM description WHERE id=%1) AND type=%2");
		stmt2=stmt2.arg(description_id).arg((int)type);
		perform_query(stmt2);
		long desc_other=description_id;
		while (query.next()) {
			desc_other=query.value(0).toLongLong();
			QString stmt( QString("DELETE FROM %1_category ")+
						  "WHERE %1_id=%2 AND category_id=%3 AND raw_data=\"%4\" "+
								"AND description_id=%5 AND POS=\"%6\"");
			stmt=stmt.arg(interpret_type(type)).arg(item_id).arg(category_id)
					 .arg(raw_data).arg(desc_other).arg(POS);
			perform_query(stmt);
		}
		if (query.numRowsAffected()==0) {
			error << "No entry removed in "+interpret_type(type)+"_category!\n";
			return -2;
		} else {
			description_id=desc_other;
		}
	} else if (query.numRowsAffected()>1) {
		error << "More than one entry in "+interpret_type(type)+"_category removed!\n";
		return -2;
	}
	stmt=QString( "SELECT COUNT(*)")+
				  "FROM %1_category "+
				  "WHERE %1_id=%2";
	stmt=stmt.arg(interpret_type(type)).arg(item_id);
	perform_query(stmt);
	assert(query.next());
	if (query.record().value(0)==0) {
		QString stmt( "DELETE FROM %1 WHERE id=%2");
		stmt=stmt.arg(interpret_type(type)).arg(item_id);
		perform_query(stmt);
	}
	return 0;
}
//let dispay table return the number of rows in the table
long display_table(QString table) //TODO: has some error in producing sources for example may result in "3,0,0" and also in rules type may result in "AA" always
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
				dbitvec b=string_to_bitset(row.value(i));
				if (b.NothingSet())
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
				dbitvec b=string_to_bitset(row.value(i));
				if (b.NothingSet())
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
int insert_source(QString name, QString normalization_process, QString creator) //returns current number of sources
{
	QString stmt("SELECT id FROM source WHERE description =\"%1\"");
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
	int id=getID("source",name,"","description");
	if (append_to_bit_order(source_ids,id)<1)
		return -1;
	return id;
}
int insert_compatibility_rules(rules rule, long id1,long id2, long result_id, QString inflectionRule, int source_id)
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
	if (!inflectionRule.isEmpty())
		assert(t1==t2);
	stmt =QString("SELECT resulting_category FROM compatibility_rules WHERE category_id1=%1 AND category_id2=%2 AND type=%3").arg(id1).arg(id2).arg((int)rule);
	perform_query(stmt);
	if (query.size()>0) {//already present
		bool insert_rule=true;
		while (query.next()) {
			bool ok;
			long old_result_id=query.value(0).toULongLong(&ok);
			if (query.isNull(0))
				old_result_id=-1;
			if (old_result_id==result_id) {
				addSource("compatibility_rules",source_id,-1,QString("category_id1=%1 AND category_id2=%2").arg(id1).arg(id2),false);
				insert_rule=false;
				break;
			} else {
			#ifndef ALLOW_MULTIPLE_RESULTING_CATEGORIES
				if (KEEP_OLD) {
					warning << QString("RESULTING CATEGORY CONFLICT at rule=(%1,%2). KEPT resulting_category_id %3 instead of %4\n").arg(id1).arg(id2).arg(old_result_id).arg(result_id);
				} else {
					dbitvec sources(max_sources);
					sources.reset();
					int bit_index=get_bitindex(source_id,source_ids);
					if (!(bit_index>=0 && bit_index<max_sources)) {
						error << "Unexpected Error: source_id ="<<source_id<<"\n";
						return -4;
					}
					sources.setBit(bit_index);
					stmt= QString("UPDATE compatibility_rules SET resulting_category='%1' ,sources='%2' WHERE category_id1 = '%3' AND category_id2 = '%4'").arg(result_id).arg(bitset_to_string(sources)).arg( id1).arg(id2);
					//qDebug() << stmt;
					perform_query(stmt);
					warning << QString("RESULTING CATEGORY CONFLICT at rule=(%1,%2). REPLACED resulting_category_id %4 by %3\n").arg(id1).arg(id2).arg(old_result_id).arg(result_id);
				}
			#endif
			}
		}
	#ifdef ALLOW_MULTIPLE_RESULTING_CATEGORIES
		if (insert_rule) {
			warning <<QString("MULTIPLE RESULTING CATEGORY at rule=(%1,%2)\n").arg(id1).arg(id2);
			goto insert;
		}
	#endif
	} else {
	insert:
		dbitvec sources(max_sources);
		sources.reset();
		int bit_index=get_bitindex(source_id,source_ids);
		if (!(bit_index>=0 && bit_index<max_sources)) {
			error << "Unexpected Error: source_id ="<<source_id<<"\n";
			return -4;
		}
		sources.setBit(bit_index);
		stmt="INSERT INTO compatibility_rules(category_id1, category_id2, type, sources, resulting_category, inflections)  VALUES(%1,%2,%3,'%4',%5,\"%6\")";
		stmt=stmt.arg(id1).arg( id2).arg( (int)(rule)).arg( bitset_to_string(sources)).arg( (result_id==-1?QString("-1"):QString("%1").arg(result_id))).arg(inflectionRule);
		perform_query(stmt);
	}
	update_dates(source_id);
	return 0;
}
int insert_compatibility_rules(rules rule, long id1,long id2, long result_id, int source_id) {
	return insert_compatibility_rules(rule,id1,id2,result_id,"",source_id);
}
int insert_compatibility_rules(rules rule,QString category1,QString category2, QString category_result, QString inflectionRule, int source_id)
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
	return insert_compatibility_rules(rule,id1,id2,id_r,inflectionRule,source_id);
}
int insert_compatibility_rules(rules rule,QString category1,QString category2, QString category_result, int source_id) {
	return insert_compatibility_rules(rule,category1,category2,category_result,"",source_id);
}
int insert_compatibility_rules(rules rule,QString category1,QString category2, int source_id)
{
	return insert_compatibility_rules(rule, category1,category2,category2,source_id);
}
int insert_compatibility_rules(rules rule, long id1,long id2, int source_id)
{
	return insert_compatibility_rules(rule,id1,id2,id2,source_id);
}
bool areCompatible(rules rule,long category1,long category2, long& resulting_category)
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
			result_id=resulting.toULongLong(&ok);
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
bool areCompatible(rules rule,long category1,long category2)
{
	long resulting_id;
	return areCompatible(rule,category1,category2,resulting_id);
}
#if 0
QHash<QString, int> abstract_id_mapping;
int get_abstractCategory_id(QString abstract_category)
{
	if (abstract_id_mapping.contains(abstract_category))
		return abstract_id_mapping[abstract_category];
	else
	{
		int id=getID("category",abstract_category,"abstract=1","name");
		abstract_id_mapping.insert(abstract_category,id);
		return id;
	}
}
#endif

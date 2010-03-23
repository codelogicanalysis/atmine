#include "sql_queries.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bitset>
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

int source_ids[max_sources+1]={0};//here last element stores number of filled entries in the array
int abstract_category_ids[max_sources+1]={0};//here last element stores number of filled entries in the array

QSqlQuery query;


/*
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

*/

QString interpret_type(item_types t)
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
bool execute_query(QString stmt, QSqlQuery &query)
{
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
int generate_bit_order(QString table,int array[],QString filter_column ="")
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
int get_bitindex(int id,int array[])
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
	if (bit<abstract_category_ids[max_sources] && bit>0)
		return abstract_category_ids[bit];
	else
		return -1;
}
long get_source_id(int bit)
{
	if (bit<source_ids[max_sources] && bit>0)
		return source_ids[bit];
	else
		return -1;
}
bitset<max_sources> bigint_to_bitset(unsigned long long ll)
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
bitset<max_sources> bigint_to_bitset(char * val)
{
	unsigned long long ll;
	sscanf(val,"%llu",&ll);
	return bigint_to_bitset(ll);
}
bitset<max_sources> bigint_to_bitset(QVariant val)
{
	bool ok;
	if (val.canConvert(QVariant::ULongLong))
		return bigint_to_bitset(val.toULongLong(&ok));
	return bigint_to_bitset((char *)val.toString().toStdString().data());
}
bitset<max_sources> string_to_bitset(QString val)
{
	ushort mask=0x1;
	bitset<max_sources> b;
	b.reset();
	int num_bits=val.length()<<4;
	for (int i=0 ; i<max_sources && i<num_bits; i++)
	{
		b[i]=(mask & (ushort)val[i>>4].unicode())!=0;
		mask=mask <<1;
		if (mask==0x0)
			mask=0x1;
	}
	return b;
}
bitset<max_sources> string_to_bitset(QVariant val)
{
	return string_to_bitset(val.toString());
}
QString bitset_to_string(bitset<max_sources> b)
{
	ushort shift=0;
	int num_characters=max_sources>>4;
	QChar val[num_characters];
	for (int i=0 ; i<num_characters; i++)
		val[i]=0;
	for (int i=0 ; i<max_sources; i++)
	{
		val[i>>4]=(val[i>>4].unicode()+((ushort)b[i] << shift));
		shift=shift +1;
		if (shift==16)
			shift=0;
	}
	return QString(val,num_characters);
}
bool start_connection() //and do other initializations
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
bitset<max_sources> get_bitset_column(QString table,QString column,unsigned long long id=-1, QString additional_condition ="", bool has_id=true)
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
bitset<max_sources> getSources(QString table,unsigned long long id, QString additional_condition, bool has_id)
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
bitset<max_sources> set_index_bitset(QString table,QString column_name, int index, long long id=-1 , QString additional_condition ="",bool has_id=true)
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
bitset<max_sources> addSource(QString table, int source_id, long long id , QString additional_condition,bool has_id)
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
bitset<max_sources> addAbstractCategory(QString table, int abstract_category_id, long long id , QString additional_condition ,bool has_id)
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
long insert_category(QString name, item_types type, bitset<max_sources> sources, bool isAbstract)//returns its id if already present and if names are equal but others are not, -1 is returned
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
	bitset<max_sources> sources;
	sources.reset();
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
long long insert_description(QString name,item_types type)
{
	QString stmt( "INSERT INTO description(name,type) VALUES('%1',%2)");
	stmt=stmt.arg(name).arg((int)type);
	query.exec(stmt);
	return getID("description",name,QString("type=%1").arg((int)type));//get id of inserted
}
//TODO: change the order of the parameters to have those related only to stems last; but dont forget to change also the calls to this function accordingly
long insert_item(item_types type,QString name, QString raw_data, QString category, int source_id, QList<long> abstract_ids, QString description, QString POS,QString grammar_stem,QString lemma_ID)
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
	//bool new_category=false;
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
		if (sources!=INVALID_BITSET /*|| abstract_categories!=INVALID_BITSET*//*check this change*/) //assumed to mean row was modified
		{
			if (type==STEM)
			{
				//check for conflict in lemmaID only since description_id and POS became now part of the primary key
				if (resolve_conflict(item_category,"lemma_ID",lemma_ID,primary_condition,source_id,false)<0)
					return -1;
			}
			else
			{//TODO: check if what follows is correct
				addSource(item_category,source_id,-1,primary_condition,false);
			}
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
int insert_source(QString name, QString normalization_process, QString creator) //returns current number of sources
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
	int id=getID("source",name,"","description");
	if (append_to_bit_order(source_ids,id)<1)
		return -1;
	return id;
}
int insert_compatibility_rules(rules rule, long id1,long id2, long result_id, int source_id)
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
		long old_result_id=query.value(0).toULongLong(&ok);
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
int insert_compatibility_rules(rules rule,QString category1,QString category2, QString category_result, int source_id)
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



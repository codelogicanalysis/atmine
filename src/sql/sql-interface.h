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

#define max_sources 64
using namespace std;

enum rules { AA,AB,AC,BC,CC };
enum item_types { PREFIX, STEM, SUFFIX};

typedef struct minimal_item_info_
{
	long category_id;
	long semantical_category_id;
	QString raw_data;
	QString description;
	QString POS;
} minimal_item_info;
typedef struct all_item_info_
{
	unsigned long long id;
	long category_id;
	long semantical_category_id;
	bitset<max_sources> sources;
	QString raw_data;
	QString description;
	QString POS;
} all_item_info;

QTextStream out/*(stdout)*/;
QTextStream in/*(stdin)*/;
QTextStream displayed_error;

bool KEEP_OLD=true;
bool display_errors=true;
bool display_warnings=true;

QSqlDatabase db;
QSqlQuery query;

#define error \
	if (display_errors) displayed_error
			//... pay attention to put such statements between {..} in if-else structures, otherwise next else would be to this if and not as intended
#define warning \
	if (display_errors && display_warnings) displayed_error
		//...
#define perform_query(stmt)  \
	if (!execute_query(stmt)) \
		return -1;

inline QString interpret_type(item_types t)
{
	qDebug()<<t;
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
inline bool execute_query(QString stmt)
{
	if (!query.exec(stmt))
	{
		error <<query.lastError().text()<<"\n";
		return false;
	}
	return true;
}
bitset<max_sources> binary_to_bitset(unsigned long long ll)
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
bitset<max_sources> binary_to_bitset(char * val)
{

	//printf("%s\n",val);
	unsigned long long ll;
	sscanf(val,"%llu",&ll);
	return binary_to_bitset(ll);

	/*//printf("%llu\n",ll);
	unsigned long l1,l2;
	l1=ll;
	for (int i=0; i<max_sources/2;i++)
			l2=ll/2;

	//printf("%lu,%lu\n",l1,l2);
	bitset<max_sources/2> b1(l1);
	bitset<max_sources/2> b2(l1);
	char * bitstring=(char *)malloc(64*sizeof(char));
	sprintf( bitstring, "%s%s",b1.to_string().data(),b2.to_string().data());
	string temp=bitstring;
	bitset<max_sources> b(temp);

	//printf("%s\n",b.to_string().data());*/
}
bitset<max_sources> binary_to_bitset(QVariant val)
{
	bool ok;
	if (val.canConvert(QVariant::ULongLong))
		return binary_to_bitset(val.toULongLong(&ok));
	return binary_to_bitset((char*)val.toString().toStdString().data());
}
inline bool start_connection()
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
		qDebug()<<db.isOpen();
		//QSqlDatabase::removeDatabase("atm");

}
inline bool existsID(char * table,unsigned long long id,QString additional_condition ="")
{
	QString stmt( "SELECT * FROM %1 WHERE id ='%2' %3");
	stmt=stmt.arg(table).arg(id).arg((additional_condition==""?additional_condition:"AND "+additional_condition));
	perform_query(stmt);
	return (query.size()>0);
}
inline long long getID(char * table, QString name, QString additional_condition="")
{
	bool ok;
	QString stmt( "SELECT id FROM %1 WHERE name ='%2' %3");
	stmt=stmt.arg(table).arg(name).arg((additional_condition==QString("")?additional_condition:"AND "+additional_condition));
	perform_query(stmt);
	if (query.next())
		return query.value(0).toULongLong(&ok);
	else
		return -1;
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
inline bitset<max_sources> getSources(char * table,unsigned long long id=-1, QString additional_condition ="", bool has_id=true)
{
	QString stmt( "SELECT sources FROM %1 WHERE %2 %3");
	stmt=stmt.arg(table).arg((has_id?QString("id ='%1'").arg(id):QString(""))).arg((additional_condition==""?additional_condition:(has_id?"AND ":"")+additional_condition));
	perform_query(stmt);
	if (query.next())
		return binary_to_bitset(query.value(0));
	else
		return NULL;
}
inline bool update_dates(int source_id)
{
	QString stmt= QString("UPDATE source SET date_last = NOW() WHERE id = '%1'").arg( source_id);
	perform_query(stmt);
	return 0;
}
inline bitset<max_sources> addSource(char * table, int source_id, long long id=-1 , QString additional_condition ="",bool has_id=true)
{
	if (!existsSOURCE(source_id))
		return NULL;
	long long num=0x1;
	num=num<<source_id;
	QString stmt("UPDATE %3 SET sources= sources | %2 WHERE %1 %4");
	stmt=stmt.arg((has_id==true?QString("id ='%1'").arg(id):QString(""))).arg(num).arg(table).arg((additional_condition==""?additional_condition:(has_id?"AND ":"")+additional_condition));
	perform_query(stmt);
	return getSources(table, id,additional_condition,has_id);
}
inline long long getGrammarStem_id(char * table,int stem_id)
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
}
inline long insert_category(QString name, item_types type, bitset<max_sources> sources)
{
	QString stmt( "INSERT INTO category(name,type,sources) VALUES('%1',%2,b'%3')");
	stmt=stmt.arg(name).arg((int)type).arg(QString(sources.to_string().data()));
	perform_query(stmt);
	return getID("category",name);//get id of inserted
	//maybe must update dates, but I think no need here
}
inline long insert_semantical_category(QString name)
{
	QString stmt( "INSERT INTO category(name) VALUES('%1')");
	stmt=stmt.arg(name);
	perform_query(stmt);
	return getID("semantical_category",name);//get id of inserted
}
inline long insert_item(item_types type,QString name, QString raw_data, QString category, int source_id, long semantical_category_id=-1,QString grammar_stem="", QString description="", QString POS="")
{
	QString table=interpret_type(type);
	QString item_category=QString("%1_category").arg(table);
	if (table=="--")
		return -3; //must not reach here
	if (!existsSOURCE(source_id))
		return -2;
	if (semantical_category_id!=-1 && !existsID("semantical_category",semantical_category_id))
	{
		error<< "Undefined Semantical Category Provided. Assumed -1\n";
		semantical_category_id=-1;
	}
	QString stmt;
	bitset<max_sources> cat_sources,sources;
	long long item_id=getID((char*)table.toStdString().data(),name);
	long long grammar_stem_id =-1;
	if (type==STEM)
		grammar_stem_id=getID((char*)table.toStdString().data(),grammar_stem);
	long category_id =getID("category",category, QString("type=%1").arg((int)(type)));
	//update sources of category or insert it altogether if not there
	if (category_id==-1)
	{
		cat_sources.set(source_id);
		category_id=insert_category(category,type,cat_sources);
		if (category_id==-1)
			return -1;
		warning << QString("New Category Automatically inserted: '%1'").arg(category)<<"\n";
	}
	else
		cat_sources=addSource("category",source_id,category_id);
	// if item is there or not
	if (item_id==-1)
	{
		if (type==STEM)
		{
			bitset<max_sources> grammar_stem_sources;
			if (grammar_stem_id!=-1)
				grammar_stem_sources.set(source_id);
			stmt = "INSERT INTO stem(name, grammar_stem_id,sources)  VALUES('%1',%2, b'%3')";
			stmt=stmt.arg(name).arg((grammar_stem_id==-1?QString("NULL"):QString("'%1'").arg(grammar_stem_id))).arg(grammar_stem_sources.to_string().data());
		}
		else
		{
			stmt = "INSERT INTO %1(name)  VALUES('%2')";
			stmt=stmt.arg(table).arg(name);
		}
		perform_query(stmt);
		item_id=getID((char*)table.toStdString().data(),name);
		sources.set(source_id);
		//later would be updated
	}
	else
	{
		if (type==STEM && grammar_stem_id!=-1)
		{
			long long old_grammar_stem_id=getGrammarStem_id("stem",item_id);
			if (old_grammar_stem_id==grammar_stem_id)
				addSource("stem",source_id,item_id);
			else
			{
				if (KEEP_OLD && old_grammar_stem_id!=-1)
				{
					warning << QString("GRAMMAR STEM CONFLICT at stem_id=%1. KEPT grammar_stem_id %2 instead of %3\n").arg(item_id).arg(old_grammar_stem_id).arg(grammar_stem_id);
				}
				else
				{
					bitset<max_sources> grammar_stem_sources;
					grammar_stem_sources.set(source_id);
					stmt= QString("UPDATE stem SET grammar_stem_id='%1' ,sources=b'%2' WHERE id = '%3'").arg(grammar_stem_id).arg(grammar_stem_sources.to_string().data()).arg( item_id);
					//qDebug() << stmt;
					perform_query(stmt);
					if (old_grammar_stem_id!=-1) //redundant checks
					{
						warning << QString("GRAMMAR STEM CONFLICT at stem_id=%1. REPLACED grammar_stem_id %2 by %3\n").arg(item_id).arg(old_grammar_stem_id).arg(grammar_stem_id);
					}
					else
					{
						warning << QString("UPDATED GRAMMAR STEM at stem_id=%1, from NULL to grammar_stem_id %2 \n").arg(item_id).arg(grammar_stem_id);
					}
				}
			}
		}
		sources=addSource((char*)item_category.toStdString().data(),source_id,-1,QString("%1_id = '%2' AND category_id = '%3' AND semantical_category_id ='%4'").arg(table).arg(item_id).arg(category_id).arg(semantical_category_id),false);
		if (sources!=NULL)
		{
			update_dates(source_id);
			return item_id;
		}
	}
	stmt="INSERT INTO %1_category(%1_id, category_id, semantical_category_id, sources, raw_data, description, POS)  VALUES('%2','%3','%4','%5','%6','%7','%8')";
	stmt=stmt.arg(table).arg( item_id).arg( category_id).arg( semantical_category_id).arg( sources.to_string().data()).arg( raw_data).arg( description).arg(POS);
	perform_query(stmt);
	update_dates(source_id);
	return item_id;
}
inline long display_table(char * table)
{

	out<<"--------------------------------------------------\n"<<table<<":\n";
	QString stmt("SHOW COLUMNS FROM %1");
	stmt=stmt.arg(table);
	perform_query(stmt);

	int source_column=-1,type_column=-1, col=-1;
	int columns=query.size();
	while ( query.next())
	{
		col++;
		//qDebug()<<query.value(0).toString();
		out<<(!query.value(0).isNull() ? query.value(0).toString() : "NULL");
		if (strcmp(query.value(0).toString().toStdString().data(),"sources")==0)
			source_column=col;
		if (strcmp(query.value(0).toString().toStdString().data(),"type")==0)
			type_column=col;
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
				bitset<max_sources> b=binary_to_bitset(row.value(i));
				if (b.count()==0)
					out<<"--------";
				else
					for (int k=0; k<max_sources;k++)
						if (b[k]==1)
							out<<k<<",";
				out<<"\t";
			}
			else if(i==type_column)
			{
				bool ok;
				if (strcmp(table,"category")==0)
					out<<interpret_type((item_types)row.value(i).toInt(&ok)).toUpper()<<"\t";
				if (strcmp(table,"compatibility_rules")==0)
					out<<interpret_type((rules)row.value(i).toInt(&ok))<<"\t";
			}
			else
				out<<QString("%1\t").arg(!row.value(i).isNull() ? (!(row.value(i).toString()==QString(""))?row.value(i).toString():QString("    ")) : QString("NULL"));
		}
		out<<"\n";
	}
	out<<QString("Rows: %1\n").arg((long)query.size());
	return 0;
}
inline int insert_source(QString name, QString normalization_process, QString creator)
{
	QString stmt( "SELECT * FROM source WHERE description ='%1'");
	stmt=stmt.arg(name);
	perform_query(stmt);
	if (query.size()>0)
	{
		error<<"Unable to perform INSERT operation; duplicate descriptions!\n";
		return -2;
	}
	stmt=QString("INSERT INTO source(description, normalization_process, creator, date_start,date_last) VALUES('%1', '%2', '%3', NOW(), NOW())").arg(name,normalization_process,creator);
	perform_query(stmt);
	return 0;
}
inline int insert_compatibility_rules(rules rule, long id1,long id2, long result_id, int source_id)
{
	QString stmt;
	item_types t1,t2;
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
				sources.set(source_id);
				stmt= QString("UPDATE compatibility_rules SET resulting_category='%1' ,sources=b'%2' WHERE category_id1 = '%3' AND category_id2 = '%4'").arg(result_id).arg(sources.to_string().data()).arg( id1).arg(id2);
				//qDebug() << stmt;
				perform_query(stmt);
				warning << QString("RESULTING CATEGORY CONFLICT at rule=(%1,%2). REPLACED resulting_category_id %4 by %3\n").arg(id1).arg(id2).arg(old_result_id).arg(result_id);
			}
		}
	}
	else
	{
		bitset <max_sources> sources;
		sources.set(source_id);
		stmt="INSERT INTO compatibility_rules(category_id1, category_id2, type, sources, resulting_category)  VALUES(%1,%2,%3,b'%4',%5)";
		stmt=stmt.arg(id1).arg( id2).arg( (int)(rule)).arg( sources.to_string().data()).arg( (result_id==-1?QString("NULL"):QString("%1").arg(result_id)));
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
		error << QString("Category 1 Undefined: '%1'").arg(category1);
		return -1;
	}
	if (id2==-1)
	{
		error << QString("Category 2 Undefined: '%1'").arg(category2);
		return -1;
	}
	if (id_r==-1)
	{
		error << QString("Resulting Category Undefined: '%1'").arg(category_result);
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
inline int search(item_types type,QString name,long category_ids[])
{
	QString table = interpret_type(type);
	long long id=getID((char *)table.toStdString().data(),name,QString("type=%1").arg((int)(type)));
	return 0;
}
inline int search(item_types type,QString name,minimal_item_info info[])
{
	return 0;
}
inline int search(item_types type,QString name,all_item_info info[])
{
	return 0;
}
inline int search(QString name,minimal_item_info info[],QString &grammar_stem) //just for stems
{
	return 0;
}
inline int search(QString name,all_item_info info[],QString grammar_stem,bitset<max_sources> grammar_stem_sources) //just for stems
{
	return 0;
}
inline bool areCompatible(rules rules,long category1,long category2, long& resulting_category)
{
	return true;
}

inline bool areCompatible(rules rules,long category1,long category2)
{
	return true;
}
void generate()
{
	for (int i = 0;i<32;i++	)
	{
		out<<"{";
		bitset<6> h(i);
		for (int j=0;j<5;j++)
		{
			if (h[j]==1)
				out<<QString("%1,").arg(j);
		}
		out<<"-1},";
	}
}
int start(QString input_str, QString &output_str, QString &error_str)
{
	out.setString(&output_str);
	out.setCodec("utf-8");
	in.setString(&input_str);
	in.setCodec("utf-8");
	displayed_error.setString(&error_str);
	displayed_error.setCodec("utf-8");
	generate();
#if 0
	bitset<max_sources> sources;
	QString r;
	long id1,id2;
	/*QFile input(args[1]);
	if (!input.open(QIODevice::ReadWrite))
		return 1;
	QTextStream file(&input);
	file.setCodec("utf-8");*/
	out.setString(&output_str);
	out.setCodec("utf-8");
	in.setString(&input_str);
	in.setCodec("utf-8");
	displayed_error.setString(&error_str);
	displayed_error.setCodec("utf-8");
	in >> r>>id1>>id2;
	start_connection();
	//insert_category(h,SUFFIX,sources);
	out<< insert_item(PREFIX,r,"","PRE-",1,2)<<"\n";
	out<< insert_compatibility_rules(AA,id1,id2,id2,1)<<"\n";
	//insert_source("hamza","","");
	display_table("source");
	display_table("category");
	//insert_item(PREFIX,r, "", "",1);
	display_table("prefix");
	display_table("prefix_category");
	display_table("compatibility_rules");
	close_connection();
#endif
	return 0;
}

#endif

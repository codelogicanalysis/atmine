#include <stdio.h>
//#include <mysql.h>
#include <string.h>
#include <stdlib.h>
#include <bitset>
#include <QString>
#include <QStringRef>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QSqlQuery>
#include <QVariant>

QTextStream out(stdout);
QTextStream in(stdin);

#define max_sources 64
using namespace std;

enum rules { aa, ab, bc, ac, cc};
enum category_types { PREFIX, STEM, SUFFIX};

int state;
const char * col_sources="sources";

QSqlDatabase db;
QSqlQuery query;

#define perform_query(stmt)  \
	if (!query.exec(stmt)) \
	{ \
	  qDebug() <<query.lastError().text(); \
	  return -1; \
	}

bitset<max_sources> binary_to_bitset(char * val)
{

	//printf("%s\n",val);
	unsigned long long ll;
	sscanf(val,"%llu",&ll);
	unsigned long long mask=0x1;
	bitset<max_sources> b;
	for (int i=0 ; i<max_sources; i++)
	{
		b[i]=(mask & ll)!=0;
		mask=mask <<1;
	}

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
	return b;
}

bool start_connection()
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
		return 0;
	}
	else
	{
		out <<db.lastError().text();
		return 1;
	}
}

void close_connection()
{
		db.close();
}

bool existsID(char * table,unsigned long long id,QString additional_condition ="")
{
	QString stmt( "SELECT * FROM %1 WHERE id ='%2' %3");
	stmt=stmt.arg(table).arg(id).arg((additional_condition==""?additional_condition:"AND "+additional_condition));
	perform_query(stmt);
	return (query.size()>0);
}

unsigned long long getID(char * table, QString name, QString additional_condition="")
{
	bool ok;
	QString stmt( "SELECT id FROM %1 WHERE name ='%2' %3");
	stmt=stmt.arg(table).arg(name).arg((additional_condition==QString("")?additional_condition:"AND "+additional_condition));
	perform_query(stmt);
	if (query.next())
		return query.value(0).toLongLong(&ok);
	else
		return -1;
}

bitset<max_sources> getSources(char * table, long long id=-1, QString additional_condition ="", bool has_id=true)
{
	QString stmt( "SELECT sources FROM %1 WHERE %2 %3");
	stmt=stmt.arg(table).arg((has_id?QString("id ='%1'").arg(id):QString(""))).arg((additional_condition==""?additional_condition:(has_id?"AND ":"")+additional_condition));
	perform_query(stmt);
	if (query.next())
		return binary_to_bitset((char *)(query.value(0).toString().toStdString().data()));
	else
		return NULL;
}

bool update_dates(int source_id)
{
	QString stmt= QString("UPDATE source SET date_last = NOW() WHERE id = '%1'").arg( source_id);
	perform_query(stmt);
	return 0;
}

bitset<max_sources> addSource(char * table, int source_id, long long id=-1 , QString additional_condition ="",bool has_id=true)
{
	long long num=0x1;
	num=num<<source_id;
	QString stmt("UPDATE %3 SET sources= sources | %2 WHERE %1 %4");
	stmt=stmt.arg((has_id==true?QString("id ='%1'").arg(id):QString(""))).arg(num).arg(table).arg((additional_condition==""?additional_condition:(has_id?"AND ":"")+additional_condition));
	perform_query(stmt);
	return getSources(table, id,additional_condition,has_id);
}

long insert_category(QString name, category_types type, bitset<max_sources> sources)
{
	QString stmt( "INSERT INTO category(name,type,sources) VALUES('%1',%2,b'%3')");
	stmt=stmt.arg(name).arg((int)type).arg(QString(sources.to_string().data()));
	perform_query(stmt);
	return getID("category",name);//get id of inserted
	//maybe must update dates, but I think no need here
}

long insert_semantical_category(QString name)
{
	QString stmt( "INSERT INTO category(name) VALUES('%1')");
	stmt=stmt.arg(name);
	perform_query(stmt);
	return getID("semantical_category",name);//get id of inserted
}

long insert_stem(QString name, QString raw_data, QString grammar_stem, QString syntax_category, long semantical_category_id, int source_id, QString description)
{
	if (!existsID("semantical_category",semantical_category_id))
	{
		qDebug()<< "Undefined Semantical Category Provided. Assumed -1";
		semantical_category_id=-1;
	}
	QString stmt;
	bitset<max_sources> cat_sources,sources;
	long long stem_id=getID("stem",name);
	long long grammar_stem_id =getID("stem",grammar_stem);
	long syntax_category_id =getID("category",syntax_category, QString("type=%1").arg((int)(STEM)));
	//update sources of category or insert it altogether if not there
	if (syntax_category_id==-1)
	{
		cat_sources.set(source_id);
		syntax_category_id=insert_category(syntax_category,STEM,cat_sources);
	}
	else
		cat_sources=addSource("category",source_id,syntax_category_id);
	// if stem is there or not
	if (stem_id==-1)
	{
		stmt = "INSERT INTO stem(name, grammar_stem_id)  VALUES('%1',%2)";
		stmt=stmt.arg(name).arg((grammar_stem_id==-1?QString("NULL"):QString("'%1'").arg(grammar_stem_id)));
		perform_query(stmt);
		stem_id=getID("stem",name);
		sources.set(source_id);
		//later would be updated
	}
	else
	{
		sources=addSource("stem_category",source_id,-1,QString("stem_id = '%1' AND category_id = '%2' AND semantical_category_id ='%3'").arg(stem_id).arg(syntax_category_id).arg(semantical_category_id),false);
		if (sources!=NULL)
		{
			update_dates(source_id);
			return stem_id;
		}
	}
	stmt="INSERT INTO stem_category(stem_id, category_id, semantical_category_id, sources, raw_data, description)  VALUES('%1','%2','%3','%4','%5','%6')";
	stmt=stmt.arg( stem_id).arg( syntax_category_id).arg( semantical_category_id).arg( sources.to_string().data()).arg( raw_data).arg( description);
	perform_query(stmt);
	update_dates(source_id);
	return stem_id;
}

long display_table(char * table)
{

	QString stmt("SHOW COLUMNS FROM %s");
	stmt=stmt.arg(table);
	perform_query(stmt);

	int category_column=-1, col=-1;
	while ( query.next())
	{
		col++;
		qDebug()<<(query.value(0).isNull() ? query.value(0) : "NULL");
		if (strcmp(query.value(0).toString().toStdString().data(),col_sources)==0)
			category_column=col;		
	}
	qDebug()<<"\n";
	/*
	sprintf(stmt, "SELECT * FROM %s", table);
		perform_query(stmt);

	int columns=mysql_num_fields(result);
	while ( ( row=mysql_fetch_row(result)) != NULL )
	{
		for (int i=0; i< columns;i++)
		{
			if (i==category_column)
			{
								bitset<max_sources> b=binary_to_bitset((char*)(row[i]));
				if (b.count()==0)
					printf("--------");
				else
										for (int k=0; k<max_sources;k++)
						if (b[k]==1)
							printf("%d,",k);
				printf("\t");
				//printf("<%s>\t", (row[i] ? row[i] : "NULL"));
			}
			else
				printf("%s\t", (row[i] ? row[i] : "NULL"));
		}
		printf("\n");
	}
		printf("Rows:%d\n",(long)mysql_num_rows(result));
	free(stmt);*/
	return 0;
}
/*
int insert_source(char * name, char * normalization_process, char * creator)
{
	char * stmt=(char *)malloc(128*sizeof(char));;
		sprintf(stmt, "INSERT INTO source(description, normalization_process, creator, date_start,date_last) VALUES('%s', '%s', '%s', NOW(), NOW())", name, normalization_process, creator);
		perform_query(stmt);
	free(stmt);
	return 0;
}

long insert_affix(char * tr_name, char * ar_name, bool is_prefix)
{
	char * stmt=(char *)malloc(128*sizeof(char));
	sprintf(stmt, "INSERT INTO affixes(tr_affix, ar_affix, is_prefix) VALUES('%s', '%s', '%i')", tr_name, ar_name, is_prefix);
		perform_query(stmt);
	free(stmt);
	return 0;
}

bool insert_rules(rules rule, char * n1, char * n2, int source_id)
{
	char * stmt=(char *)malloc(256*sizeof(char));
	sprintf(stmt, "SELECT id from affixes ", tr_name, ar_name, is_prefix);
	state = mysql_query(connection, stmt);
	if (state !=0)
	{
		printf("%s\n",mysql_error(connection));
		return 1;
	}
	free(stmt);
	return 0;
}*/
/*
bool test()
{
	char * stmt=(char *)malloc(128*sizeof(char));
	
	sprintf(stmt, "SELECT cast(category as unsigned) FROM main");
	state = mysql_query(connection, stmt);
	
	if (state !=0)
	{
		printf("%s\n",mysql_error(connection));
		return 1;
	}

	result = mysql_store_result(connection);
	while ( ( row=mysql_fetch_row(result)) != NULL )
	{
				bitset<max_sources> b=binary_to_bitset((char*)row[0]);
				for (int i=0; i<max_sources;i++)
			if (b[i]==1)
				printf("%d,",i);
	}
	free(stmt);
	return 0;
}
*/
int main(int arg_num,char **args)
{
	bitset<max_sources> sources;
	QString h("jad"),r;
	QFile input(args[1]);
	if (!input.open(QIODevice::ReadWrite))
		return 1;
	QTextStream file(&input);
	out.setCodec("utf-8");
	file.setCodec("utf-8");
	file >> r;
	out<< r;
	//printf("%d %s\n",strlen(args[1]),args[1]);
	start_connection();
	//insert_category(h,SUFFIX,sources);
	out<< insert_stem(r,"","","hh",3,2,"bhe");
	qDebug() << getID ("category","jad");
	/*
	insert_source("hamza","","");
		display_table("source");
		display_table("category");
		bitset<max_sources> sources;
	//cat.set();
		insert_stem(args[1], "", "", cat,1,"");
		display_table("stem");
		//test();*/
		close_connection();
	return 0;
}

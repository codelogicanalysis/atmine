#include "Retrieve_Template.h"
#include "sql_queries.h"
#include <assert.h>

typedef QVector<QString> Columns;
typedef QVector<QVariant> Elements;

void Retrieve_Template::intitialize(QString table,Columns columns,QString where){
	this->query=new QSqlQuery (db);
	QString cols;
	for(int i=0;i<columns.count();i++)
	{
		if (i!=0)
			cols.append(" ,");
		cols.append(columns[i]);
	}
	QString stmt( "SELECT %2 FROM %1 %3");
	stmt=stmt.arg(table).arg(cols).arg(where==""?"":QString("WHERE ").append(where) );
	err= (!execute_query(stmt,*(this->query)));
	if (!err)
		this->columns=columns;
}

Retrieve_Template::Retrieve_Template(QString table,Columns columns,QString where)
{
	intitialize(table,columns,where);
}
Retrieve_Template::Retrieve_Template(QString table,QString column, QString where)
{
	Columns list;
	list.clear();
	list.append(column);
	intitialize(table,list,where);
}
Retrieve_Template::Retrieve_Template(QString table,QString column1, QString column2, QString where)
{
	Columns list;
	list.clear();
	list.append(column1);
	list.append(column2);
	intitialize(table,list,where);
}
Retrieve_Template::Retrieve_Template(QString table,QString column1, QString column2, QString column3, QString where)
{
	Columns list;
	list.clear();
	list.append(column1);
	list.append(column2);
	list.append(column3);
	intitialize(table,list,where);
}
Retrieve_Template::Retrieve_Template(QString table,QString column1, QString column2, QString column3, QString column4, QString where){
	Columns list;
	list.clear();
	list.append(column1);
	list.append(column2);
	list.append(column3);
	list.append(column4);
	intitialize(table,list,where);
}

bool Retrieve_Template::retrieve(Elements &elements) //returns just a category but can contain redundancy
{
	if (!err && this->query->next())
	{
		elements.clear();
		for (int i=0;i<columns.count();i++)
			elements.append(this->query->value(i));
		return true;
	}
	else
		return false;
}
bool Retrieve_Template::retrieve()
{
	if (!err && this->query->next())
	{
		return true;
	}
	else
		return false;
}
int Retrieve_Template::size()
{
	return query->size();
}
QVariant Retrieve_Template::get(int index)
{
	//assert (index>=0 && index<size());
	return this->query->value(index);
}
QVariant Retrieve_Template::get(QString col)
{
	int index=columns.indexOf(col);
	assert (index>=0 && index<size());
	return this->query->value(index);
}

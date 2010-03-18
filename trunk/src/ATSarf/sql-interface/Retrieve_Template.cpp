#include "Retrieve_Template.h"
#include "sql_queries.h"
#include <assert.h>

Retrieve_Template::Retrieve_Template(QString table,QVector<QString> columns,QString where)
{
        QSqlQuery temp(db);
        query=temp;
		QString cols;
		for(int i=0;i<columns.count();i++)
		{
			if (i!=0)
				cols.append(" ,");
			cols.append(columns[i]);
		}
		QString stmt( "SELECT %2 FROM %1 %3");
		stmt=stmt.arg(table).arg(cols).arg(where==""?"":QString("WHERE ").append(where) );
		err= (!execute_query(stmt,query));
		if (!err)
			this->columns=columns;
}
Retrieve_Template::Retrieve_Template(QString table,QString column, QString where)
{
	QVector<QString> list;
	list.append(column);
	Retrieve_Template(table,list,where);
}
Retrieve_Template::Retrieve_Template(QString table,QString column1, QString column2, QString where)
{
	QVector<QString> list;
	list.append(column1);
	list.append(column2);
	Retrieve_Template(table,list,where);
}
Retrieve_Template::Retrieve_Template(QString table,QString column1, QString column2, QString column3, QString where)
{
	QVector<QString> list;
	list.append(column1);
	list.append(column2);
	list.append(column3);
	Retrieve_Template(table,list,where);
}
inline bool Retrieve_Template::retrieve(QVector<QVariant> &elements) //returns just a category but can contain redundancy
{
	if (!err && query.next())
	{
		elements.clear();
		for (int i=0;i<columns.count();i++)
			elements.append(query.value(i));
		return true;
	}
	else
		return false;
}
inline bool Retrieve_Template::retrieve()
{
	if (!err && query.next())
	{
		return true;
	}
	else
		return false;
}
inline int Retrieve_Template::size()
{
	return query.size();
}
inline QVariant Retrieve_Template::get(int index)
{
	assert (index>=0 && index<size());
	return query.value(index);

}
inline QVariant Retrieve_Template::get(QString col)
{
	int index=columns.indexOf(col);
	assert (index>=0 && index<size());
	return query.value(index);
}

#ifndef _RETRIEVE_TEMPLATE_H
#define	_RETRIEVE_TEMPLATE_H

#include "sql_queries.h"

class Retrieve_Template
{
private:
	QSqlQuery query;
	QVector<QString> columns;
	bool err;

	void intitialize(QString table,QVector<QString> columns,QString where);
public:
	Retrieve_Template(QString table,QVector<QString> columns, QString where_condition);
	Retrieve_Template(QString table,QString column, QString where_condition);
	Retrieve_Template(QString table,QString column1, QString column2, QString where_condition);
	Retrieve_Template(QString table,QString column1, QString column2, QString column3, QString where_condition);
	int size(); //total size and not what is left
	bool retrieve(QVector<QVariant> &elements);
	bool retrieve();
	QVariant get(int index);
	QVariant get(QString name);
};


#endif	/* _RETRIEVE_TEMPLATE_H */


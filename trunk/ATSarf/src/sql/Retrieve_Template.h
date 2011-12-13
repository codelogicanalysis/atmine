#ifndef _RETRIEVE_TEMPLATE_H
#define	_RETRIEVE_TEMPLATE_H

#include "sql_queries.h"

typedef QVector<QString> Columns;
typedef QVector<QVariant> Elements;

class Retrieve_Template
{
private:
	QSqlQuery * query;
	QVector<QString> columns;
	bool err;

	void intitialize(QString table,Columns columns,QString where);
public:
	Retrieve_Template(QString table,Columns columns, QString where_condition);
	Retrieve_Template(QString table,QString column, QString where_condition);
	Retrieve_Template(QString table,QString column1, QString column2, QString where_condition);
	Retrieve_Template(QString table,QString column1, QString column2, QString column3, QString where_condition);
	Retrieve_Template(QString table,QString column1, QString column2, QString column3, QString column4, QString where_condition);
	int size(); //total size and not what is left
	bool retrieve(Elements &elements);
	bool retrieve();
	QVariant get(int index);
	QVariant get(QString name);
	~Retrieve_Template()
	{
		if (query!=NULL)
			delete query;
	}
};


#endif	/* _RETRIEVE_TEMPLATE_H */


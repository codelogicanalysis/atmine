#ifndef _RETRIEVE_TEMPLATE_H
#define	_RETRIEVE_TEMPLATE_H

#include "sql_queries.h"

class Retrieve_Template
{
private:
	QSqlQuery query;
	QVector<QString> columns;
	bool err;
public:
	Retrieve_Template(QString table,QVector<QString> columns, QString where_condition);
	Retrieve_Template(QString table,QString column, QString where_condition);
	Retrieve_Template(QString table,QString column1, QString column2, QString where_condition);
	Retrieve_Template(QString table,QString column1, QString column2, QString column3, QString where_condition);
	inline int size(); //total size and not what is left
	inline bool retrieve(QVector<QVariant> &elements);
	inline bool retrieve();
	inline QVariant get(int index);
	inline QVariant get(QString name);
};


#endif	/* _RETRIEVE_TEMPLATE_H */


#include <stdio.h>
#include <stdlib.h>
#include "functions.h"
#include "../sql-interface/sql_queries.h"
#include "../logger/logger.h"
#include <QDebug>

int augment()
{
	if (insert_buckwalter()<0)
		return -1;
	if (insert_rules_for_Nprop_Al())
		return -1;
	if (insert_propernames()<0)
		return -1;
	if (insert_placenames()<0)
		return -1;
	return 0;
}


/*
 * Executable to augment disctionary
 */
int main(int , char** ) {//change to err and output of command line or output file

	start_connection();
	QTextStream out_temp(stdout);
	QString output,errors;
	out.setString(&output);
	out.setCodec("utf-8");
	QTextStream err_temp(stderr);
	displayed_error.setString(&errors);
	displayed_error.setCodec("utf-8");
	int result=augment();
	out_temp<<output;
	qDebug()<<"Output\n"<<output;
	err_temp<<"ERRORS:\n"<<errors;
	return (result);
}


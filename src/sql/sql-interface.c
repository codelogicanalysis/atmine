#include <stdio.h>
#include <mysql.h>
#include <string.h>
#include <stdlib.h>
//#inculde <vector.h>

MYSQL_RES *result;
MYSQL_ROW row;
MYSQL *connection, mysql;
int state;

int start_connection()
{
	mysql_init(&mysql);
	connection = mysql_real_connect(&mysql,"localhost","root","","atm",0,0,0);

	if (connection == NULL) 
	{ 
		printf(mysql_error(&mysql)); 
		return 1; 
	}
	return 0;
}

void close_connection()
{
	mysql_free_result(result);
	mysql_close(connection); 
}

int insert_category(char * name)
{
	char * stmt=(char *)malloc(128*sizeof(char));;
	sprintf(stmt, "INSERT INTO categories(name) VALUES('%s')", name);
	state = mysql_query(connection, stmt);
	if (state !=0)
	{
		printf("%s\n",mysql_error(connection));
		return 1;
	}
	free(stmt);
	return 0;
}

/*int insert_word(char * ar_key, char * tr_key, char * stem, bit_vector category, int source_id, char * raw_form)
{
	char * stmt=(char *)malloc(1024*sizeof(char));;
	sprintf(stmt, "INSERT INTO main(ar_key, tr_key, stem, category, source_id, raw_form)  VALUES('%s','%s','%s',%d,'%s')", ar_key, tr_key, stem, category, source_id, raw_form);
	state = mysql_query(connection, stmt);
	if (state !=0)
	{
		printf("%s\n",mysql_error(connection));
		return 1;
	}
	free(stmt);
	return 0;
}*/

int display_table(char * table)
{
	
	char * stmt=(char *)malloc(128*sizeof(char));;
	sprintf(stmt, "SHOW COLUMNS FROM %s", table);
	state = mysql_query(connection, stmt);
	if (state !=0)
	{
		printf("%s\n",mysql_error(connection));
		return 1;
	}
	result = mysql_store_result(connection);

	while ( ( row=mysql_fetch_row(result)) != NULL )
	{
		printf(" %s\t", (row[0] ? row[0] : "NULL"));
	}
	printf("\n");

	sprintf(stmt, "SELECT * FROM %s", table);
	state = mysql_query(connection, stmt);
	
	if (state !=0)
	{
		printf("%s\n",mysql_error(connection));
		return 1;
	}

	result = mysql_store_result(connection);
	int columns=mysql_num_fields(result);
	while ( ( row=mysql_fetch_row(result)) != NULL )
	{
		for (int i=0; i< columns;i++)
			printf(" %s\t", (row[i] ? row[i] : "NULL"));
		printf("\n");
	}
	printf("Rows:%d\n",mysql_num_rows(result));
	free(stmt);
	return 0;
}

int main(int a,char **args)
{
	start_connection();
	insert_category(args[1]);
	display_table("categories");
	display_table("main");
	close_connection();
	return 0;
};

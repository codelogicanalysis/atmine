#include <stdio.h>
#include <mysql.h>
#include <string.h>
#include <stdlib.h>
#include <bitset>
#include <QString>
#include <QStringRef>

#define max_sources 64
using namespace std;

enum rules { aa, ab, bc, ac, cc};
enum category_types { PREFIX, STEM, SUFFIX};
MYSQL_RES *result;
MYSQL_ROW row;
MYSQL *connection, mysql;
int state;
const char * col_sources="sources";

#define perform_update(stmt)  \
        state = mysql_query(connection, stmt); \
        if (state !=0) \
        { \
                printf("%s\n",mysql_error(connection)); \
                return 1; \
        }
#define perform_query(stmt)  \
        perform_update(stmt); \
        result = mysql_store_result(connection);

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

int start_connection()
{
	mysql_init(&mysql);
	connection = mysql_real_connect(&mysql,"localhost","root","","atm",0,0,0);

	if (connection == NULL) 
	{ 
                printf((char *)mysql_error(&mysql));
		return 1; 
	}
	return 0;
}

void close_connection()
{
	mysql_free_result(result);
	mysql_close(connection); 
}

int insert_category(char * name, category_types type, bitset<max_sources> sources)
{
	char * stmt=(char *)malloc(128*sizeof(char));;
        sprintf(stmt, "INSERT INTO category(name,type,sources) VALUES('%s','%d',b'%s')", name,type,sources.to_string().data());
        perform_update(stmt);
        free(stmt);
        return 0;
}

int insert_stem(char * ar_key, char * tr_key, char * stem, bitset<max_sources> category, int source_id, char * raw_form)
{
	char * stmt=(char *)malloc(1024*sizeof(char));;
	sprintf(stmt, "INSERT INTO main(ar_key, tr_key, stem, category, source_id, raw_form)  VALUES('%s','%s','%s',b'%s',%d,'%s')", ar_key, tr_key, stem, category.to_string().data(), source_id, raw_form);
        perform_update(stmt);
	sprintf(stmt, "UPDATE sources SET date_last = NOW() WHERE id = '%d'", source_id);
        perform_update(stmt);
        free(stmt);
        return 0;
}

int display_table(char * table)
{
	
	char * stmt=(char *)malloc(128*sizeof(char));;
	sprintf(stmt, "SHOW COLUMNS FROM %s", table);
        perform_query(stmt);

	int category_column=-1, col=-1;
	while ( ( row=mysql_fetch_row(result)) != NULL )
	{
		col++;
		printf("%s\t", (row[0] ? row[0] : "NULL"));
                if (strcmp((char*)row[0],col_sources)==0)
			category_column=col;
			
	}
	printf("\n");
	/*if (category_column>=0)
		sprintf(stmt, "SELECT id, ar_key, tr_key, stem, cast(category as unsigned), source_id,raw_form FROM %s", table);
	else*/
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
	printf("Rows:%d\n",(int)mysql_num_rows(result));
	free(stmt);
	return 0;
}

int insert_source(char * name, char * normalization_process, char * creator)
{
	char * stmt=(char *)malloc(128*sizeof(char));;
        sprintf(stmt, "INSERT INTO source(description, normalization_process, creator, date_start,date_last) VALUES('%s', '%s', '%s', NOW(), NOW())", name, normalization_process, creator);
        perform_update(stmt);
	free(stmt);
	return 0;
}

int insert_affix(char * tr_name, char * ar_name, bool is_prefix)
{
	char * stmt=(char *)malloc(128*sizeof(char));
	sprintf(stmt, "INSERT INTO affixes(tr_affix, ar_affix, is_prefix) VALUES('%s', '%s', '%i')", tr_name, ar_name, is_prefix);
        perform_update(stmt);
	free(stmt);
	return 0;
}

/*int insert_rules(rules rule, char * n1, char * n2, int source_id)
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

int main(int num,char **args)
{
	printf("%d %s\n",strlen(args[1]),args[1]);
	start_connection();
	//insert_category(args[1]);
	insert_source("hamza","","");
        display_table("source");
        display_table("category");
        bitset<max_sources> cat;
	//cat.set();
        insert_stem(args[1], "", "", cat,1,"");
        display_table("stem");
	//test();
	close_connection();
	return 0;
};

/*
#include "initialize_tool.h"
#include "sql_queries.h"
#include "common.h"
#include "database_info_block.h"

bool sarfStart(QFile * _out, QFile * _displayed_error, ATMProgressIFC * pIFC) {
    try {
        if(_out==NULL) {
            _out=new QFile();
            _out->open(stdout,QIODevice::WriteOnly);
        }
        if(_displayed_error==NULL) {
            _displayed_error = new QFile();
            _displayed_error->open(stderr, QIODevice::WriteOnly);
        }
        if(pIFC==NULL) {
            pIFC = new EmptyProgressIFC();
        }
        theSarf->displayed_error.setDevice(_displayed_error);
        theSarf->displayed_error.setCodec("utf-8");
        theSarf->out.setDevice(_out);
        theSarf->out.setCodec("utf-8");
        initialize_variables();
        start_connection(pIFC);
        generate_bit_order("source",source_ids);
        generate_bit_order("category",abstract_category_ids,"abstract");
        database_info.fill(pIFC);
        return 1;
    }
    catch(const char * ex)  {
            error<<"Fail to initialize tool. Exception is "<<ex<<'.'<<endl;
            return 0;
        }
}

bool sarfExit()
{
    close_connection();
    return 1;
}
*/

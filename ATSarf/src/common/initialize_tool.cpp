#include "initialize_tool.h"
#include "sql_queries.h"
#include "common.h"
#include "database_info_block.h"

bool initialize_tool(QFile * _out, QFile * _displayed_error, ATMProgressIFC * pIFC)
{
    try
    {

        displayed_error.setDevice(_displayed_error);
        out.setDevice(_out);
        initialize_variables();
        start_connection(pIFC);
        generate_bit_order("source",source_ids);
        generate_bit_order("category",abstract_category_ids,"abstract");
        database_info.fill(pIFC);
        return 1;
    }
    catch(const char * ex)  {
            error<<"Fail to initialize tool. Exception is "<<ex<<'.'<<endl;
        }
    return 0;
}

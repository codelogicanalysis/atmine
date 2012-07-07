#include"sarf.h"
#include "database_info_block.h"
#include "sql_queries.h"
#include <QSqlError>
#include <iostream>

Sarf::Sarf() :
        db(*(new QSqlDatabase)), query(*(new QSqlQuery))
{
}

/*
bool
Sarf::start(ATMProgressIFC * pIFC) {
    try {
        out.setString(output_str);
        out.setCodec("utf-8");
        displayed_error.setString(error_str);
        displayed_error.setCodec("utf-8");
        initialize_variables();
        start_connection(pIFC);
        generate_bit_order("source",source_ids);
        generate_bit_order("category",abstract_category_ids,"abstract");
        database_info.fill(pIFC);

    } catch(const char * ex)  {
            error<<"Fail to initialize tool. Exception is "<<ex<<'.'<<endl;
            return false;
    }
    return true;
}
*/

Sarf::~Sarf() {
    if (theSarf == this) {
        theSarf = NULL;
        out << "Destructing Sarf instance. Sarf services are not available until you construct another Sarf instance.";
    }
}

bool
Sarf::start(QFile * _out, QFile * _displayed_error, ATMProgressIFC * pIFC) {
    Sarf * oldSarf = use(this);
    bool rc = true;
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
        displayed_error.setDevice(_displayed_error);
        displayed_error.setCodec("utf-8");
        out.setDevice(_out);
        out.setCodec("utf-8");
        initialize_variables();
        start_connection(pIFC);
        generate_bit_order("source",source_ids);
        generate_bit_order("category",abstract_category_ids,"abstract");
        database_info.fill(pIFC);
    } catch(const char * ex)  {
            error<<"Fail to initialize tool. Exception is "<<ex<<'.'<<endl;
            rc = false;
    }
    if (oldSarf != NULL)
        use(oldSarf);
    return rc;
}

bool
Sarf::exit() {
    try {
        if (theSarf->db.lastError().type() != QSqlError::NoError) {
            std::cerr << theSarf->db.lastError().text().toStdString() << std::endl;
        }

        delete &query;

        if (theSarf->db.lastError().type() !=  QSqlError::NoError) {
            std::cerr << theSarf->db.lastError().text().toStdString() << std::endl;
        }
        db.close();
        if (theSarf->db.lastError().type() != QSqlError::NoError) {
            std::cerr << theSarf->db.lastError().text().toStdString() << std::endl;
        }

        delete &db;

        QSqlDatabase::removeDatabase("atm");
/*
        if (theSarf->db.lastError().type() != NoError) {
            cerr << theSarf->db.lastError().text().toStdString() << endl;
        }
*/
        //TODO: must destroy the db before calling the following
    } catch(const char * ex)  {
        error<<"Fail to exit tool. Exception is "<<ex<<'.'<<endl;
        return false;
    }
    return true;
}

Sarf * theSarf = NULL;

Sarf *
Sarf::use(Sarf * srf){
    Sarf * old = theSarf;
    theSarf = srf;
    return old;
}

/*** The following are the implementations of the private functions of the class Sarf ****/
/*
bool tried_once=false;

bool Sarf::start_connection(ATMProgressIFC * p_ifc) //and do other initializations
{
    if(db.isOpen()) {
        return true;
    }

        db = QSqlDatabase::addDatabase("QMYSQL");
        db.setHostName("localhost");
        db.setDatabaseName("atm");
        db.setUserName("root");
        db.setPassword("");
        bool ok = db.open();
        if (ok)
        {
                db.exec("SET NAMES 'utf8'");
                QSqlQuery temp(db);
                query=temp;
                check_for_staleness();
                return 0;
        }
        else
        {
                if (!tried_once)
                {
                        system("mysql --user=\"root\" --password=\"\" -e \"create database atm\"");
                        system(string("mysql --user=\"root\" --password=\"\" atm <\""+databaseFileName.toStdString()+"\"").c_str());
                        tried_once=true;
                        start_connection(p_ifc);
                }
                else
                {
                        displayed_error<<"Unable to build databases. Reason: ";
                        displayed_error<<db.lastError().text() << "\n";
                        return 1;
                }
        }
        return 0;
}
void Sarf::close_connection()
{
                db.close();
                QSqlDatabase::removeDatabase("atm");
                //TODO: must destroy the db before calling the following
}
*/

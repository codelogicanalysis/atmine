#ifndef SARF_H
#define SARF_H

#include<QSqlDatabase>
#include<QSqlQuery>
#include<QTextStream>
#include <QFile>
#include "ATMProgressIFC.h"

//class QFile;
//class ATMProgressIFC;

class Sarf {
/*
private:
    bool start_connection(ATMProgressIFC *); //and do other initializations
    void close_connection();
*/
public:
    QSqlDatabase & db;
    QSqlQuery & query;
    QTextStream displayed_error;
    QTextStream out;
    QTextStream in;

    Sarf();
    ~Sarf();
    bool start(QFile * _out=NULL, QFile * _displayed_error=NULL, ATMProgressIFC * pIFC=NULL);
    bool start(QString * output_str, QString * error_str, ATMProgressIFC * pIFC);

    bool exit();

    static Sarf * use (Sarf * srf);
};

#endif // SARF_H

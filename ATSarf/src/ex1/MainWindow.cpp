#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "stemmer.h"
#include "POSVerb.h"
//#include "sql_queries.h"
//#include "database_info_block.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow)
{
    /*
    ui->setupUi(this);
    initialize_variables();
    start_connection();
    generate_bit_order("source",source_ids);
    generate_bit_order("category",abstract_category_ids,"abstract");
    database_info.fill();
    */
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    /*
    QString output_str;
    QString input=ui->input->toPlainText();   //get the input text/word from the GUI
    out.setString(&output_str);
    out.setCodec("utf-8");
    //in.setString(&input);
    //in.setCodec("utf-8");
    sarfParameters.enableRunonwords=ui->chk_runon->isChecked();
    QString line=input.split('\n')[0];

    if(ui->morphology->isChecked())
    {
        Stemmer stemmer(&line,0);
        stemmer();
    }
    else if(ui->POSVerb->isChecked())
    {
        POSVerb posverb(&line);
        posverb();
    }
    else if(ui->GlossSFR->isChecked())
    {
        GlossSFR glosssfr(&line);
        glosssfr();
    }
    //out.setString(&output_str);
    ui->output->setText(output_str);
    */
}

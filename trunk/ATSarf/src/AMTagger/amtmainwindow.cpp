#include <QtGui/QApplication>
#include "amtmainwindow.h"
#include "ui_amtmainwindow.h"

AMTMainWindow::AMTMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AMTMainWindow)
{
    ui->setupUi(this);
}

AMTMainWindow::~AMTMainWindow()
{
    delete ui;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AMTMainWindow w;
    w.show();

    return a.exec();
}

/*
int AMT_main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AMTMainWindow w;
    w.show();

    return a.exec();
}
*/

#include "mainwindow.h"
#include "sql-interface.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
{
	m_ui->setupUi(this);
	//m_ui->pushButton->setAutoDefault(false);
	//connect((QObject*)m_ui->pushButton,SIGNAL(clicked()),(QObject*)m_ui,SLOT(do_it()));
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_pushButton_clicked()
{
	m_ui->output->setText("clicked");
	QString error_str,output_str;
	start(m_ui->input->toPlainText(),output_str,error_str);
	m_ui->errors->setText(error_str);
	m_ui->output->setText(output_str);
}

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	MainWindow mainw;
	mainw.show();
	return app.exec();
}

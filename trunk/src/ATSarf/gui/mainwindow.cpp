#include <QFileInfo>
#include "mainwindow.h"
#include "test.h"
#include "../sql-interface/sql_queries.h"
#include "ui_mainwindow.h"
#include "../caching_structures/database_info_block.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
{
	m_ui->setupUi(this);
	m_ui->label_5->setPixmap(QPixmap("./sarf33.bmp"));
	//m_ui->pushButton->setAutoDefault(false);
	//connect((QObject*)m_ui->MainWindow,SIGNAL(destroyed())),(QObject*)m_ui,SLOT(do_it()));
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::report(int value)
{
	if (this==NULL)
		return;
	m_ui->progressBar->setValue(value);
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
	QString error_str,output_str,hadith_str;
	start(m_ui->input->toPlainText(),output_str,error_str,hadith_str,m_ui->checkBox->isChecked(),this);
	m_ui->errors->setText(error_str);
	m_ui->output->setText(output_str);
	m_ui->hadith_display->setText(hadith_str);
#if 0
	if (hadith_str.length()>200)
	{
		QFile f("hadith_chains.txt");
		if (!f.open(QIODevice::WriteOnly))
		{
			throw "Unable to open file";
		}
		QTextStream file(&f);
		file.setCodec("utf-8");
		file<<hadith_str;
		f.close();
	}
#endif
}

/*void MainWindow::on_destroyed()
{
	close_connection();
}*/


int main(int argc, char *argv[])
{
	/*QFileInfo fileinfo(argv[0]);
	executable_timestamp=fileinfo.lastModified();*/
	QApplication app(argc, argv);
	MainWindow mainw;
	start_connection(&mainw);
	mainw.show();
	return app.exec();
}

void MainWindow::on_exit_clicked()
{
	exit(0);
}

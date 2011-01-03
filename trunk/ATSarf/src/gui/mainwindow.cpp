#include <QFileInfo>
#include "mainwindow.h"
#include "test.h"
#include "sql_queries.h"
#include "ui_mainwindow.h"
#include "database_info_block.h"
#include "hadith.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
{
	m_ui->setupUi(this);
	//m_ui->hadith_chain->setPixmap(QPixmap("./sarf33.bmp"));
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
	bool v1,v2,v3;
	parameters.narr_min=m_ui->NARRATOR->toPlainText().toInt(&v1);
	parameters.nmc_max=m_ui->NMC->toPlainText().toInt(&v2);
	parameters.nrc_max=m_ui->NRC->toPlainText().toInt(&v3);
	if (m_ui->checkBox->isChecked() && (!v1 || !v2 || !v3))
	{
		m_ui->errors->setText("Parameters for Hadith Segmentaion are not valid integers!\n");
		return;
	}
	if (!m_ui->chk_testing->isChecked())
		start(m_ui->input->toPlainText(),output_str,error_str,hadith_str,m_ui->checkBox->isChecked(),this);
	else
		test(m_ui->input->toPlainText(),output_str,error_str,hadith_str,m_ui->checkBox->isChecked(),this);
	m_ui->errors->setText(error_str);
	m_ui->output->setText(output_str);
	m_ui->hadith_display->setText(hadith_str);
#ifdef TEST_EQUAL_NARRATORS
	if (m_ui->checkBox->isChecked())
	{
		try{
			system("dot -Tsvg graph.dot -o graph.svg");
			m_ui->hadith_chain->setPixmap(QPixmap("./graph.svg"));
		}
		catch(int i)
		{}
	}
#endif
}
void MainWindow::on_fill_clicked()
{
	initialize_variables();
	database_info.fill(this);
	hadith_initialize();
}

void MainWindow::on_cmd_browse_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open File"), "~/Desktop", tr("All Files (*)"));
	if (!fileName.isEmpty())
		m_ui->input->setText(fileName);
}

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

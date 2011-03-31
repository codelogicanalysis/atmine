#include <QFileInfo>
#include <QScrollArea>
#include "mainwindow.h"
#include "test.h"
#include "sql_queries.h"
#include "ui_mainwindow.h"
#include "database_info_block.h"
#include "hadith.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    browseFileDlg(NULL),
    m_ui(new Ui::MainWindow)
{
	m_ui->setupUi(this);
	m_ui->pushButton->setVisible(false);
	//m_ui->chk_testing->setVisible(false);
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
void MainWindow::setCurrentAction(const QString & s)
{
	if (this==NULL)
		return;
	m_ui->progressBar->setFormat(s+"(%p%)");
}

void MainWindow::resetActionDisplay()
{
	if (this==NULL)
		return;
	m_ui->progressBar->setFormat("%p%");
	m_ui->progressBar->reset();
}

void MainWindow::tag(int start, int length,QColor color, bool textcolor)
{
	if (this==NULL)
		return;
	QTextBrowser * taggedBox=m_ui->hadith_display;
	QTextCursor c=taggedBox->textCursor();
#if 0
	int lastpos=c.position();
	int diff=start-lastpos;
	if (diff>=0)
		c.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,diff);
	else
		c.movePosition(QTextCursor::Left,QTextCursor::MoveAnchor,-diff);
	c.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor,length);
#else
	c.setPosition(start,QTextCursor::MoveAnchor);
	c.setPosition(start+length,QTextCursor::KeepAnchor);
#endif
	taggedBox->setTextCursor(c);
	if (textcolor)
		taggedBox->setTextColor(color);
	else
		taggedBox->setTextBackgroundColor(color);
}

void MainWindow::startTaggingText(QString & text)
{
	if (this==NULL)
		return;
	QTextBrowser * taggedBox=m_ui->hadith_display;
	taggedBox->clear();
	taggedBox->setLayoutDirection(Qt::RightToLeft);
	QTextCursor c=taggedBox->textCursor();
	c.clearSelection();
	c.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
	taggedBox->setTextCursor(c);
	taggedBox->setTextBackgroundColor(Qt::white);
	taggedBox->setTextColor(Qt::black);
	taggedBox->setText(text);

}

void MainWindow::finishTaggingText()
{
	if (this==NULL)
		return;
	QTextBrowser * taggedBox=m_ui->hadith_display;
	QTextCursor c=taggedBox->textCursor();
#if 0
	c.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
#else
	c.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
#endif
	taggedBox->setTextCursor(c);
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
	bool v1,v2,v3,v4,v5,v6;
	parameters.narr_min=m_ui->NARRATOR->toPlainText().toInt(&v1);
	parameters.nmc_max=m_ui->NMC->toPlainText().toInt(&v2);
	parameters.nrc_max=m_ui->NRC->toPlainText().toInt(&v3);
	parameters.equality_delta=m_ui->EQ_delta->toPlainText().toDouble(&v4);
	parameters.equality_radius=m_ui->EQ_radius->toPlainText().toInt(&v5);
	parameters.equality_threshold=m_ui->EQ_threshold->toPlainText().toDouble(&v6);
	parameters.display_chain_num=m_ui->chk_chainNum->isChecked();
	if (m_ui->chk_hadith->isChecked() && (!v1 || !v2 || !v3 || !v4 || !v5 || !v6))
	{
		m_ui->errors->setText("Parameters for Hadith Segmentaion are not valid integers/doubles!\n");
		return;
	}
	int rc;
	if (!m_ui->chk_testing->isChecked())
		rc=start(m_ui->input->toPlainText(),output_str,error_str,hadith_str,m_ui->chk_hadith->isChecked(),this);
	else
		test(m_ui->input->toPlainText(),output_str,error_str,hadith_str,m_ui->chk_hadith->isChecked(),this);
	if (!m_ui->chk_hadith->isChecked())
		m_ui->hadith_display->setText(hadith_str);
	m_ui->errors->setText(error_str);
	m_ui->output->setText(output_str);
#ifdef TEST_NARRATOR_GRAPH
	if (m_ui->chk_hadith->isChecked())
	{
		if (rc==0)
		{
			setWindowTitle(QString("Sarf (")+m_ui->input->toPlainText()+")");
			try{
				system("dot -Tsvg graph.dot -o graph.svg");
				QMainWindow * mw =new QMainWindow(NULL);
				mw->setWindowTitle(QString("Sarf Graph (")+m_ui->input->toPlainText()+")");
				QScrollArea * sa=new QScrollArea(mw);
				mw->setCentralWidget(sa);
				QLabel *pic=new QLabel(sa);
				pic->setPixmap(QPixmap("./graph.svg"));
				sa->setWidget(pic);
				mw->show();
			}
			catch(int i)
			{}
		}
	}
#endif
}
void MainWindow::on_fill_clicked()
{
	initialize_variables();
	database_info.fill(this);
	hadith_initialize();
	m_ui->pushButton->setVisible(true);
	m_ui->fill->setVisible(false);
}

void MainWindow::on_cmd_browse_clicked()
{
	//QString fileName = QFileDialog::getOpenFileName(this,
//		tr("Open File"), "", tr("All Files (*)")); // /home/jad/Desktop/linux
    if (browseFileDlg == NULL) {
        QString dir = QDir::currentPath();
        browseFileDlg = new QFileDialog(this, tr("Open File"), dir, tr("All Files (*)"));
        browseFileDlg->setOptions(QFileDialog::DontUseNativeDialog);
        browseFileDlg->setFileMode(QFileDialog::ExistingFile);
        browseFileDlg->setViewMode(QFileDialog::Detail);
    }
    if (browseFileDlg->exec()){
        QStringList files = browseFileDlg->selectedFiles();
        QString fileName = files[0];
        if (!fileName.isEmpty())
            m_ui->input->setText(fileName);
    }
}

int main(int argc, char *argv[])
{
	/*QFileInfo fileinfo(argv[0]);
	executable_timestamp=fileinfo.lastModified();*/
	QApplication app(argc, argv);
	MainWindow mainw;
#ifndef SUBMISSION
	start_connection(&mainw);
#endif
	mainw.show();
	return app.exec();
}

void MainWindow::on_exit_clicked()
{
	exit(0);
}

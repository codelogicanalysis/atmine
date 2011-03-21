#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include "ATMProgressIFC.h"

namespace Ui {
    class MainWindow;
}
class QFileDialog;

class MainWindow : public QMainWindow,public ATMProgressIFC {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
	virtual void report(int value);
	virtual void tag(int start, int length,QColor color, bool textcolor);
	virtual void startTaggingText(QString & text);
	virtual void finishTaggingText();

protected:
    void changeEvent(QEvent *e);

private:
    QFileDialog * browseFileDlg;
    Ui::MainWindow *m_ui;
	friend int hadith(QString input_str,ATMProgressIFC *prg);

private slots:
	void on_pushButton_clicked();
	void on_fill_clicked();
	void on_exit_clicked();
	void on_cmd_browse_clicked();
	//void on_destroyed();
};

#endif // MAINWINDOW_H
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
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();
	virtual void report(int value);
	virtual void tag(int start, int length,QColor color, bool textcolor);
	virtual void startTaggingText(QString & text);
	virtual void finishTaggingText();
	virtual void setCurrentAction(const QString & s);
	virtual void resetActionDisplay();
	virtual QString getFileName();
	virtual void displayGraph(AbstractGraph * graph);

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
	void on_chk_hadith_toggled(bool checked);
	void on_chk_bible_toggled(bool checked);
	//void on_destroyed();
};

#endif // MAINWINDOW_H

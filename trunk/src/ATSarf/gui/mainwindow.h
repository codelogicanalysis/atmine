#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include "../logger/ATMProgressIFC.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow,public ATMProgressIFC {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
	virtual void report(int value);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *m_ui;

private slots:
	void on_pushButton_clicked();
	void on_destroyed();
};

#endif // MAINWINDOW_H

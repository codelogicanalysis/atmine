#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
	//void start();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *m_ui;

private slots:
	void on_pushButton_clicked();
	void on_destroyed();
};

#endif // MAINWINDOW_H

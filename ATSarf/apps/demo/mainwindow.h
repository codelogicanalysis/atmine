#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <ATMProgressIFC.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public ATMProgressIFC {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();
        virtual void report(int value);
        virtual void tag(int start, int length, QColor color, bool textcolor);
        virtual void startTaggingText(QString &text);
        virtual void finishTaggingText();
        virtual void setCurrentAction(const QString &action);
        virtual void resetActionDisplay();
        virtual QString getFileName();
        virtual void displayGraph(AbstractGraph *graph);

    protected:
        void changeEvent(QEvent *e);

    signals:
        void updateProgress(int);
        void resetProgress();
        void actionUpdate(const QString &);

    private slots:
        void on_initializeButton_clicked();
        void on_updateProgress(int progress);
        void on_resetProgress();
        void on_actionUpdate(const QString &action);

    private:
        Ui::MainWindow *ui;
        QString error_str;
        QString output_str;
        QString hadith_str;
};

#endif // MAINWINDOW_H

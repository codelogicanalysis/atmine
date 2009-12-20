/********************************************************************************
** Form generated from reading ui file 'mainwindow.ui'
**
** Created: Sun Dec 20 00:43:00 2009
**      by: Qt User Interface Compiler version 4.4.3
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QTextBrowser>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QTextEdit *input;
    QTextBrowser *output;
    QTextBrowser *errors;
    QLabel *label;
    QLabel *label_3;
    QLabel *label_2;
    QPushButton *pushButton;

    void setupUi(QMainWindow *MainWindow)
    {
    if (MainWindow->objectName().isEmpty())
        MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
    MainWindow->resize(712, 488);
    centralwidget = new QWidget(MainWindow);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    input = new QTextEdit(centralwidget);
    input->setObjectName(QString::fromUtf8("input"));
    input->setGeometry(QRect(90, 0, 621, 41));
    output = new QTextBrowser(centralwidget);
    output->setObjectName(QString::fromUtf8("output"));
    output->setGeometry(QRect(0, 50, 711, 261));
    errors = new QTextBrowser(centralwidget);
    errors->setObjectName(QString::fromUtf8("errors"));
    errors->setGeometry(QRect(0, 340, 711, 101));
    label = new QLabel(centralwidget);
    label->setObjectName(QString::fromUtf8("label"));
    label->setGeometry(QRect(40, 0, 41, 18));
    label_3 = new QLabel(centralwidget);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    label_3->setGeometry(QRect(0, 320, 59, 18));
    label_2 = new QLabel(centralwidget);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    label_2->setGeometry(QRect(0, 30, 59, 18));
    pushButton = new QPushButton(centralwidget);
    pushButton->setObjectName(QString::fromUtf8("pushButton"));
    pushButton->setGeometry(QRect(310, 450, 92, 28));
    MainWindow->setCentralWidget(centralwidget);

    retranslateUi(MainWindow);

    QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
    MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("MainWindow", "Input:", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("MainWindow", "Errors:", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("MainWindow", "Output:", 0, QApplication::UnicodeUTF8));
    pushButton->setText(QApplication::translate("MainWindow", "Start", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(MainWindow);
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H

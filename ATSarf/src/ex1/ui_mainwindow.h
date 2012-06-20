/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Fri Jun 1 11:35:06 2012
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QStatusBar>
#include <QtGui/QTextBrowser>
#include <QtGui/QTextEdit>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QPushButton *pushButton;
    QTextEdit *input;
    QTextBrowser *output;
    QRadioButton *POSVerb;
    QRadioButton *morphology;
    QRadioButton *GlossSFR;
    QCheckBox *chk_runon;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(583, 493);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(240, 60, 98, 31));
        input = new QTextEdit(centralWidget);
        input->setObjectName(QString::fromUtf8("input"));
        input->setGeometry(QRect(50, 10, 501, 31));
        input->setLayoutDirection(Qt::LeftToRight);
        input->setLocale(QLocale(QLocale::Arabic, QLocale::Lebanon));
        output = new QTextBrowser(centralWidget);
        output->setObjectName(QString::fromUtf8("output"));
        output->setGeometry(QRect(50, 120, 501, 311));
        output->setMinimumSize(QSize(150, 0));
        output->setMaximumSize(QSize(1151, 16777215));
        QFont font;
        font.setFamily(QString::fromUtf8("Titr"));
        font.setBold(true);
        font.setWeight(75);
        output->setFont(font);
        output->setLayoutDirection(Qt::LeftToRight);
        output->setAutoFillBackground(false);
        output->setLocale(QLocale(QLocale::Arabic, QLocale::Lebanon));
        POSVerb = new QRadioButton(centralWidget);
        POSVerb->setObjectName(QString::fromUtf8("POSVerb"));
        POSVerb->setGeometry(QRect(50, 70, 111, 22));
        POSVerb->setChecked(false);
        morphology = new QRadioButton(centralWidget);
        morphology->setObjectName(QString::fromUtf8("morphology"));
        morphology->setGeometry(QRect(50, 50, 111, 22));
        morphology->setChecked(false);
        GlossSFR = new QRadioButton(centralWidget);
        GlossSFR->setObjectName(QString::fromUtf8("GlossSFR"));
        GlossSFR->setGeometry(QRect(50, 90, 111, 22));
        GlossSFR->setChecked(false);
        chk_runon = new QCheckBox(centralWidget);
        chk_runon->setObjectName(QString::fromUtf8("chk_runon"));
        chk_runon->setGeometry(QRect(390, 60, 161, 22));
        chk_runon->setChecked(false);
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 583, 25));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("MainWindow", "PushButton", 0, QApplication::UnicodeUTF8));
        POSVerb->setText(QApplication::translate("MainWindow", "POSVerb", 0, QApplication::UnicodeUTF8));
        morphology->setText(QApplication::translate("MainWindow", "Morphology", 0, QApplication::UnicodeUTF8));
        GlossSFR->setText(QApplication::translate("MainWindow", "GlossSFR", 0, QApplication::UnicodeUTF8));
        chk_runon->setText(QApplication::translate("MainWindow", "Enable Runon Words", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H


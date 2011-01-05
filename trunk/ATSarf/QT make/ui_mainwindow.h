/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Wed Jan 5 02:14:16 2011
**      by: Qt User Interface Compiler version 4.6.2
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
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QProgressBar>
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
    QLabel *hadith_chain;
    QCheckBox *checkBox;
    QTextBrowser *hadith_display;
    QLabel *label_6;
    QProgressBar *progressBar;
    QPushButton *exit;
    QPushButton *fill;
    QCheckBox *chk_testing;
    QPushButton *cmd_browse;
    QTextEdit *NMC;
    QTextEdit *NRC;
    QTextEdit *NARRATOR;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_7;
    QLabel *label_8;
    QTextEdit *EQ_delta;
    QLabel *label_9;
    QTextEdit *EQ_radius;
    QTextEdit *EQ_threshold;
    QLabel *label_10;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1000, 650);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        input = new QTextEdit(centralwidget);
        input->setObjectName(QString::fromUtf8("input"));
        input->setGeometry(QRect(110, 10, 401, 31));
        input->setLayoutDirection(Qt::LeftToRight);
        input->setLocale(QLocale(QLocale::Arabic, QLocale::Lebanon));
        output = new QTextBrowser(centralwidget);
        output->setObjectName(QString::fromUtf8("output"));
        output->setGeometry(QRect(40, 60, 501, 391));
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
        errors = new QTextBrowser(centralwidget);
        errors->setObjectName(QString::fromUtf8("errors"));
        errors->setGeometry(QRect(40, 470, 501, 151));
        errors->setLayoutDirection(Qt::LeftToRight);
        errors->setLocale(QLocale(QLocale::Arabic, QLocale::Lebanon));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(40, 10, 51, 18));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Bitstream Charter"));
        font1.setPointSize(12);
        font1.setBold(true);
        font1.setWeight(75);
        label->setFont(font1);
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(40, 450, 59, 18));
        label_3->setFont(font1);
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(40, 40, 59, 18));
        label_2->setFont(font1);
        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(520, 0, 92, 28));
        pushButton->setFont(font1);
        hadith_chain = new QLabel(centralwidget);
        hadith_chain->setObjectName(QString::fromUtf8("hadith_chain"));
        hadith_chain->setGeometry(QRect(550, 80, 441, 371));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(255);
        sizePolicy.setVerticalStretch(255);
        sizePolicy.setHeightForWidth(hadith_chain->sizePolicy().hasHeightForWidth());
        hadith_chain->setSizePolicy(sizePolicy);
        hadith_chain->setFrameShape(QFrame::NoFrame);
        hadith_chain->setPixmap(QPixmap(QString::fromUtf8("../sarf33.bmp")));
        hadith_chain->setScaledContents(true);
        hadith_chain->setAlignment(Qt::AlignCenter);
        checkBox = new QCheckBox(centralwidget);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));
        checkBox->setGeometry(QRect(420, 40, 91, 23));
        checkBox->setFont(font1);
        hadith_display = new QTextBrowser(centralwidget);
        hadith_display->setObjectName(QString::fromUtf8("hadith_display"));
        hadith_display->setGeometry(QRect(550, 470, 441, 151));
        label_6 = new QLabel(centralwidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(550, 80, 121, 18));
        label_6->setFont(font1);
        progressBar = new QProgressBar(centralwidget);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setGeometry(QRect(720, 10, 271, 23));
        progressBar->setValue(0);
        exit = new QPushButton(centralwidget);
        exit->setObjectName(QString::fromUtf8("exit"));
        exit->setGeometry(QRect(620, 10, 92, 28));
        fill = new QPushButton(centralwidget);
        fill->setObjectName(QString::fromUtf8("fill"));
        fill->setGeometry(QRect(520, 30, 91, 27));
        chk_testing = new QCheckBox(centralwidget);
        chk_testing->setObjectName(QString::fromUtf8("chk_testing"));
        chk_testing->setGeometry(QRect(320, 40, 91, 23));
        chk_testing->setFont(font1);
        cmd_browse = new QPushButton(centralwidget);
        cmd_browse->setObjectName(QString::fromUtf8("cmd_browse"));
        cmd_browse->setGeometry(QRect(620, 10, 93, 27));
        NMC = new QTextEdit(centralwidget);
        NMC->setObjectName(QString::fromUtf8("NMC"));
        NMC->setGeometry(QRect(960, 40, 31, 21));
        NMC->setInputMethodHints(Qt::ImhDigitsOnly);
        NMC->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        NMC->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        NRC = new QTextEdit(centralwidget);
        NRC->setObjectName(QString::fromUtf8("NRC"));
        NRC->setGeometry(QRect(830, 40, 31, 21));
        NRC->setInputMethodHints(Qt::ImhDigitsOnly);
        NRC->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        NRC->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        NARRATOR = new QTextEdit(centralwidget);
        NARRATOR->setObjectName(QString::fromUtf8("NARRATOR"));
        NARRATOR->setGeometry(QRect(710, 40, 31, 21));
        NARRATOR->setInputMethodHints(Qt::ImhDigitsOnly);
        NARRATOR->setFrameShape(QFrame::StyledPanel);
        NARRATOR->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        NARRATOR->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(630, 40, 71, 17));
        label_5 = new QLabel(centralwidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(750, 40, 71, 17));
        label_7 = new QLabel(centralwidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(870, 40, 71, 17));
        label_8 = new QLabel(centralwidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(630, 60, 71, 17));
        EQ_delta = new QTextEdit(centralwidget);
        EQ_delta->setObjectName(QString::fromUtf8("EQ_delta"));
        EQ_delta->setGeometry(QRect(830, 60, 31, 21));
        EQ_delta->setInputMethodHints(Qt::ImhDigitsOnly);
        EQ_delta->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        EQ_delta->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        label_9 = new QLabel(centralwidget);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(870, 60, 91, 17));
        EQ_radius = new QTextEdit(centralwidget);
        EQ_radius->setObjectName(QString::fromUtf8("EQ_radius"));
        EQ_radius->setGeometry(QRect(710, 60, 31, 21));
        EQ_radius->setInputMethodHints(Qt::ImhDigitsOnly);
        EQ_radius->setFrameShape(QFrame::StyledPanel);
        EQ_radius->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        EQ_radius->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        EQ_threshold = new QTextEdit(centralwidget);
        EQ_threshold->setObjectName(QString::fromUtf8("EQ_threshold"));
        EQ_threshold->setGeometry(QRect(960, 60, 31, 21));
        EQ_threshold->setInputMethodHints(Qt::ImhDigitsOnly);
        EQ_threshold->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        EQ_threshold->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        label_10 = new QLabel(centralwidget);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setGeometry(QRect(750, 60, 71, 17));
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "AUBSarf", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "Input:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("MainWindow", "Issues:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "Output:", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("MainWindow", "GO!", 0, QApplication::UnicodeUTF8));
        hadith_chain->setText(QString());
        checkBox->setText(QApplication::translate("MainWindow", "Hadith", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("MainWindow", "Hadith Chains:", 0, QApplication::UnicodeUTF8));
        exit->setText(QApplication::translate("MainWindow", "exit", 0, QApplication::UnicodeUTF8));
        fill->setText(QApplication::translate("MainWindow", "Fill", 0, QApplication::UnicodeUTF8));
        chk_testing->setText(QApplication::translate("MainWindow", "Testing", 0, QApplication::UnicodeUTF8));
        cmd_browse->setText(QApplication::translate("MainWindow", "Browse", 0, QApplication::UnicodeUTF8));
        NMC->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">3</p></body></html>", 0, QApplication::UnicodeUTF8));
        NRC->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">5</p></body></html>", 0, QApplication::UnicodeUTF8));
        NARRATOR->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">3</p></body></html>", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("MainWindow", "NARR min:", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("MainWindow", "NRC max:", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("MainWindow", "NMC max:", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("MainWindow", "EQ radius:", 0, QApplication::UnicodeUTF8));
        EQ_delta->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">0.4</p></body></html>", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("MainWindow", "EQ threshold:", 0, QApplication::UnicodeUTF8));
        EQ_radius->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">3</p></body></html>", 0, QApplication::UnicodeUTF8));
        EQ_threshold->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">1.0</p></body></html>", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("MainWindow", "EQ delta:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H

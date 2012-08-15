/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Wed Aug 15 22:15:27 2012
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
#include <QtGui/QFrame>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
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
    QTextBrowser *hadith_display;
    QLabel *label_6;
    QProgressBar *progressBar;
    QPushButton *exit;
    QPushButton *fill;
    QPushButton *cmd_browse;
    QTextEdit *NMC;
    QTextEdit *NRC;
    QTextEdit *NARRATOR;
    QLabel *label_narr;
    QLabel *label_nrc;
    QLabel *label_nmc;
    QLabel *label_eq_rad;
    QTextEdit *EQ_delta;
    QLabel *label_9;
    QTextEdit *EQ_radius;
    QTextEdit *EQ_threshold;
    QLabel *label_eq_delta;
    QCheckBox *chk_chainNum;
    QCheckBox *chk_breakCycles;
    QCheckBox *chk_runon;
    QFrame *line;
    QFrame *line_2;
    QFrame *line_3;
    QFrame *line_4;
    QFrame *line_5;
    QFrame *line_6;
    QFrame *line_7;
    QFrame *line_8;
    QRadioButton *chk_hadith;
    QRadioButton *chk_biography;
    QRadioButton *chk_morphology;
    QRadioButton *chk_bible;
    QRadioButton *chk_bible_anotation;
    QRadioButton *chk_AffixBreaker;
    QRadioButton *chk_verification;
    QRadioButton *chk_time_anotation;
    QRadioButton *chk_time;
    QRadioButton *chk_hadith_anotation;
    QFrame *line_9;
    QRadioButton *chk_testing;
    QRadioButton *chk_hadith_name_anotation;
    QRadioButton *chk_bible_agreement;
    QRadioButton *chk_hadith_agreement;
    QRadioButton *chk_narrator_annotation;
    QFrame *line_10;
    QFrame *line_11;
    QFrame *line_12;
    QRadioButton *chk_narrator_equality;
    QCheckBox *chk_detailed_Statistics;
    QCheckBox *chk_genealogyRefined;
    QFrame *line_13;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1000, 650);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        input = new QTextEdit(centralwidget);
        input->setObjectName(QString::fromUtf8("input"));
        input->setGeometry(QRect(100, 10, 401, 31));
        input->setLayoutDirection(Qt::LeftToRight);
        input->setLocale(QLocale(QLocale::Arabic, QLocale::Lebanon));
        output = new QTextBrowser(centralwidget);
        output->setObjectName(QString::fromUtf8("output"));
        output->setGeometry(QRect(30, 120, 501, 331));
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
        errors->setGeometry(QRect(30, 470, 501, 151));
        errors->setLayoutDirection(Qt::LeftToRight);
        errors->setLocale(QLocale(QLocale::Arabic, QLocale::Lebanon));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(30, 10, 51, 18));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Bitstream Charter"));
        font1.setPointSize(12);
        font1.setBold(true);
        font1.setWeight(75);
        label->setFont(font1);
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(30, 450, 59, 18));
        label_3->setFont(font1);
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(30, 100, 59, 18));
        label_2->setFont(font1);
        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(520, 10, 92, 28));
        pushButton->setFont(font1);
        hadith_display = new QTextBrowser(centralwidget);
        hadith_display->setObjectName(QString::fromUtf8("hadith_display"));
        hadith_display->setGeometry(QRect(550, 120, 441, 501));
        label_6 = new QLabel(centralwidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(550, 100, 61, 18));
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
        fill->setGeometry(QRect(520, 10, 91, 27));
        cmd_browse = new QPushButton(centralwidget);
        cmd_browse->setObjectName(QString::fromUtf8("cmd_browse"));
        cmd_browse->setGeometry(QRect(620, 10, 93, 27));
        NMC = new QTextEdit(centralwidget);
        NMC->setObjectName(QString::fromUtf8("NMC"));
        NMC->setGeometry(QRect(960, 40, 31, 21));
        NMC->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        NMC->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        NRC = new QTextEdit(centralwidget);
        NRC->setObjectName(QString::fromUtf8("NRC"));
        NRC->setGeometry(QRect(810, 40, 31, 21));
        NRC->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        NRC->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        NARRATOR = new QTextEdit(centralwidget);
        NARRATOR->setObjectName(QString::fromUtf8("NARRATOR"));
        NARRATOR->setGeometry(QRect(660, 40, 31, 21));
        NARRATOR->setFrameShape(QFrame::StyledPanel);
        NARRATOR->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        NARRATOR->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        label_narr = new QLabel(centralwidget);
        label_narr->setObjectName(QString::fromUtf8("label_narr"));
        label_narr->setGeometry(QRect(550, 40, 101, 17));
        label_narr->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_nrc = new QLabel(centralwidget);
        label_nrc->setObjectName(QString::fromUtf8("label_nrc"));
        label_nrc->setGeometry(QRect(700, 40, 101, 17));
        label_nrc->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_nmc = new QLabel(centralwidget);
        label_nmc->setObjectName(QString::fromUtf8("label_nmc"));
        label_nmc->setGeometry(QRect(850, 40, 101, 20));
        label_nmc->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_eq_rad = new QLabel(centralwidget);
        label_eq_rad->setObjectName(QString::fromUtf8("label_eq_rad"));
        label_eq_rad->setGeometry(QRect(550, 60, 101, 17));
        label_eq_rad->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        EQ_delta = new QTextEdit(centralwidget);
        EQ_delta->setObjectName(QString::fromUtf8("EQ_delta"));
        EQ_delta->setGeometry(QRect(810, 60, 31, 21));
        EQ_delta->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        EQ_delta->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        label_9 = new QLabel(centralwidget);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(850, 60, 101, 20));
        label_9->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        EQ_radius = new QTextEdit(centralwidget);
        EQ_radius->setObjectName(QString::fromUtf8("EQ_radius"));
        EQ_radius->setGeometry(QRect(660, 60, 31, 21));
        EQ_radius->setFrameShape(QFrame::StyledPanel);
        EQ_radius->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        EQ_radius->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        EQ_threshold = new QTextEdit(centralwidget);
        EQ_threshold->setObjectName(QString::fromUtf8("EQ_threshold"));
        EQ_threshold->setGeometry(QRect(960, 60, 31, 21));
        EQ_threshold->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        EQ_threshold->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        label_eq_delta = new QLabel(centralwidget);
        label_eq_delta->setObjectName(QString::fromUtf8("label_eq_delta"));
        label_eq_delta->setGeometry(QRect(700, 60, 101, 17));
        label_eq_delta->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        chk_chainNum = new QCheckBox(centralwidget);
        chk_chainNum->setObjectName(QString::fromUtf8("chk_chainNum"));
        chk_chainNum->setGeometry(QRect(810, 80, 181, 22));
        chk_breakCycles = new QCheckBox(centralwidget);
        chk_breakCycles->setObjectName(QString::fromUtf8("chk_breakCycles"));
        chk_breakCycles->setGeometry(QRect(700, 80, 111, 22));
        chk_breakCycles->setChecked(true);
        chk_runon = new QCheckBox(centralwidget);
        chk_runon->setObjectName(QString::fromUtf8("chk_runon"));
        chk_runon->setGeometry(QRect(540, 80, 161, 22));
        chk_runon->setChecked(false);
        line = new QFrame(centralwidget);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(130, 40, 16, 41));
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);
        line_2 = new QFrame(centralwidget);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setGeometry(QRect(300, 40, 16, 41));
        line_2->setFrameShape(QFrame::VLine);
        line_2->setFrameShadow(QFrame::Sunken);
        line_3 = new QFrame(centralwidget);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setGeometry(QRect(410, 40, 16, 61));
        line_3->setFrameShape(QFrame::VLine);
        line_3->setFrameShadow(QFrame::Sunken);
        line_4 = new QFrame(centralwidget);
        line_4->setObjectName(QString::fromUtf8("line_4"));
        line_4->setGeometry(QRect(310, 70, 111, 21));
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);
        line_5 = new QFrame(centralwidget);
        line_5->setObjectName(QString::fromUtf8("line_5"));
        line_5->setGeometry(QRect(280, 90, 171, 21));
        line_5->setFrameShape(QFrame::HLine);
        line_5->setFrameShadow(QFrame::Sunken);
        line_6 = new QFrame(centralwidget);
        line_6->setObjectName(QString::fromUtf8("line_6"));
        line_6->setGeometry(QRect(520, 40, 21, 81));
        line_6->setFrameShape(QFrame::VLine);
        line_6->setFrameShadow(QFrame::Sunken);
        line_7 = new QFrame(centralwidget);
        line_7->setObjectName(QString::fromUtf8("line_7"));
        line_7->setGeometry(QRect(20, 40, 20, 61));
        line_7->setFrameShape(QFrame::VLine);
        line_7->setFrameShadow(QFrame::Sunken);
        line_8 = new QFrame(centralwidget);
        line_8->setObjectName(QString::fromUtf8("line_8"));
        line_8->setGeometry(QRect(30, 30, 501, 21));
        line_8->setFrameShape(QFrame::HLine);
        line_8->setFrameShadow(QFrame::Sunken);
        chk_hadith = new QRadioButton(centralwidget);
        chk_hadith->setObjectName(QString::fromUtf8("chk_hadith"));
        chk_hadith->setGeometry(QRect(140, 40, 71, 22));
        chk_biography = new QRadioButton(centralwidget);
        chk_biography->setObjectName(QString::fromUtf8("chk_biography"));
        chk_biography->setGeometry(QRect(310, 80, 91, 22));
        chk_morphology = new QRadioButton(centralwidget);
        chk_morphology->setObjectName(QString::fromUtf8("chk_morphology"));
        chk_morphology->setGeometry(QRect(30, 40, 111, 22));
        chk_morphology->setChecked(true);
        chk_bible = new QRadioButton(centralwidget);
        chk_bible->setObjectName(QString::fromUtf8("chk_bible"));
        chk_bible->setGeometry(QRect(420, 40, 101, 22));
        chk_bible_anotation = new QRadioButton(centralwidget);
        chk_bible_anotation->setObjectName(QString::fromUtf8("chk_bible_anotation"));
        chk_bible_anotation->setGeometry(QRect(420, 60, 101, 22));
        chk_AffixBreaker = new QRadioButton(centralwidget);
        chk_AffixBreaker->setObjectName(QString::fromUtf8("chk_AffixBreaker"));
        chk_AffixBreaker->setEnabled(true);
        chk_AffixBreaker->setGeometry(QRect(780, 100, 111, 22));
        chk_AffixBreaker->setChecked(false);
        chk_verification = new QRadioButton(centralwidget);
        chk_verification->setObjectName(QString::fromUtf8("chk_verification"));
        chk_verification->setEnabled(true);
        chk_verification->setGeometry(QRect(890, 100, 101, 22));
        chk_time_anotation = new QRadioButton(centralwidget);
        chk_time_anotation->setObjectName(QString::fromUtf8("chk_time_anotation"));
        chk_time_anotation->setGeometry(QRect(310, 60, 91, 22));
        chk_time = new QRadioButton(centralwidget);
        chk_time->setObjectName(QString::fromUtf8("chk_time"));
        chk_time->setGeometry(QRect(310, 40, 91, 22));
        chk_hadith_anotation = new QRadioButton(centralwidget);
        chk_hadith_anotation->setObjectName(QString::fromUtf8("chk_hadith_anotation"));
        chk_hadith_anotation->setGeometry(QRect(210, 40, 101, 22));
        line_9 = new QFrame(centralwidget);
        line_9->setObjectName(QString::fromUtf8("line_9"));
        line_9->setGeometry(QRect(30, 70, 111, 21));
        line_9->setFrameShape(QFrame::HLine);
        line_9->setFrameShadow(QFrame::Sunken);
        chk_testing = new QRadioButton(centralwidget);
        chk_testing->setObjectName(QString::fromUtf8("chk_testing"));
        chk_testing->setGeometry(QRect(30, 60, 71, 22));
        chk_hadith_name_anotation = new QRadioButton(centralwidget);
        chk_hadith_name_anotation->setObjectName(QString::fromUtf8("chk_hadith_name_anotation"));
        chk_hadith_name_anotation->setGeometry(QRect(140, 80, 151, 22));
        chk_bible_agreement = new QRadioButton(centralwidget);
        chk_bible_agreement->setObjectName(QString::fromUtf8("chk_bible_agreement"));
        chk_bible_agreement->setGeometry(QRect(420, 80, 101, 22));
        chk_hadith_agreement = new QRadioButton(centralwidget);
        chk_hadith_agreement->setObjectName(QString::fromUtf8("chk_hadith_agreement"));
        chk_hadith_agreement->setGeometry(QRect(30, 80, 101, 22));
        chk_narrator_annotation = new QRadioButton(centralwidget);
        chk_narrator_annotation->setObjectName(QString::fromUtf8("chk_narrator_annotation"));
        chk_narrator_annotation->setGeometry(QRect(140, 60, 161, 22));
        line_10 = new QFrame(centralwidget);
        line_10->setObjectName(QString::fromUtf8("line_10"));
        line_10->setGeometry(QRect(270, 100, 20, 21));
        line_10->setFrameShape(QFrame::VLine);
        line_10->setFrameShadow(QFrame::Sunken);
        line_11 = new QFrame(centralwidget);
        line_11->setObjectName(QString::fromUtf8("line_11"));
        line_11->setGeometry(QRect(30, 90, 111, 21));
        line_11->setFrameShape(QFrame::HLine);
        line_11->setFrameShadow(QFrame::Sunken);
        line_12 = new QFrame(centralwidget);
        line_12->setObjectName(QString::fromUtf8("line_12"));
        line_12->setGeometry(QRect(130, 100, 16, 21));
        line_12->setFrameShape(QFrame::VLine);
        line_12->setFrameShadow(QFrame::Sunken);
        chk_narrator_equality = new QRadioButton(centralwidget);
        chk_narrator_equality->setObjectName(QString::fromUtf8("chk_narrator_equality"));
        chk_narrator_equality->setGeometry(QRect(140, 100, 141, 22));
        chk_detailed_Statistics = new QCheckBox(centralwidget);
        chk_detailed_Statistics->setObjectName(QString::fromUtf8("chk_detailed_Statistics"));
        chk_detailed_Statistics->setGeometry(QRect(290, 100, 141, 22));
        chk_detailed_Statistics->setChecked(false);
        chk_genealogyRefined = new QCheckBox(centralwidget);
        chk_genealogyRefined->setObjectName(QString::fromUtf8("chk_genealogyRefined"));
        chk_genealogyRefined->setGeometry(QRect(450, 100, 81, 22));
        chk_genealogyRefined->setChecked(true);
        line_13 = new QFrame(centralwidget);
        line_13->setObjectName(QString::fromUtf8("line_13"));
        line_13->setGeometry(QRect(440, 100, 16, 21));
        line_13->setFrameShape(QFrame::VLine);
        line_13->setFrameShadow(QFrame::Sunken);
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Sarf", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "Input:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("MainWindow", "Issues:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "Output:", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("MainWindow", "GO!", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("MainWindow", "Chains:", 0, QApplication::UnicodeUTF8));
        exit->setText(QApplication::translate("MainWindow", "exit", 0, QApplication::UnicodeUTF8));
        fill->setText(QApplication::translate("MainWindow", "Fill", 0, QApplication::UnicodeUTF8));
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
        label_narr->setText(QApplication::translate("MainWindow", "NARR min:", 0, QApplication::UnicodeUTF8));
        label_nrc->setText(QApplication::translate("MainWindow", "NRC max:", 0, QApplication::UnicodeUTF8));
        label_nmc->setText(QApplication::translate("MainWindow", "NMC max:", 0, QApplication::UnicodeUTF8));
        label_eq_rad->setText(QApplication::translate("MainWindow", "EQ radius:", 0, QApplication::UnicodeUTF8));
        EQ_delta->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">0.3</p></body></html>", 0, QApplication::UnicodeUTF8));
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
        label_eq_delta->setText(QApplication::translate("MainWindow", "EQ delta:", 0, QApplication::UnicodeUTF8));
        chk_chainNum->setText(QApplication::translate("MainWindow", "Display Chain Numbers", 0, QApplication::UnicodeUTF8));
        chk_breakCycles->setText(QApplication::translate("MainWindow", "Break Cycles", 0, QApplication::UnicodeUTF8));
        chk_runon->setText(QApplication::translate("MainWindow", "Enable Runon Words", 0, QApplication::UnicodeUTF8));
        chk_hadith->setText(QApplication::translate("MainWindow", "Hadith", 0, QApplication::UnicodeUTF8));
        chk_biography->setText(QApplication::translate("MainWindow", "Biography", 0, QApplication::UnicodeUTF8));
        chk_morphology->setText(QApplication::translate("MainWindow", "Morphology", 0, QApplication::UnicodeUTF8));
        chk_bible->setText(QApplication::translate("MainWindow", "Genealogy", 0, QApplication::UnicodeUTF8));
        chk_bible_anotation->setText(QApplication::translate("MainWindow", "Anotation", 0, QApplication::UnicodeUTF8));
        chk_AffixBreaker->setText(QApplication::translate("MainWindow", "Affix Breaker", 0, QApplication::UnicodeUTF8));
        chk_verification->setText(QApplication::translate("MainWindow", "Verification", 0, QApplication::UnicodeUTF8));
        chk_time_anotation->setText(QApplication::translate("MainWindow", "Anotation", 0, QApplication::UnicodeUTF8));
        chk_time->setText(QApplication::translate("MainWindow", "Time", 0, QApplication::UnicodeUTF8));
        chk_hadith_anotation->setText(QApplication::translate("MainWindow", "Annotation", 0, QApplication::UnicodeUTF8));
        chk_testing->setText(QApplication::translate("MainWindow", "Testing", 0, QApplication::UnicodeUTF8));
        chk_hadith_name_anotation->setText(QApplication::translate("MainWindow", "Names Annotation", 0, QApplication::UnicodeUTF8));
        chk_bible_agreement->setText(QApplication::translate("MainWindow", "Agreement", 0, QApplication::UnicodeUTF8));
        chk_hadith_agreement->setText(QApplication::translate("MainWindow", "Agreement", 0, QApplication::UnicodeUTF8));
        chk_narrator_annotation->setText(QApplication::translate("MainWindow", "Narrator Annotation", 0, QApplication::UnicodeUTF8));
        chk_narrator_equality->setText(QApplication::translate("MainWindow", "Narrator Equality", 0, QApplication::UnicodeUTF8));
        chk_detailed_Statistics->setText(QApplication::translate("MainWindow", "Verbose Statistics", 0, QApplication::UnicodeUTF8));
        chk_genealogyRefined->setText(QApplication::translate("MainWindow", "Refined", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H

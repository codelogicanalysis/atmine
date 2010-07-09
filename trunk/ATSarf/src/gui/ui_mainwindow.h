/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Fri Jul 9 14:45:45 2010
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
    QLabel *label_5;
    QCheckBox *checkBox;
    QTextBrowser *hadith_display;
    QLabel *label_6;
    QProgressBar *progressBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1000, 650);
        QPalette palette;
        QBrush brush(QColor(0, 0, 0, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
        QBrush brush1(QColor(85, 170, 0, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Button, brush1);
        QBrush brush2(QColor(128, 255, 0, 255));
        brush2.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Light, brush2);
        QBrush brush3(QColor(106, 212, 0, 255));
        brush3.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Midlight, brush3);
        QBrush brush4(QColor(42, 85, 0, 255));
        brush4.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Dark, brush4);
        QBrush brush5(QColor(56, 113, 0, 255));
        brush5.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Mid, brush5);
        palette.setBrush(QPalette::Active, QPalette::Text, brush);
        QBrush brush6(QColor(255, 255, 255, 255));
        brush6.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::BrightText, brush6);
        palette.setBrush(QPalette::Active, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Active, QPalette::Base, brush6);
        palette.setBrush(QPalette::Active, QPalette::Window, brush1);
        palette.setBrush(QPalette::Active, QPalette::Shadow, brush);
        QBrush brush7(QColor(170, 212, 127, 255));
        brush7.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::AlternateBase, brush7);
        QBrush brush8(QColor(255, 255, 220, 255));
        brush8.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::ToolTipBase, brush8);
        palette.setBrush(QPalette::Active, QPalette::ToolTipText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Button, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Light, brush2);
        palette.setBrush(QPalette::Inactive, QPalette::Midlight, brush3);
        palette.setBrush(QPalette::Inactive, QPalette::Dark, brush4);
        palette.setBrush(QPalette::Inactive, QPalette::Mid, brush5);
        palette.setBrush(QPalette::Inactive, QPalette::Text, brush);
        palette.setBrush(QPalette::Inactive, QPalette::BrightText, brush6);
        palette.setBrush(QPalette::Inactive, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush6);
        palette.setBrush(QPalette::Inactive, QPalette::Window, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Shadow, brush);
        palette.setBrush(QPalette::Inactive, QPalette::AlternateBase, brush7);
        palette.setBrush(QPalette::Inactive, QPalette::ToolTipBase, brush8);
        palette.setBrush(QPalette::Inactive, QPalette::ToolTipText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush4);
        palette.setBrush(QPalette::Disabled, QPalette::Button, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Light, brush2);
        palette.setBrush(QPalette::Disabled, QPalette::Midlight, brush3);
        palette.setBrush(QPalette::Disabled, QPalette::Dark, brush4);
        palette.setBrush(QPalette::Disabled, QPalette::Mid, brush5);
        palette.setBrush(QPalette::Disabled, QPalette::Text, brush4);
        palette.setBrush(QPalette::Disabled, QPalette::BrightText, brush6);
        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush4);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Window, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Shadow, brush);
        palette.setBrush(QPalette::Disabled, QPalette::AlternateBase, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::ToolTipBase, brush8);
        palette.setBrush(QPalette::Disabled, QPalette::ToolTipText, brush);
        MainWindow->setPalette(palette);
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
        QPalette palette1;
        palette1.setBrush(QPalette::Active, QPalette::WindowText, brush);
        QBrush brush9(QColor(0, 170, 0, 255));
        brush9.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::Button, brush9);
        QBrush brush10(QColor(0, 255, 0, 255));
        brush10.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::Light, brush10);
        QBrush brush11(QColor(0, 212, 0, 255));
        brush11.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::Midlight, brush11);
        QBrush brush12(QColor(0, 85, 0, 255));
        brush12.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::Dark, brush12);
        QBrush brush13(QColor(0, 113, 0, 255));
        brush13.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::Mid, brush13);
        palette1.setBrush(QPalette::Active, QPalette::Text, brush);
        palette1.setBrush(QPalette::Active, QPalette::BrightText, brush6);
        palette1.setBrush(QPalette::Active, QPalette::ButtonText, brush);
        palette1.setBrush(QPalette::Active, QPalette::Base, brush6);
        palette1.setBrush(QPalette::Active, QPalette::Window, brush9);
        palette1.setBrush(QPalette::Active, QPalette::Shadow, brush);
        QBrush brush14(QColor(127, 212, 127, 255));
        brush14.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::AlternateBase, brush14);
        palette1.setBrush(QPalette::Active, QPalette::ToolTipBase, brush8);
        palette1.setBrush(QPalette::Active, QPalette::ToolTipText, brush);
        palette1.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        palette1.setBrush(QPalette::Inactive, QPalette::Button, brush9);
        palette1.setBrush(QPalette::Inactive, QPalette::Light, brush10);
        palette1.setBrush(QPalette::Inactive, QPalette::Midlight, brush11);
        palette1.setBrush(QPalette::Inactive, QPalette::Dark, brush12);
        palette1.setBrush(QPalette::Inactive, QPalette::Mid, brush13);
        palette1.setBrush(QPalette::Inactive, QPalette::Text, brush);
        palette1.setBrush(QPalette::Inactive, QPalette::BrightText, brush6);
        palette1.setBrush(QPalette::Inactive, QPalette::ButtonText, brush);
        palette1.setBrush(QPalette::Inactive, QPalette::Base, brush6);
        palette1.setBrush(QPalette::Inactive, QPalette::Window, brush9);
        palette1.setBrush(QPalette::Inactive, QPalette::Shadow, brush);
        palette1.setBrush(QPalette::Inactive, QPalette::AlternateBase, brush14);
        palette1.setBrush(QPalette::Inactive, QPalette::ToolTipBase, brush8);
        palette1.setBrush(QPalette::Inactive, QPalette::ToolTipText, brush);
        palette1.setBrush(QPalette::Disabled, QPalette::WindowText, brush12);
        palette1.setBrush(QPalette::Disabled, QPalette::Button, brush9);
        palette1.setBrush(QPalette::Disabled, QPalette::Light, brush10);
        palette1.setBrush(QPalette::Disabled, QPalette::Midlight, brush11);
        palette1.setBrush(QPalette::Disabled, QPalette::Dark, brush12);
        palette1.setBrush(QPalette::Disabled, QPalette::Mid, brush13);
        palette1.setBrush(QPalette::Disabled, QPalette::Text, brush12);
        palette1.setBrush(QPalette::Disabled, QPalette::BrightText, brush6);
        palette1.setBrush(QPalette::Disabled, QPalette::ButtonText, brush12);
        palette1.setBrush(QPalette::Disabled, QPalette::Base, brush9);
        palette1.setBrush(QPalette::Disabled, QPalette::Window, brush9);
        palette1.setBrush(QPalette::Disabled, QPalette::Shadow, brush);
        palette1.setBrush(QPalette::Disabled, QPalette::AlternateBase, brush9);
        palette1.setBrush(QPalette::Disabled, QPalette::ToolTipBase, brush8);
        palette1.setBrush(QPalette::Disabled, QPalette::ToolTipText, brush);
        errors->setPalette(palette1);
        errors->setLayoutDirection(Qt::LeftToRight);
        errors->setLocale(QLocale(QLocale::Arabic, QLocale::Lebanon));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(40, 10, 51, 18));
        QPalette palette2;
        QBrush brush15(QColor(255, 255, 0, 255));
        brush15.setStyle(Qt::SolidPattern);
        palette2.setBrush(QPalette::Active, QPalette::WindowText, brush15);
        palette2.setBrush(QPalette::Inactive, QPalette::WindowText, brush15);
        palette2.setBrush(QPalette::Disabled, QPalette::WindowText, brush4);
        label->setPalette(palette2);
        QFont font1;
        font1.setFamily(QString::fromUtf8("Bitstream Charter"));
        font1.setPointSize(12);
        font1.setBold(true);
        font1.setWeight(75);
        label->setFont(font1);
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(40, 450, 59, 18));
        QPalette palette3;
        palette3.setBrush(QPalette::Active, QPalette::WindowText, brush15);
        palette3.setBrush(QPalette::Inactive, QPalette::WindowText, brush15);
        palette3.setBrush(QPalette::Disabled, QPalette::WindowText, brush4);
        label_3->setPalette(palette3);
        label_3->setFont(font1);
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(40, 40, 59, 18));
        QPalette palette4;
        palette4.setBrush(QPalette::Active, QPalette::WindowText, brush15);
        palette4.setBrush(QPalette::Active, QPalette::Text, brush);
        palette4.setBrush(QPalette::Inactive, QPalette::WindowText, brush15);
        palette4.setBrush(QPalette::Inactive, QPalette::Text, brush);
        palette4.setBrush(QPalette::Disabled, QPalette::WindowText, brush4);
        palette4.setBrush(QPalette::Disabled, QPalette::Text, brush4);
        label_2->setPalette(palette4);
        label_2->setFont(font1);
        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(520, 10, 92, 28));
        QPalette palette5;
        palette5.setBrush(QPalette::Active, QPalette::WindowText, brush);
        palette5.setBrush(QPalette::Active, QPalette::Button, brush15);
        QBrush brush16(QColor(255, 255, 127, 255));
        brush16.setStyle(Qt::SolidPattern);
        palette5.setBrush(QPalette::Active, QPalette::Light, brush16);
        QBrush brush17(QColor(255, 255, 63, 255));
        brush17.setStyle(Qt::SolidPattern);
        palette5.setBrush(QPalette::Active, QPalette::Midlight, brush17);
        QBrush brush18(QColor(127, 127, 0, 255));
        brush18.setStyle(Qt::SolidPattern);
        palette5.setBrush(QPalette::Active, QPalette::Dark, brush18);
        QBrush brush19(QColor(170, 170, 0, 255));
        brush19.setStyle(Qt::SolidPattern);
        palette5.setBrush(QPalette::Active, QPalette::Mid, brush19);
        palette5.setBrush(QPalette::Active, QPalette::Text, brush);
        palette5.setBrush(QPalette::Active, QPalette::BrightText, brush6);
        palette5.setBrush(QPalette::Active, QPalette::ButtonText, brush);
        palette5.setBrush(QPalette::Active, QPalette::Base, brush6);
        palette5.setBrush(QPalette::Active, QPalette::Window, brush15);
        palette5.setBrush(QPalette::Active, QPalette::Shadow, brush);
        palette5.setBrush(QPalette::Active, QPalette::AlternateBase, brush16);
        palette5.setBrush(QPalette::Active, QPalette::ToolTipBase, brush8);
        palette5.setBrush(QPalette::Active, QPalette::ToolTipText, brush);
        palette5.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        palette5.setBrush(QPalette::Inactive, QPalette::Button, brush15);
        palette5.setBrush(QPalette::Inactive, QPalette::Light, brush16);
        palette5.setBrush(QPalette::Inactive, QPalette::Midlight, brush17);
        palette5.setBrush(QPalette::Inactive, QPalette::Dark, brush18);
        palette5.setBrush(QPalette::Inactive, QPalette::Mid, brush19);
        palette5.setBrush(QPalette::Inactive, QPalette::Text, brush);
        palette5.setBrush(QPalette::Inactive, QPalette::BrightText, brush6);
        palette5.setBrush(QPalette::Inactive, QPalette::ButtonText, brush);
        palette5.setBrush(QPalette::Inactive, QPalette::Base, brush6);
        palette5.setBrush(QPalette::Inactive, QPalette::Window, brush15);
        palette5.setBrush(QPalette::Inactive, QPalette::Shadow, brush);
        palette5.setBrush(QPalette::Inactive, QPalette::AlternateBase, brush16);
        palette5.setBrush(QPalette::Inactive, QPalette::ToolTipBase, brush8);
        palette5.setBrush(QPalette::Inactive, QPalette::ToolTipText, brush);
        palette5.setBrush(QPalette::Disabled, QPalette::WindowText, brush18);
        palette5.setBrush(QPalette::Disabled, QPalette::Button, brush15);
        palette5.setBrush(QPalette::Disabled, QPalette::Light, brush16);
        palette5.setBrush(QPalette::Disabled, QPalette::Midlight, brush17);
        palette5.setBrush(QPalette::Disabled, QPalette::Dark, brush18);
        palette5.setBrush(QPalette::Disabled, QPalette::Mid, brush19);
        palette5.setBrush(QPalette::Disabled, QPalette::Text, brush18);
        palette5.setBrush(QPalette::Disabled, QPalette::BrightText, brush6);
        palette5.setBrush(QPalette::Disabled, QPalette::ButtonText, brush18);
        palette5.setBrush(QPalette::Disabled, QPalette::Base, brush15);
        palette5.setBrush(QPalette::Disabled, QPalette::Window, brush15);
        palette5.setBrush(QPalette::Disabled, QPalette::Shadow, brush);
        palette5.setBrush(QPalette::Disabled, QPalette::AlternateBase, brush15);
        palette5.setBrush(QPalette::Disabled, QPalette::ToolTipBase, brush8);
        palette5.setBrush(QPalette::Disabled, QPalette::ToolTipText, brush);
        pushButton->setPalette(palette5);
        pushButton->setFont(font1);
        label_5 = new QLabel(centralwidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(570, 60, 421, 191));
        label_5->setPixmap(QPixmap(QString::fromUtf8("../sarf33.bmp")));
        label_5->setScaledContents(true);
        label_5->setAlignment(Qt::AlignBottom|Qt::AlignHCenter);
        checkBox = new QCheckBox(centralwidget);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));
        checkBox->setGeometry(QRect(620, 10, 91, 23));
        QPalette palette6;
        palette6.setBrush(QPalette::Active, QPalette::WindowText, brush15);
        palette6.setBrush(QPalette::Active, QPalette::Dark, brush15);
        palette6.setBrush(QPalette::Active, QPalette::Text, brush);
        palette6.setBrush(QPalette::Active, QPalette::ButtonText, brush15);
        palette6.setBrush(QPalette::Active, QPalette::Shadow, brush15);
        palette6.setBrush(QPalette::Active, QPalette::HighlightedText, brush);
        palette6.setBrush(QPalette::Active, QPalette::ToolTipText, brush15);
        palette6.setBrush(QPalette::Inactive, QPalette::WindowText, brush15);
        palette6.setBrush(QPalette::Inactive, QPalette::Dark, brush15);
        palette6.setBrush(QPalette::Inactive, QPalette::Text, brush);
        palette6.setBrush(QPalette::Inactive, QPalette::ButtonText, brush15);
        palette6.setBrush(QPalette::Inactive, QPalette::Shadow, brush15);
        palette6.setBrush(QPalette::Inactive, QPalette::HighlightedText, brush);
        palette6.setBrush(QPalette::Inactive, QPalette::ToolTipText, brush15);
        palette6.setBrush(QPalette::Disabled, QPalette::WindowText, brush15);
        palette6.setBrush(QPalette::Disabled, QPalette::Dark, brush15);
        palette6.setBrush(QPalette::Disabled, QPalette::Text, brush15);
        palette6.setBrush(QPalette::Disabled, QPalette::ButtonText, brush15);
        palette6.setBrush(QPalette::Disabled, QPalette::Shadow, brush15);
        palette6.setBrush(QPalette::Disabled, QPalette::HighlightedText, brush);
        palette6.setBrush(QPalette::Disabled, QPalette::ToolTipText, brush15);
        checkBox->setPalette(palette6);
        checkBox->setFont(font1);
        hadith_display = new QTextBrowser(centralwidget);
        hadith_display->setObjectName(QString::fromUtf8("hadith_display"));
        hadith_display->setGeometry(QRect(570, 290, 421, 331));
        label_6 = new QLabel(centralwidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(570, 260, 121, 18));
        QPalette palette7;
        palette7.setBrush(QPalette::Active, QPalette::WindowText, brush15);
        palette7.setBrush(QPalette::Active, QPalette::Text, brush15);
        palette7.setBrush(QPalette::Inactive, QPalette::WindowText, brush15);
        palette7.setBrush(QPalette::Inactive, QPalette::Text, brush15);
        palette7.setBrush(QPalette::Disabled, QPalette::WindowText, brush4);
        palette7.setBrush(QPalette::Disabled, QPalette::Text, brush4);
        label_6->setPalette(palette7);
        label_6->setFont(font1);
        progressBar = new QProgressBar(centralwidget);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setGeometry(QRect(720, 10, 271, 23));
        QPalette palette8;
        palette8.setBrush(QPalette::Active, QPalette::WindowText, brush);
        palette8.setBrush(QPalette::Active, QPalette::Button, brush16);
        palette8.setBrush(QPalette::Active, QPalette::Light, brush6);
        QBrush brush20(QColor(255, 255, 191, 255));
        brush20.setStyle(Qt::SolidPattern);
        palette8.setBrush(QPalette::Active, QPalette::Midlight, brush20);
        QBrush brush21(QColor(127, 127, 63, 255));
        brush21.setStyle(Qt::SolidPattern);
        palette8.setBrush(QPalette::Active, QPalette::Dark, brush21);
        QBrush brush22(QColor(170, 170, 84, 255));
        brush22.setStyle(Qt::SolidPattern);
        palette8.setBrush(QPalette::Active, QPalette::Mid, brush22);
        palette8.setBrush(QPalette::Active, QPalette::Text, brush);
        palette8.setBrush(QPalette::Active, QPalette::BrightText, brush6);
        palette8.setBrush(QPalette::Active, QPalette::ButtonText, brush);
        palette8.setBrush(QPalette::Active, QPalette::Base, brush6);
        palette8.setBrush(QPalette::Active, QPalette::Window, brush16);
        palette8.setBrush(QPalette::Active, QPalette::Shadow, brush);
        palette8.setBrush(QPalette::Active, QPalette::AlternateBase, brush20);
        palette8.setBrush(QPalette::Active, QPalette::ToolTipBase, brush8);
        palette8.setBrush(QPalette::Active, QPalette::ToolTipText, brush);
        palette8.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        palette8.setBrush(QPalette::Inactive, QPalette::Button, brush16);
        palette8.setBrush(QPalette::Inactive, QPalette::Light, brush6);
        palette8.setBrush(QPalette::Inactive, QPalette::Midlight, brush20);
        palette8.setBrush(QPalette::Inactive, QPalette::Dark, brush21);
        palette8.setBrush(QPalette::Inactive, QPalette::Mid, brush22);
        palette8.setBrush(QPalette::Inactive, QPalette::Text, brush);
        palette8.setBrush(QPalette::Inactive, QPalette::BrightText, brush6);
        palette8.setBrush(QPalette::Inactive, QPalette::ButtonText, brush);
        palette8.setBrush(QPalette::Inactive, QPalette::Base, brush6);
        palette8.setBrush(QPalette::Inactive, QPalette::Window, brush16);
        palette8.setBrush(QPalette::Inactive, QPalette::Shadow, brush);
        palette8.setBrush(QPalette::Inactive, QPalette::AlternateBase, brush20);
        palette8.setBrush(QPalette::Inactive, QPalette::ToolTipBase, brush8);
        palette8.setBrush(QPalette::Inactive, QPalette::ToolTipText, brush);
        palette8.setBrush(QPalette::Disabled, QPalette::WindowText, brush21);
        palette8.setBrush(QPalette::Disabled, QPalette::Button, brush16);
        palette8.setBrush(QPalette::Disabled, QPalette::Light, brush6);
        palette8.setBrush(QPalette::Disabled, QPalette::Midlight, brush20);
        palette8.setBrush(QPalette::Disabled, QPalette::Dark, brush21);
        palette8.setBrush(QPalette::Disabled, QPalette::Mid, brush22);
        palette8.setBrush(QPalette::Disabled, QPalette::Text, brush21);
        palette8.setBrush(QPalette::Disabled, QPalette::BrightText, brush6);
        palette8.setBrush(QPalette::Disabled, QPalette::ButtonText, brush21);
        palette8.setBrush(QPalette::Disabled, QPalette::Base, brush16);
        palette8.setBrush(QPalette::Disabled, QPalette::Window, brush16);
        palette8.setBrush(QPalette::Disabled, QPalette::Shadow, brush);
        palette8.setBrush(QPalette::Disabled, QPalette::AlternateBase, brush16);
        palette8.setBrush(QPalette::Disabled, QPalette::ToolTipBase, brush8);
        palette8.setBrush(QPalette::Disabled, QPalette::ToolTipText, brush);
        progressBar->setPalette(palette8);
        progressBar->setValue(100);
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
        label_5->setText(QString());
        checkBox->setText(QApplication::translate("MainWindow", "Hadith", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("MainWindow", "Hadith Chains:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H

#ifndef CUSTOMSTTVIEW_H
#define CUSTOMSTTVIEW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QListWidget>
#include "colorlisteditor.h"

class QGroupBox;

class CustomSTTView : public QMainWindow
{
    Q_OBJECT
public:
    explicit CustomSTTView(QWidget *parent = 0);

signals:

private slots:
    void btnPOS_clicked();
    void btnGloss_clicked();
    void btnStem_clicked();
    void btnPrefix_clicked();
    void btnSuffix_clicked();
    void btnTagTypes_clicked();
    void btnSelectAll_clicked();
    void btnUnselectAll_clicked();
    void btnAdd_clicked();
    void btnSave_clicked();
    void btnLoad_clicked();
    void btnSelect_clicked();
    void btnUnselect_clicked();

private:
    QPushButton *btnPOS;
    QPushButton *btnGloss;
    QPushButton *btnStem;
    QPushButton *btnPrefix;
    QPushButton *btnSuffix;
    QPushButton *btnTagTypes;
    QPushButton *btnSelectAll;
    QPushButton *btnUnselectAll;
    QPushButton *btnAdd;
    QPushButton *btnSave;
    QPushButton *btnLoad;
    QPushButton *btnSelect;
    QPushButton *btnUnselect;

    QLabel *lblPattern;
    QLabel *lblTagName;
    QLabel *lblDescription;
    QLabel *lblFGColor;
    QLabel *lblBGColor;
    QLabel *lblFont;
    QLabel *lblItalic;
    QLabel *lblBold;
    QLabel *lblUnderline;

    QLineEdit *editPattern;
    QLineEdit *editTagName;
    QLineEdit *editDescription;

    ColorListEditor * colorfgcolor;
    ColorListEditor * colorbgcolor;

    QComboBox * cbfont;

    QCheckBox * cbunderline;
    QCheckBox * cbBold;
    QCheckBox * cbItalic;

    QListWidget *listPossibleTags;
    QListWidget *listSelectedTags;
};

#endif // CUSTOMSTTVIEW_H

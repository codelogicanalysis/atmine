#ifndef DIFFVIEW_H
#define DIFFVIEW_H

#include <QMainWindow>
#include <QTextBrowser>
#include <QRadioButton>
#include <QGroupBox>
#include <QLabel>

class DiffView : public QMainWindow
{
    Q_OBJECT
public:
    explicit DiffView(QWidget *parent = 0);

signals:

private slots:
    void rbTagTypes_clicked();
    void rbTags_clicked();
    void rbExact_clicked();
    void rbIntersect_clicked();
    void rbAContainB_clicked();
    void rbBContainA_clicked();

private:
    QTextBrowser *txtCommon;
    QTextBrowser *txtForwardDiff;
    QTextBrowser *txtReverseDiff;
    QTextBrowser *txtStats;

    QLabel *lblCommon;
    QLabel *lblForwardDiff;
    QLabel *lblReverseDiff;

    QRadioButton *rbTTDiff;
    QRadioButton *rbTDiff;
    QRadioButton *rbExact;
    QRadioButton *rbIntersect;
    QRadioButton *rbAContainB;
    QRadioButton *rbBContainA;

    //QRadioButton

};

#endif // DIFF_H

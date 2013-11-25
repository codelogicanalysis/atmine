#ifndef DIFFVIEW_H
#define DIFFVIEW_H

#include <QMainWindow>
#include <QTextBrowser>
#include <QRadioButton>
#include <QGroupBox>
#include <QLabel>
#include <QSignalMapper>
#include <QMenu>
#include <QAction>

#include "tag.h"
#include "tagtype.h"

class DiffView : public QMainWindow
{
    Q_OBJECT
public:
    explicit DiffView(QWidget *parent = 0);
    void startTaggingText(QString &);
    void tagWord(int, int, QColor, QColor ,int, bool, bool, bool, DestText);
    void addTags(QVector<const Tag*> &, QVector<const Tag*> &, QVector<const Tag*> &);
    void finishTaggingText();

private slots:
    void createActions();
    void rbTagTypes_clicked();
    void rbTags_clicked();
    void rbExact_clicked();
    void rbIntersect_clicked();
    void rbAContainB_clicked();
    void rbBContainA_clicked();
    void showContextMenuCommon(const QPoint &pt);
    void showContextMenuForward(const QPoint &pt);
    void showContextMenuReverse(const QPoint &pt);
    void untagCommon(QString tagValue);
    void tagCommon(QString tagValue);
    void untagForward(QString tagValue);
    void tagForward(QString tagValue);
    void untagReverse(QString tagValue);
    void tagReverse(QString tagValue);
    int insertTag(QString type, int pos, int length, int wordIndex, Source source, Dest dest);
    void closeEvent(QCloseEvent *event);

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

    QGroupBox *gb1;
    QGroupBox *gb2;

    QStringList commonTT;
    QStringList forwardTT;
    QStringList reverseTT;

    QAction *untagCommonAct;
    QAction *untagForwardAct;
    QAction *untagReverseAct;
    QAction *addtagAct;
    QAction *adduntagAct;

    QTextCursor myTC;
    QSignalMapper * signalMapper;
    QSignalMapper * signalMapperU;

    bool dirty;

    QVector<Tag> *tVector;
    QVector<Tag> *cttVector;

    QVector<const Tag*> commonVector;
    QVector<const Tag*> forwardVector;
    QVector<const Tag*> reverseVector;
};

#endif // DIFF_H

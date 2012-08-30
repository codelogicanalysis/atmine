#ifndef AMTMAINWINDOW_H
#define AMTMAINWINDOW_H

#include <QMainWindow>
#include<QFileDialog>
#include<QTextBrowser>
#include<QTreeWidget>
#include<QTextEdit>

#include <QMenu>
#include <QMenuBar>
#include <QSignalMapper>
#include "ATMProgressIFC.h"
#include "commonS.h"

class AMTMainWindow : public QMainWindow,public ATMProgressIFC
{
    Q_OBJECT

    protected:
         //void contextMenuEvent(QContextMenuEvent *event);

     public slots:
         void save();
         void showContextMenu(const QPoint &pt);
         void tagWord(int start, int length, QColor fcolor, QColor bcolor,int font, bool underline, bool italic, bool bold);
         void tag(QString tagValue);
         void itemSelectionChanged(QTreeWidgetItem*,int);

     private slots:
         void open();
         bool saveas();
         void tagadd();
         void tagremove();
         void edittagtypes();
         void tagtypeadd();
         void tagtyperemove();
         void untag();
         void addtagtype();
         void about();
         void aboutQt();
         void sarfTagging();

public:
         virtual void report(int value);
         virtual void tag(int start, int length,QColor color, bool textcolor);
         virtual void setCurrentAction(const QString & s);
         virtual void resetActionDisplay();
         virtual QString getFileName();

         void fillTreeWidget(Source Data);
         void finishTaggingText();
         void process(QByteArray & json);
         void applyTags();

private:
         void createActions();
         void createMenus();
         void createTagMenu();
         void createDockWindows();
         void startTaggingText(QString & text);
         bool saveFile(const QString &fileName, QByteArray &tagD);

         QMenu *fileMenu;
         QMenu *tagMenu;
         QMenu *tagtypeMenu;
         QMenu *editMenu;
         QMenu *viewMenu;
         QMenu *helpMenu;
         QMenu * mTags;
         QMenu *sarfMenu;

         QAction *openAct;
         QAction *saveAct;
         QAction *saveasAct;
         QAction *exitAct;
         QAction *tagaddAct;
         QAction *tagremoveAct;
         QAction *edittagtypesAct;
         QAction *tagtypeaddAct;
         QAction *tagtyperemoveAct;
         QAction *tagAct;
         QAction *untagMAct;
         QAction *addtagAct;
         QAction *aboutAct;
         QAction *aboutQtAct;
         QAction *sarfAct;

         QTextBrowser * txtBrwsr;
         //QTextEdit * txtBrwsr;
         QTreeWidget * tagDescription;
         QTreeWidget * descBrwsr;

public:
         explicit AMTMainWindow(QWidget *parent = 0);
         ~AMTMainWindow();
    
private:
         QFileDialog * browseFileDlg;
         QSignalMapper *signalMapper;
         QSignalMapper *signalMapperM;
};

#endif // AMTMAINWINDOW_H

#ifndef AMTMAINWINDOW_H
#define AMTMAINWINDOW_H

#include <QMainWindow>
#include<QFileDialog>
#include<QTextBrowser>
#include<QTreeWidget>
#include<QTextEdit>

#include <QMenu>
#include <QMenuBar>

class AMTMainWindow : public QMainWindow
{
    Q_OBJECT

    protected:
         void contextMenuEvent(QContextMenuEvent *event);
         void showContextMenu(const QPoint &pt);

     private slots:
         void open();
         void save();
         bool saveas();
         void tagadd();
         void tagremove();
         void tagtypeadd();
         void tagtyperemove();
         void tag(char *);
         void untag();
         void addtagtype();
         void about();
         void aboutQt();

     private:
         void createActions();
         void createMenus();
         void createDockWindows();
         void fillTreeWidget();
         void startTaggingText(QString & text);
         void finishTaggingText();
         void process(QByteArray & json);
         void tagWord(int start, int length, QColor fcolor, QColor bcolor,int font, bool underline, bool italic, bool bold);
         bool saveFile(const QString &fileName, QByteArray &tagD, QByteArray &tagTD);

         QMenu *fileMenu;
         QMenu *tagMenu;
         QMenu *tagtypeMenu;
         QMenu *editMenu;
         QMenu *viewMenu;
         QMenu *helpMenu;

         QAction *openAct;
         QAction *saveAct;
         QAction *saveasAct;
         QAction *exitAct;
         QAction *tagaddAct;
         QAction *tagremoveAct;
         QAction *tagtypeaddAct;
         QAction *tagtyperemoveAct;
         QAction *tagAct;
         QAction *untagAct;
         QAction *addtagAct;
         QAction *aboutAct;
         QAction *aboutQtAct;

         QTextBrowser * txtBrwsr;
         //QTextEdit * txtBrwsr;
         QTreeWidget * tagDescription;
         QTextBrowser * descBrwsr;

    
public:
         explicit AMTMainWindow(QWidget *parent = 0);
         ~AMTMainWindow();
    
private:
         QFileDialog * browseFileDlg;
};

#endif // AMTMAINWINDOW_H

#ifndef AMTMAINWINDOW_H
#define AMTMAINWINDOW_H

#include <QMainWindow>
#include<QFileDialog>

namespace Ui {
class AMTMainWindow;
}

class AMTMainWindow : public QMainWindow
{
    Q_OBJECT

    protected:
         void contextMenuEvent(QContextMenuEvent *event);

     private slots:
         void open();
         void save();
         void saveas();
         void cut();
         void copy();
         void paste();
         void about();
         void aboutQt();

     private:
         void createActions();
         void createMenus();
         void startTaggingText(QString & text);
         void finishTaggingText();
         void process(QByteArray & json);
         void tagWord(int start, int length, QColor fcolor, QColor bcolor);

         QMenu *fileMenu;
         QMenu *editMenu;
         QMenu *helpMenu;
         QAction *openAct;
         QAction *saveAct;
         QAction *saveasAct;
         QAction *exitAct;
         QAction *cutAct;
         QAction *copyAct;
         QAction *pasteAct;
         QAction *aboutAct;
         QAction *aboutQtAct;
    
public:
         explicit AMTMainWindow(QWidget *parent = 0);
         ~AMTMainWindow();
    
private:
         Ui::AMTMainWindow *ui;
         QFileDialog * browseFileDlg;
};

#endif // AMTMAINWINDOW_H

#ifndef REMOVETAGTYPEVIEW_H
#define REMOVETAGTYPEVIEW_H

#include <QMainWindow>
#include <QTextBrowser>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <QGridLayout>
#include <QTreeWidget>

class RemoveTagTypeView : public QMainWindow
{
    Q_OBJECT
public:
    explicit RemoveTagTypeView(QTextBrowser *txtBrwsr,QTreeWidget * tagDescription, QWidget *parent = 0);

public slots:
    void removeTagType_clicked();

signals:
    void tagWordS(int start, int length, QColor bgcolor, QColor fgcolor, int font, bool underline, bool bold, bool italic);
    void fillTreeWidgetS();

private:
    QLabel * lblType;
    QComboBox * cbType;
    QPushButton * btnRemoveTagType;
    QScrollArea *scrollArea;
    QGridLayout * grid;
    QWidget * parent;
    QTextBrowser *txtBrwsr;
    QTreeWidget * tagDescription;



};

#endif // REMOVETAGTYPEVIEW_H

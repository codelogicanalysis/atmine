#ifndef ADDTAGVIEW_H
#define ADDTAGVIEW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QGridLayout>
#include <QTextBrowser>
#include <QComboBox>
#include "amtmainwindow.h"

class AddTagView : public QMainWindow
{
    Q_OBJECT
public:
    //explicit AddTagView(int _start, int _end, QWidget *parent = 0);
    explicit AddTagView(QTextBrowser *txtBrwsr, QWidget *parent = 0);

public slots:
    void addTag_clicked();

private:
    QLabel * lblType;
    QLabel * lblPOS;
    QLabel * lblLength;
    QComboBox * cbType;
    QLineEdit * lineEditPOS;
    QLineEdit * lineEditLength;
    QPushButton * btnAddTag;
    QScrollArea *scrollArea;
    QGridLayout * grid;
    QTextBrowser *txtBrwsr;
};

#endif // ADDTAGVIEW_H

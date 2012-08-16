#ifndef ADDTAGVIEW_H
#define ADDTAGVIEW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QGridLayout>

class AddTagView : public QMainWindow
{
    Q_OBJECT
public:
    explicit AddTagView(QWidget *parent = 0);

public slots:
    void addTag_clicked();

private:
    QLabel * lblType;
    QLabel * lblPOS;
    QLabel * lblLength;
    QLineEdit * lineEditType;
    QLineEdit * lineEditPOS;
    QLineEdit * lineEditLength;
    QPushButton * btnAddTag;
    QScrollArea *scrollArea;
    QGridLayout * grid;

};

#endif // ADDTAGVIEW_H

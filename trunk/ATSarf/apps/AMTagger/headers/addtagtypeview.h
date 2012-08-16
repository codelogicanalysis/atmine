#ifndef ADDTAGTYPEVIEW_H
#define ADDTAGTYPEVIEW_H

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QGridLayout>
#include <QColorDialog>

class AddTagTypeView : public QMainWindow
{
    Q_OBJECT
public:
    explicit AddTagTypeView(QWidget *parent = 0);

public slots:
    void addTagType_clicked();

private:
    QLabel * lblTag;
    QLabel * lblDescription;
    QLabel * lblfgcolor;
    QLabel * lblbgcolor;
    QLabel * lblfont;
    QLabel * lblunderline;
    QLabel * lblbold;
    QLabel * lblitalic;
    QLineEdit * lineEditTag;
    QLineEdit * lineEditDescription;
    QLineEdit * lineEditfgcolor;
    QColorDialog * colorfgcolor;
    QLineEdit * lineEditbgcolor;
    QColorDialog * colorbgcolor;
    QLineEdit * lineEditfont;
    QLineEdit * lineEditunderline;
    QLineEdit * lineEditBold;
    QLineEdit * lineEditItalic;
    QPushButton * btnAddTagType;
    QScrollArea *scrollArea;
    QGridLayout * grid;

};

#endif // ADDTAGTYPEVIEW_H

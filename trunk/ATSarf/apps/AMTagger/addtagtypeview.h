#ifndef ADDTAGTYPEVIEW_H
#define ADDTAGTYPEVIEW_H

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QGridLayout>
#include <QColorDialog>
#include <QCheckBox>
#include <QComboBox>
#include "edittagtypeview.h"
#include "colorlisteditor.h"

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
    ColorListEditor * colorfgcolor;
    ColorListEditor * colorbgcolor;
    QComboBox * cbfont;
    QCheckBox * cbunderline;
    QCheckBox * cbBold;
    QCheckBox * cbItalic;
    QPushButton * btnAddTagType;
    QScrollArea *scrollArea;
    QGridLayout * grid;

};

#endif // ADDTAGTYPEVIEW_H

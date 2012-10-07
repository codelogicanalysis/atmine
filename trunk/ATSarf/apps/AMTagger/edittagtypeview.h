#ifndef EDITTAGTYPEVIEW_H
#define EDITTAGTYPEVIEW_H

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QGridLayout>
#include <QColorDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFontComboBox>
#include <QMenu>
#include "amtmainwindow.h"
#include "colorlisteditor.h"
#include "tagtype.h"

class EditTagTypeView : public QMainWindow
{
    Q_OBJECT
public:
    explicit EditTagTypeView(QWidget *parent = 0);
    //bool showWindow;

private:
    void createMenus();

public slots:
    void update_TagTypes();
    void add_clicked();
    void rmv_clicked();
    void edit_clicked();
    void save_clicked();
    void load_clicked();
    void item_clicked();

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
    //QFontComboBox * cbfont;
    QComboBox * cbfont;
    QCheckBox * cbunderline;
    QCheckBox * cbBold;
    QCheckBox * cbItalic;
    QPushButton * btnAdd;
    QPushButton * btnRmv;
    QPushButton * btnEdit;
    QPushButton * btnSave;
    QPushButton * btnLoad;
    QListWidget * lvTypes;
    QScrollArea *scrollArea;
    QGridLayout * grid;
    QGridLayout * gButtons1;
    QGridLayout * gButtons2;
    QGridLayout * gVertical1;
    QGridLayout * gVertical2;
    QGridLayout * gFinal;
    QScrollArea *scroll1;
    QScrollArea *scroll2;
    QScrollArea *scroll3;
    bool edit;
    QVector<TagType*> *tagTypeVector;
    QMenu *viewMenu;
};
#endif // EDITTAGTYPEVIEW_H

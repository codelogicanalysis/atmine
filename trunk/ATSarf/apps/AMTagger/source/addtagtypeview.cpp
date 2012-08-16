#include "addtagtypeview.h"

AddTagTypeView::AddTagTypeView(QWidget *parent) :
    QMainWindow(parent)
{
    lblTag = new QLabel(this);
    lblDescription = new QLabel(this);
    lblfgcolor = new QLabel(this);
    lblbgcolor = new QLabel(this);
    lblfont = new QLabel(this);
    lblunderline = new QLabel(this);
    lblbold = new QLabel(this);
    lblitalic = new QLabel(this);

    lblTag->setText("Tag:");
    lblDescription->setText("Description:");
    lblfgcolor->setText("Foreground Color:");
    lblbgcolor->setText("Background Color:");
    lblfont->setText("Font:");
    lblunderline->setText("Underline:");
    lblbold->setText("Bold:");
    lblitalic->setText("Italic:");

    lineEditTag = new QLineEdit(this);
    lineEditDescription = new QLineEdit(this);
    lineEditfgcolor = new QLineEdit(this);
    lineEditbgcolor = new QLineEdit(this);
    lineEditfont = new QLineEdit(this);
    lineEditunderline = new QLineEdit(this);
    lineEditBold = new QLineEdit(this);
    lineEditItalic = new QLineEdit(this);

    btnAddTagType = new QPushButton(this);
    btnAddTagType->setText("Add TagType");

    scrollArea=new QScrollArea(this);
    grid=new QGridLayout(scrollArea);
    grid->addWidget(lblTag,0,0);
    grid->addWidget(lineEditTag,0,1);
    grid->addWidget(lblDescription,1,0);
    grid->addWidget(lineEditDescription,1,1);
    grid->addWidget(lblfgcolor,2,0);
    grid->addWidget(lineEditfgcolor,2,1);
    grid->addWidget(lblbgcolor,3,0);
    grid->addWidget(lineEditbgcolor,3,1);
    grid->addWidget(lblfont,4,0);
    grid->addWidget(lineEditfont,4,1);
    grid->addWidget(lblunderline,5,0);
    grid->addWidget(lineEditunderline,5,1);
    grid->addWidget(lblbold,6,0);
    grid->addWidget(lineEditBold,6,1);
    grid->addWidget(lblitalic,7,0);
    grid->addWidget(lineEditItalic,7,1);
    grid->addWidget(btnAddTagType,8,0,1,2,Qt::AlignCenter);
    setCentralWidget(scrollArea);

    connect(btnAddTagType,SIGNAL(clicked()),this,SLOT(addTagType_clicked()));

    setWindowTitle(tr("Add Tag"));
    resize(500,300);

}

void AddTagTypeView::addTagType_clicked() {

}

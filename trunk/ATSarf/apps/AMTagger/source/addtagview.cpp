#include "addtagview.h"

AddTagView::AddTagView(QWidget *parent) :
    QMainWindow(parent)
{
    lblType = new QLabel(this);
    lblPOS = new QLabel(this);
    lblLength = new QLabel(this);
    lblType->setText("Type:");
    lblPOS->setText("Position:");
    lblLength->setText("Length:");
    lineEditType = new QLineEdit(this);
    lineEditPOS = new QLineEdit(this);
    lineEditLength = new QLineEdit(this);
    btnAddTag = new QPushButton(this);
    btnAddTag->setText("Add Tag");

    scrollArea=new QScrollArea(this);
    grid=new QGridLayout(scrollArea);
    grid->addWidget(lblType,0,0);
    grid->addWidget(lineEditType,0,1);
    grid->addWidget(lblPOS,1,0);
    grid->addWidget(lineEditPOS,1,1);
    grid->addWidget(lblLength,2,0);
    grid->addWidget(lineEditLength,2,1);
    grid->addWidget(btnAddTag,3,0,1,2,Qt::AlignCenter);

    setCentralWidget(scrollArea);

    connect(btnAddTag,SIGNAL(clicked()),this,SLOT(addTag_clicked()));

    setWindowTitle(tr("Add Tag"));
}

void AddTagView::addTag_clicked() {

}

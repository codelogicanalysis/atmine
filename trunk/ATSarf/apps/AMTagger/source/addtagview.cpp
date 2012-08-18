#include <QTextCursor>
#include "addtagview.h"
#include "global.h"
#include <sstream>

AddTagView::AddTagView(int _start, int _end, QWidget *parent) :
    QMainWindow(parent)
{
    start = _start;
    end = _end;
    lblType = new QLabel(this);
    lblPOS = new QLabel(this);
    lblLength = new QLabel(this);
    lblType->setText("Type:");
    lblPOS->setText("Position:");
    lblLength->setText("Length:");
    cbType = new QComboBox(this);
    lineEditPOS = new QLineEdit(this);
    lineEditLength = new QLineEdit(this);
    btnAddTag = new QPushButton(this);
    btnAddTag->setText("Add Tag");

    scrollArea=new QScrollArea(this);
    grid=new QGridLayout(scrollArea);
    grid->addWidget(lblType,0,0);
    grid->addWidget(cbType,0,1);
    grid->addWidget(lblPOS,1,0);
    grid->addWidget(lineEditPOS,1,1);
    grid->addWidget(lblLength,2,0);
    grid->addWidget(lineEditLength,2,1);
    grid->addWidget(btnAddTag,3,0,1,2,Qt::AlignCenter);

    for(int i=0; i< _atagger->tagTypeVector->count(); i++) {
        QString type = (_atagger->tagTypeVector->at(i)).tag;
        cbType->insertItem(i,type);
    }
    std::stringstream out1;
    out1 << start;
    std::string str1 = out1.str();
    lineEditPOS->setText(QString::fromStdString(str1));
    std::stringstream out2;
    out2 << (end - start);
    std::string str2 = out2.str();
    lineEditLength->setText(QString::fromStdString(str2));

    setCentralWidget(scrollArea);

    connect(btnAddTag,SIGNAL(clicked()),this,SLOT(addTag_clicked()));

    setWindowTitle(tr("Add Tag"));
}

void AddTagView::addTag_clicked() {

    _atagger->insertTag("Verb",start, end, user);
}

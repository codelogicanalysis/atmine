#include <QTextCursor>
#include "addtagview.h"
#include "global.h"
#include <sstream>

AddTagView::AddTagView(QTextBrowser *txtBrwsr, QWidget *parent) :
    QMainWindow(parent)
{
    //start = _start;
    //end = _end;
    parent = parent;
    this->txtBrwsr = txtBrwsr;
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
    out1 << txtBrwsr->textCursor().selectionStart();
    std::string str1 = out1.str();
    lineEditPOS->setText(QString::fromStdString(str1));
    std::stringstream out2;
    out2 << (txtBrwsr->textCursor().selectionEnd()-txtBrwsr->textCursor().selectionStart());
    std::string str2 = out2.str();
    lineEditLength->setText(QString::fromStdString(str2));

    setCentralWidget(scrollArea);

    connect(btnAddTag,SIGNAL(clicked()),this,SLOT(addTag_clicked()));

    setWindowTitle(tr("Add Tag"));
}

void AddTagView::addTag_clicked() {
    QTextCursor cursor = txtBrwsr->textCursor();
    QString tag = cbType->currentText();
    _atagger->insertTag(tag,cursor.selectionStart(), cursor.selectionEnd(), user);

    for(int i=0; i< _atagger->tagTypeVector->count(); i++) {
        if((_atagger->tagTypeVector->at(i)).tag == tag) {
            QColor bgcolor((_atagger->tagTypeVector->at(i)).bgcolor);
            QColor fgcolor((_atagger->tagTypeVector->at(i)).fgcolor);
            int font = (_atagger->tagTypeVector->at(i)).font;
            bool underline = (_atagger->tagTypeVector->at(i)).underline;
            bool bold = (_atagger->tagTypeVector->at(i)).bold;
            bool italic = (_atagger->tagTypeVector->at(i)).italic;

            ((AMTMainWindow*)parentWidget())->tagWord(cursor.selectionStart(),cursor.selectionEnd()-cursor.selectionStart(),fgcolor,bgcolor,font,underline,italic,bold);
        }
    }
    cursor.clearSelection();
    ((AMTMainWindow*)parentWidget())->fillTreeWidget();
    this->close();
}

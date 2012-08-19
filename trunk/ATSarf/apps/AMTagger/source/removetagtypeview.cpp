#include "removetagtypeview.h"
#include "global.h"
#include "atagger.h"
#include "amtmainwindow.h"

RemoveTagTypeView::RemoveTagTypeView(QTextBrowser *txtBrwsr,QTreeWidget * tagDescription, QWidget *parent) {
    this->txtBrwsr = txtBrwsr;
    this->tagDescription = tagDescription;
    lblType = new QLabel(this);
    cbType = new QComboBox(this);
    btnRemoveTagType = new QPushButton(this);

    lblType->setText("Tag Type: ");
    btnRemoveTagType->setText("Remove TagType");
    for(int i=0; i < _atagger->tagTypeVector->count(); i++) {
        cbType->addItem((_atagger->tagTypeVector->at(i)).tag);
    }

    scrollArea=new QScrollArea(this);
    grid=new QGridLayout(scrollArea);
    grid->addWidget(lblType,0,0);
    grid->addWidget(cbType,0,1);
    grid->addWidget(btnRemoveTagType,1,0,1,2,Qt::AlignCenter);

    connect(this,
            SIGNAL(tagWordS(int, int, QColor, QColor, int, bool, bool, bool)),
            (AMTMainWindow*)parentWidget(),
            SLOT(tagWord(int, int, QColor, QColor, int, bool, bool, bool)));

    connect(this, SIGNAL(fillTreeWidgetS()),(AMTMainWindow*)parentWidget(),SLOT(fillTreeWidget()));

    setCentralWidget(scrollArea);
    connect(btnRemoveTagType,SIGNAL(clicked()),this,SLOT(removeTagType_clicked()));
    setWindowTitle(tr("Add Tag"));
}

void RemoveTagTypeView::removeTagType_clicked() {
    QString tag = cbType->currentText();
    for(int i=0; i < _atagger->tagTypeVector->count(); i++) {
        if((_atagger->tagTypeVector->at(i)).tag == tag) {
            for(int j=0; j < _atagger->tagVector->count(); j++) {
                if((_atagger->tagVector->at(j)).type == tag) {
                    int start = (_atagger->tagVector->at(j)).pos;
                    int length = (_atagger->tagVector->at(j)).length;
                    emit tagWordS(start,length,QColor("black"),QColor("white"),9,false,false,false);
                    _atagger->tagVector->remove(j);
                }
            }
            _atagger->tagTypeVector->remove(i);
            emit fillTreeWidgetS();
            break;
        }
    }
    this->close();
}

/*
void RemoveTagTypeView::tagWordS(int start, int length, QColor bgcolor, QColor fgcolor, int font, bool underline, bool bold, bool italic) {

}

void RemoveTagTypeView::fillTreeWidgetS() {

}
*/

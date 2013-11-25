#include "removetagtypeview.h"
#include "global.h"
#include "atagger.h"
#include "amtmainwindow.h"

RemoveTagTypeView::RemoveTagTypeView(QTextBrowser *txtBrwsr,QTreeWidget * tagDescription, QWidget *parent) :
        QMainWindow(parent)
{
    this->txtBrwsr = txtBrwsr;
    this->tagDescription = tagDescription;
    lblType = new QLabel(this);
    cbType = new QComboBox(this);
    btnRemoveTagType = new QPushButton(this);

    lblType->setText("Tag Type: ");
    btnRemoveTagType->setText("Remove TagType");
    for(int i=0; i < _atagger->tagTypeVector->count(); i++) {
        cbType->addItem((_atagger->tagTypeVector->at(i))->name);
    }

    scrollArea=new QScrollArea(this);
    grid=new QGridLayout(scrollArea);
    grid->addWidget(lblType,0,0);
    grid->addWidget(cbType,0,1);
    grid->addWidget(btnRemoveTagType,1,0,1,2,Qt::AlignCenter);

    setCentralWidget(scrollArea);
    connect(btnRemoveTagType,SIGNAL(clicked()),this,SLOT(removeTagType_clicked()));
    setWindowTitle(tr("Add Tag"));
}

void RemoveTagTypeView::removeTagType_clicked() {
    QString tagtype = cbType->currentText();
    for(int i=0; i < _atagger->tagTypeVector->count(); i++) {
        if((_atagger->tagTypeVector->at(i))->name == tagtype) {
            _atagger->tagTypeVector->remove(i);

            /// Remove all tags based on removed tag type
            QHashIterator<int, Tag> iTag(_atagger->tagHash);
            while (iTag.hasNext()) {
                iTag.next();
                if(iTag.value().tagtype->name == tagtype) {
                    _atagger->tagHash.remove(iTag.value().wordIndex,iTag.value());
                }
            }

            if(_atagger->isTagMBF) {
                ((AMTMainWindow*)parentWidget())->applyTags(0);
            }
            break;
        }
    }
    this->close();
}

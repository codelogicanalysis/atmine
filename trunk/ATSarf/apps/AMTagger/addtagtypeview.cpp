#include "addtagtypeview.h"
#include "atagger.h"
#include "global.h"
#include <QItemEditorFactory>
#include <sstream>
#include <QMessageBox>

AddTagTypeView::AddTagTypeView(QWidget *parent) :
    QMainWindow(parent)
{
    /*
    QItemEditorFactory *factory = new QItemEditorFactory;
    QItemEditorCreatorBase *colorListCreator = new QStandardItemEditorCreator<ColorListEditor>();
    factory->registerEditor(QVariant::Color, colorListCreator);
    QItemEditorFactory::setDefaultFactory(factory);
    */
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
    //lineEditfgcolor = new QLineEdit(this);
    //lineEditbgcolor = new QLineEdit(this);
    colorfgcolor = new ColorListEditor(this);
    colorbgcolor = new ColorListEditor(this);
    cbfont = new QComboBox(this);
    for(int i=5; i<20; i++) {
        std::stringstream out;
        out << i;
        std::string str = out.str();
        cbfont->addItem(QString::fromStdString(str));
    }
    cbunderline = new QCheckBox(this);
    cbBold = new QCheckBox(this);
    cbItalic = new QCheckBox(this);

    btnAddTagType = new QPushButton(this);
    btnAddTagType->setText("Add TagType");

    scrollArea=new QScrollArea(this);
    grid=new QGridLayout(scrollArea);
    grid->addWidget(lblTag,0,0);
    grid->addWidget(lineEditTag,0,1);
    grid->addWidget(lblDescription,1,0);
    grid->addWidget(lineEditDescription,1,1);
    grid->addWidget(lblfgcolor,2,0);
    grid->addWidget(colorfgcolor,2,1);
    grid->addWidget(lblbgcolor,3,0);
    grid->addWidget(colorbgcolor,3,1);
    grid->addWidget(lblfont,4,0);
    grid->addWidget(cbfont,4,1);
    grid->addWidget(lblunderline,5,0);
    grid->addWidget(cbunderline,5,1);
    grid->addWidget(lblbold,6,0);
    grid->addWidget(cbBold,6,1);
    grid->addWidget(lblitalic,7,0);
    grid->addWidget(cbItalic,7,1);
    grid->addWidget(btnAddTagType,8,0,1,2,Qt::AlignCenter);
    setCentralWidget(scrollArea);

    connect(btnAddTagType,SIGNAL(clicked()),this,SLOT(addTagType_clicked()));

    setWindowTitle(tr("Add Tag"));
    resize(500,300);

}

void AddTagTypeView::addTagType_clicked() {
    QString tag = lineEditTag->text();
    for(int i=0; i < _atagger->tagTypeVector->count(); i++) {
        if((_atagger->tagTypeVector->at(i)).tag == tag) {

            switch( QMessageBox::information( this, "Tag Type","The <b>TagType</b> is duplicate!","&Ok",0,0) ) {
                return;
            }
        }
    }
    QString description = lineEditDescription->text();
    int id = _atagger->tagTypeVector->count();
    QString fgcolor = colorfgcolor->color().name();
    QString bgcolor = colorbgcolor->color().name();
    int font = cbfont->currentText().toInt();
    bool underline = cbunderline->isChecked();
    bool bold = cbBold->isChecked();
    bool italic = cbItalic->isChecked();
    _atagger->insertTagType(tag,description,id,fgcolor,bgcolor,font,underline,bold,italic);
    if(parentCheck) {
        parentCheck = false;
        ((EditTagTypeView*)parentWidget())->update_TagTypes();
    }

    this->close();
}

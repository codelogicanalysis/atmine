#include "edittagtypeview.h"
#include "addtagtypeview.h"
#include "atagger.h"
#include "global.h"
#include <QItemEditorFactory>
#include <sstream>
#include <QMessageBox>
#include <QColor>
#include <QDockWidget>
#include <QInputDialog>
#include <QTextStream>

EditTagTypeView::EditTagTypeView(QWidget *parent) :
    QMainWindow(parent)
{
    //resize(700,500);

    setWindowTitle("Tag Types: " + _atagger->tagtypeFile);

    /** Create Menu **/
    //createMenus();

    QGridLayout *grid = new QGridLayout();

    /** Initialize Elements **/

    lblTag = new QLabel(this);
    lblDescription = new QLabel(this);
    lblfgcolor = new QLabel(this);
    lblbgcolor = new QLabel(this);
    lblfont = new QLabel(this);
    lblunderline = new QLabel(this);
    lblbold = new QLabel(this);
    lblitalic = new QLabel(this);

    lblTag->setText("Name:");
    lblDescription->setText("Description:");
    lblfgcolor->setText("Foreground Color:");
    lblbgcolor->setText("Background Color:");
    lblfont->setText("Font:");
    lblunderline->setText("Underline:");
    lblbold->setText("Bold:");
    lblitalic->setText("Italic:");

    grid->addWidget(lblTag,0,3);
    grid->addWidget(lblDescription,1,3);
    grid->addWidget(lblfgcolor,2,3);
    grid->addWidget(lblbgcolor,3,3);
    grid->addWidget(lblfont,4,3);
    grid->addWidget(lblunderline,5,3);
    grid->addWidget(lblbold,6,3);
    grid->addWidget(lblitalic,7,3);

    lineEditTag = new QLineEdit(this);
    lineEditTag->setEnabled(false);
    lineEditDescription = new QLineEdit(this);
    lineEditDescription->setEnabled(false);

    grid->addWidget(lineEditTag,0,4);
    grid->addWidget(lineEditDescription,1,4);

    colorfgcolor = new ColorListEditor(this);
    colorfgcolor->setColor("Red");
    colorfgcolor->setEnabled(false);
    colorbgcolor = new ColorListEditor(this);
    colorbgcolor->setColor("Yellow");
    colorbgcolor->setEnabled(false);

    grid->addWidget(colorfgcolor,2,4);
    grid->addWidget(colorbgcolor,3,4);

    cbfont = new QComboBox(this);
    cbfont->setEnabled(false);
    for(int i=5; i<20; i++) {
        std::stringstream out;
        out << i;
        std::string str = out.str();
        cbfont->addItem(QString::fromStdString(str));
    }

    grid->addWidget(cbfont,4,4);

    cbunderline = new QCheckBox(this);
    cbunderline->setEnabled(false);
    cbBold = new QCheckBox(this);
    cbBold->setEnabled(false);
    cbItalic = new QCheckBox(this);
    cbItalic->setEnabled(false);

    grid->addWidget(cbunderline,5,4);
    grid->addWidget(cbBold,6,4);
    grid->addWidget(cbItalic,7,4);

    btnAdd = new QPushButton(this);
    btnAdd->setText("+");
    btnAdd->setFixedSize(40,25);
    connect(btnAdd,SIGNAL(clicked()),this,SLOT(add_clicked()));

    grid->addWidget(btnAdd,8,0);

    btnRmv = new QPushButton(this);
    btnRmv->setText("-");
    btnRmv->setFixedSize(40,25);
    connect(btnRmv,SIGNAL(clicked()),this,SLOT(rmv_clicked()));

    grid->addWidget(btnRmv,8,1);

    btnEdit = new QPushButton(this);
    btnEdit->setText("...");
    btnEdit->setFixedSize(40,25);
    connect(btnEdit,SIGNAL(clicked()),this,SLOT(edit_clicked()));

    grid->addWidget(btnEdit,8,2);

    btnSave = new QPushButton(this);
    btnSave->setText("Save");
    //btnSave->setFixedSize(60,30);
    connect(btnSave,SIGNAL(clicked()),this,SLOT(save_clicked()));

    grid->addWidget(btnSave,8,3);

    btnLoad = new QPushButton(this);
    btnLoad->setText("Load");
    //`btnLoad->setFixedSize(60,30);
    connect(btnLoad,SIGNAL(clicked()),this,SLOT(load_clicked()));

    grid->addWidget(btnLoad,8,4);

    lvTypes = new QListWidget(this);
    lvTypes->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(lvTypes, SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(item_clicked()));

    grid->addWidget(lvTypes,0,0,8,3);

    QWidget *widget = new QWidget(this);

    widget->setLayout(grid);
    setCentralWidget(widget);

    /** Fill Data in View **/

    tagTypeVector = new QVector<TagType>();

    if(_atagger->tagTypeVector->count() == 0) {
        btnEdit->setEnabled(false);
        return;
    }

    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        tagTypeVector->append(_atagger->tagTypeVector->at(i));
    }

    int tagtypeCount = tagTypeVector->count();
    for( int i=0; i< tagtypeCount; i++) {
        lvTypes->addItem((tagTypeVector->at(i)).tag);
    }

    lvTypes->item(0)->setSelected(true);
    lineEditTag->setText((tagTypeVector->at(0)).tag);
    lineEditDescription->setText((tagTypeVector->at(0)).description);
    colorfgcolor->setColor(QColor((tagTypeVector->at(0)).fgcolor));
    colorbgcolor->setColor(QColor((tagTypeVector->at(0)).bgcolor));
    int index = cbfont->findText(QString::number((tagTypeVector->at(0)).font));
    if(index != -1) {
        cbfont->setCurrentIndex(index);
    }
    cbunderline->setChecked((tagTypeVector->at(0)).underline);
    cbBold->setChecked((tagTypeVector->at(0)).bold);
    cbItalic->setChecked((tagTypeVector->at(0)).italic);

    edit = false;

    setWindowTitle(tr("Edit Tag Type DialogBox"));
}

void EditTagTypeView::update_TagTypes() {
    lvTypes->clear();
    int tagtypeCount = tagTypeVector->count();
    for( int i=0; i< tagtypeCount; i++) {
        lvTypes->addItem((tagTypeVector->at(i)).tag);
    }

    lvTypes->item(0)->setSelected(true);
    lineEditTag->setText((tagTypeVector->at(0)).tag);
    lineEditDescription->setText((tagTypeVector->at(0)).description);
    colorfgcolor->setColor(QColor((tagTypeVector->at(0)).fgcolor));
    colorbgcolor->setColor(QColor((tagTypeVector->at(0)).bgcolor));
    int index = cbfont->findText(QString::number((tagTypeVector->at(0)).font));
    if(index != -1) {
        cbfont->setCurrentIndex(index);
    }
    cbunderline->setChecked((tagTypeVector->at(0)).underline);
    cbBold->setChecked((tagTypeVector->at(0)).bold);
    cbItalic->setChecked((tagTypeVector->at(0)).italic);

    edit = false;
}

void EditTagTypeView::createMenus() {
    viewMenu = menuBar()->addMenu(tr("&View"));

    /*
    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
    */
}

void EditTagTypeView::add_clicked() {
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add New Tag"),
                                             tr("Tag Name:"), QLineEdit::Normal,
                                             QString(), &ok);
    if(!ok || text.isEmpty())
        return;

    for(int i=0; i< tagTypeVector->count(); i++) {
        if((tagTypeVector->at(i)).tag == text) {
            QMessageBox::warning(this,"Warning","Tag Type Name already Present!");
            return;
        }
    }

    lvTypes->addItem(text);
    lvTypes->setCurrentRow(lvTypes->count()-1);

    edit= true;
    btnEdit->setText("lock");
    lineEditTag->setEnabled(true);
    lineEditTag->setText(text);
    lineEditDescription->setEnabled(true);
    lineEditDescription->setText(QString());
    colorfgcolor->setEnabled(true);
    colorbgcolor->setEnabled(true);
    cbfont->setEnabled(true);
    cbunderline->setEnabled(true);
    cbunderline->setChecked(false);
    cbBold->setEnabled(true);
    cbBold->setChecked(false);
    cbItalic->setEnabled(true);
    cbItalic->setChecked(false);

    if(_atagger->tagTypeVector->count() == 0) {
        btnEdit->setEnabled(true);
    }
}

void EditTagTypeView::rmv_clicked() {

    int index = lvTypes->currentRow();
    if(index != -1 && index < tagTypeVector->count()) {
        tagTypeVector->remove(index);
    }
    update_TagTypes();
}

void EditTagTypeView::edit_clicked() {
    if(!edit) {
        btnEdit->setText("Lock");
        edit= true;
        lineEditTag->setEnabled(true);
        lineEditDescription->setEnabled(true);
        colorfgcolor->setEnabled(true);
        colorbgcolor->setEnabled(true);
        cbfont->setEnabled(true);
        cbunderline->setEnabled(true);
        cbBold->setEnabled(true);
        cbItalic->setEnabled(true);
    }
    else {

        btnEdit->setText("...");
        edit = false;
        lineEditTag->setEnabled(false);
        lineEditDescription->setEnabled(false);
        colorfgcolor->setEnabled(false);
        colorbgcolor->setEnabled(false);
        cbfont->setEnabled(false);
        cbunderline->setEnabled(false);
        cbBold->setEnabled(false);
        cbItalic->setEnabled(false);

        int index = lvTypes->currentRow();
        QString tag = lineEditTag->text();
        QString description = lineEditDescription->text();
        QString fgcolor = colorfgcolor->color().name();
        QString bgcolor = colorbgcolor->color().name();
        int font = cbfont->currentText().toInt();
        bool underline = cbunderline->isChecked();
        bool bold = cbBold->isChecked();
        bool italic = cbItalic->isChecked();
        if(index >= tagTypeVector->count()) {
            TagType t(tag,description,tagTypeVector->count(), fgcolor,bgcolor,font,underline,bold,italic);
            tagTypeVector->append(t);
        }
        else {
            (*tagTypeVector)[index].tag = tag;
            (*tagTypeVector)[index].description = description;
            (*tagTypeVector)[index].fgcolor = fgcolor;
            (*tagTypeVector)[index].bgcolor = bgcolor;
            (*tagTypeVector)[index].font = font;
            (*tagTypeVector)[index].underline = underline;
            (*tagTypeVector)[index].bold = bold;
            (*tagTypeVector)[index].italic = italic;
        }
        update_TagTypes();
    }
}

void EditTagTypeView::save_clicked() {
    *(_atagger->tagTypeVector) = *(tagTypeVector);
    ((AMTMainWindow*)parentWidget())->applyTags();

    /** Save to Default Destination **/

    QByteArray tagtypeData = _atagger->dataInJsonFormat(tagTV);
    QFile tfile(_atagger->tagtypeFile);
    if (!tfile.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this,"Warning","Can't open tagtypes file to Save");
        return;
    }

    QTextStream outtags(&tfile);
    outtags << tagtypeData;
    tfile.close();
}

void EditTagTypeView::load_clicked() {

    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Open Tag Types"), "",
             tr("Tag Types (*.tt.json);;All Files (*)"));

         if (fileName.isEmpty())
             return;
         else {
             QFile file(fileName);
             if (!file.open(QIODevice::ReadOnly)) {
                 QMessageBox::information(this, tr("Unable to open file"),file.errorString());
                 return;
             }

             QByteArray TagTypes = file.readAll();
             file.close();
             ((AMTMainWindow*)parentWidget())->process_TagTypes(TagTypes);
             //((AMTMainWindow*)parentWidget())->finishTaggingText();

             tagTypeVector = new QVector<TagType>();
             for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
                 tagTypeVector->append(_atagger->tagTypeVector->at(i));
             }

             update_TagTypes();
         }
}

void EditTagTypeView::item_clicked() {
    edit = false;
    lineEditTag->setEnabled(false);
    lineEditDescription->setEnabled(false);
    colorfgcolor->setEnabled(false);
    colorbgcolor->setEnabled(false);
    cbfont->setEnabled(false);
    int index = lvTypes->currentRow();
    lineEditTag->setText((tagTypeVector->at(index)).tag);
    lineEditDescription->setText((tagTypeVector->at(index)).description);
    colorfgcolor->setColor(QColor((tagTypeVector->at(index)).fgcolor));
    colorbgcolor->setColor(QColor((tagTypeVector->at(index)).bgcolor));
    int fontindex = cbfont->findText(QString::number((tagTypeVector->at(index)).font));
    if(fontindex != -1) {
        cbfont->setCurrentIndex(fontindex);
    }
    cbunderline->setChecked((tagTypeVector->at(index)).underline);
    cbBold->setChecked((tagTypeVector->at(index)).bold);
    cbItalic->setChecked((tagTypeVector->at(index)).italic);
}

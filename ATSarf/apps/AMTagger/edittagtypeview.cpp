#include "edittagtypeview.h"
#include "addtagtypeview.h"
#include "atagger.h"
#include "global.h"
#include <QItemEditorFactory>
#include <sstream>
#include <QMessageBox>
#include <QColor>
#include <QDockWidget>

EditTagTypeView::EditTagTypeView(QWidget *parent) :
    QMainWindow(parent)
{

    resize(700,500);

    /** Create Menu **/
    createMenus();

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

    lineEditTag = new QLineEdit(this);
    lineEditTag->setEnabled(false);
    lineEditDescription = new QLineEdit(this);
    lineEditDescription->setEnabled(false);
    //lineEditfgcolor = new QLineEdit(this);
    //lineEditbgcolor = new QLineEdit(this);
    colorfgcolor = new ColorListEditor(this);
    colorfgcolor->setEnabled(false);
    colorbgcolor = new ColorListEditor(this);
    colorbgcolor->setEnabled(false);
    //cbfont = new QFontComboBox(this);
    cbfont = new QComboBox(this);
    cbfont->setEnabled(false);
    for(int i=5; i<20; i++) {
        std::stringstream out;
        out << i;
        std::string str = out.str();
        cbfont->addItem(QString::fromStdString(str));
    }

    cbunderline = new QCheckBox(this);
    cbunderline->setEnabled(false);
    cbBold = new QCheckBox(this);
    cbBold->setEnabled(false);
    cbItalic = new QCheckBox(this);
    cbItalic->setEnabled(false);

    btnAdd = new QPushButton(this);
    btnAdd->setText("+");
    btnAdd->setFixedSize(20,20);
    connect(btnAdd,SIGNAL(clicked()),this,SLOT(add_clicked()));

    btnRmv = new QPushButton(this);
    btnRmv->setText("-");
    btnRmv->setFixedSize(20,20);
    connect(btnRmv,SIGNAL(clicked()),this,SLOT(rmv_clicked()));

    btnEdit = new QPushButton(this);
    btnEdit->setText("...");
    btnEdit->setFixedSize(40,20);
    connect(btnEdit,SIGNAL(clicked()),this,SLOT(edit_clicked()));

    btnSave = new QPushButton(this);
    btnSave->setText("Save");
    btnSave->setFixedSize(60,30);
    connect(btnSave,SIGNAL(clicked()),this,SLOT(save_clicked()));

    btnLoad = new QPushButton(this);
    btnLoad->setText("Source");
    btnLoad->setFixedSize(60,30);
    connect(btnLoad,SIGNAL(clicked()),this,SLOT(load_clicked()));

    /** Create Dock Windows **/

    QDockWidget *dock = new QDockWidget(tr("Tag Types"), this);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    lvTypes = new QListWidget(dock);
    lvTypes->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(lvTypes, SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(item_clicked()));
    //setCentralWidget(lvTypes);
    dock->setWidget(lvTypes);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());

    dock = new QDockWidget(tr("Tag Add/Rmv/Edit"), this);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    scroll1 = new QScrollArea(dock);
    gButtons1 = new QGridLayout(scroll1);
    gButtons1->addWidget(btnAdd,0,0);
    gButtons1->addWidget(btnRmv,0,1);
    gButtons1->addWidget(btnEdit,0,2);

    dock->setWidget(scroll1);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());

    dock = new QDockWidget(tr("TagType Details"), this);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    scroll3 = new QScrollArea(dock);
    grid=new QGridLayout(scroll3);
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

    dock->setWidget(scroll3);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());

    dock = new QDockWidget(tr("Tag Save/Load"), this);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    scroll2 = new QScrollArea(dock);
    gButtons2 = new QGridLayout(scroll2);
    gButtons2->addWidget(btnSave,0,0);
    gButtons2->addWidget(btnLoad,0,1);

    dock->setWidget(scroll2);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());

    /** Fill Data in View **/

    tagTypeVector = new QVector<TagType>();
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

void EditTagTypeView::createMenus()
{
    viewMenu = menuBar()->addMenu(tr("&View"));

    /*
    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
    */
}

void EditTagTypeView::add_clicked() {
    parentCheck = true;
    AddTagTypeView * attv = new AddTagTypeView(this);
    attv->show();
}

void EditTagTypeView::rmv_clicked() {

    int index = lvTypes->currentRow();
    if(index != -1) {
        tagTypeVector->remove(index);
    }
    update_TagTypes();
}

void EditTagTypeView::edit_clicked() {
    if(!edit) {
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
        (*tagTypeVector)[index].tag = tag;
        (*tagTypeVector)[index].description = lineEditDescription->text();
        (*tagTypeVector)[index].fgcolor = colorfgcolor->color().name();
        (*tagTypeVector)[index].bgcolor = colorbgcolor->color().name();
        (*tagTypeVector)[index].font = cbfont->currentText().toInt();
        (*tagTypeVector)[index].underline = cbunderline->isChecked();
        (*tagTypeVector)[index].bold = cbBold->isChecked();
        (*tagTypeVector)[index].italic = cbItalic->isChecked();
    }
}

void EditTagTypeView::save_clicked() {
    *(_atagger->tagTypeVector) = *(tagTypeVector);
    ((AMTMainWindow*)parentWidget())->applyTags();
    ((AMTMainWindow*)parentWidget())->save();
}

void EditTagTypeView::load_clicked() {

    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Open Tag Types"), "",
             tr("Tag Types (*.tagtypes);;All Files (*)"));

         if (fileName.isEmpty())
             return;
         else {
             QFile file(fileName);
             if (!file.open(QIODevice::ReadOnly)) {
                 QMessageBox::information(this, tr("Unable to open file"),file.errorString());
                 return;
             }

             QByteArray Tags = file.readAll();
             file.close();
             ((AMTMainWindow*)parentWidget())->process(Tags);
             ((AMTMainWindow*)parentWidget())->finishTaggingText();

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

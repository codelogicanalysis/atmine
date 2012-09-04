#include "customsttview.h"
#include "sstream"
#include "logger.h"
#include "global.h"
#include <QGridLayout>
#include <QScrollArea>
#include <QMessageBox>
#include <QFileDialog>

CustomSTTView::CustomSTTView(QWidget *parent) :
    QMainWindow(parent)
{
    QGridLayout *grid = new QGridLayout();

    btnPOS = new QRadioButton(tr("POS"), this);
    btnGloss = new QRadioButton(tr("Gloss"), this);
    btnStem = new QRadioButton(tr("Stem"), this);
    btnPrefix = new QRadioButton(tr("Prefix"), this);
    btnSuffix = new QRadioButton(tr("Suffix"), this);

    btnStem->setChecked(true);
    field = "Stem";
    /*
    btnPOS = new QPushButton(tr("POS"), this);
    btnGloss = new QPushButton(tr("Gloss"), this);
    btnStem = new QPushButton(tr("Stem"), this);
    btnPrefix = new QPushButton(tr("Prefix"), this);
    btnSuffix = new QPushButton(tr("Suffix"), this);
    */
    btnTagTypes = new QPushButton(tr("TagTypes"), this);
    btnSelectAll = new QPushButton(tr("Select All"), this);
    btnUnselectAll = new QPushButton(tr("Unselect All"), this);
    btnSelect = new QPushButton(tr(">"), this);
    btnUnselect = new QPushButton(tr("<"), this);
    btnAdd = new QPushButton(tr("Add"), this);
    btnLoad = new QPushButton(tr("Load"), this);
    btnSave = new QPushButton(tr("Save"), this);

    connect(btnPOS,SIGNAL(clicked()),this,SLOT(btnPOS_clicked()));
    connect(btnGloss,SIGNAL(clicked()),this,SLOT(btnGloss_clicked()));
    connect(btnStem,SIGNAL(clicked()),this,SLOT(btnStem_clicked()));
    connect(btnPrefix,SIGNAL(clicked()),this,SLOT(btnPrefix_clicked()));
    connect(btnSuffix,SIGNAL(clicked()),this,SLOT(btnSuffix_clicked()));
    connect(btnTagTypes,SIGNAL(clicked()),this,SLOT(btnTagTypes_clicked()));
    connect(btnSelectAll,SIGNAL(clicked()),this,SLOT(btnSelectAll_clicked()));
    connect(btnUnselectAll,SIGNAL(clicked()),this,SLOT(btnUnselectAll_clicked()));
    connect(btnSelect,SIGNAL(clicked()),this,SLOT(btnSelect_clicked()));
    connect(btnUnselect,SIGNAL(clicked()),this,SLOT(btnUnselect_clicked()));
    connect(btnAdd,SIGNAL(clicked()),this,SLOT(btnAdd_clicked()));
    connect(btnLoad,SIGNAL(clicked()),this,SLOT(btnLoad_clicked()));
    connect(btnSave, SIGNAL(clicked()), this, SLOT(btnSave_clicked()));

    grid->addWidget(btnStem,0,0);
    grid->addWidget(btnPrefix,0,1);
    grid->addWidget(btnSuffix,0,2);
    grid->addWidget(btnGloss,0,3);
    grid->addWidget(btnPOS,0,4);
    grid->addWidget(btnTagTypes,0,5);
    grid->addWidget(btnSave,0,6);
    grid->addWidget(btnSelectAll,7,0);
    grid->addWidget(btnUnselectAll,7,1);
    grid->addWidget(btnSelect,4,2);
    grid->addWidget(btnUnselect,5,2);
    grid->addWidget(btnAdd,10,5);
    grid->addWidget(btnLoad,10,6);

    lblPattern = new QLabel(tr("Pattern"),this);
    lblTagName = new QLabel(tr("TagName"), this);
    lblDescription = new QLabel(tr("Description"), this);
    lblFGColor = new QLabel(tr("Foregroud Color"), this);
    lblBGColor = new QLabel(tr("Background Color"), this);
    lblFont = new QLabel(tr("Font"), this);
    lblBold = new QLabel(tr("Bold"), this);
    lblItalic = new QLabel(tr("Italic"), this);
    lblUnderline = new QLabel(tr("Underline"), this);

    grid->addWidget(lblPattern,1,0);
    grid->addWidget(lblTagName,1,3);
    grid->addWidget(lblDescription,1,5);//7,3
    grid->addWidget(lblFGColor,2,5);
    grid->addWidget(lblBGColor,3,5);
    grid->addWidget(lblFont,4,5);
    grid->addWidget(lblBold,5,5);
    grid->addWidget(lblItalic,6,5);
    grid->addWidget(lblUnderline,7,5);

    editPattern = new QLineEdit(this);
    connect(editPattern,SIGNAL(textChanged(QString)),this,SLOT(editPattern_changed(QString)));
    editTagName = new QLineEdit(this);
    editDescription = new QLineEdit(this);

    grid->addWidget(editPattern,1,1);
    grid->addWidget(editTagName,1,4);
    grid->addWidget(editDescription,1,6);

    colorfgcolor = new ColorListEditor(this);
    colorbgcolor = new ColorListEditor(this);

    grid->addWidget(colorfgcolor,2,6);
    grid->addWidget(colorbgcolor,3,6);

    cbfont = new QComboBox(this);
    for(int i=5; i<20; i++) {
        std::stringstream out;
        out << i;
        std::string str = out.str();
        cbfont->addItem(QString::fromStdString(str));
    }

    grid->addWidget(cbfont,4,6);

    cbunderline = new QCheckBox(this);
    cbBold = new QCheckBox(this);
    cbItalic = new QCheckBox(this);

    grid->addWidget(cbBold,5,6);
    grid->addWidget(cbItalic,6,6);
    grid->addWidget(cbunderline,7,6);

    listPossibleTags = new QListWidget(this);
    listPossibleTags->setSelectionMode(QAbstractItemView::MultiSelection);
    listSelectedTags = new QListWidget(this);
    listSelectedTags->setSelectionMode(QAbstractItemView::MultiSelection);

    grid->addWidget(listPossibleTags,2,0,12,2);
    grid->addWidget(listSelectedTags,2,3,12,2);

    QWidget *widget = new QWidget(this);

    widget->setLayout(grid);
    setCentralWidget(widget);
    setWindowTitle(tr("Custom Sarf Tag Types"));
    //resize(480, 320);

    /** Do queries and Fill Tables**/
    theSarf->query.exec("SELECT raw_data FROM stem_category");
    while(theSarf->query.next()) {
        if(!(theSarf->query.value(0).toString().isEmpty()))
            listStems << theSarf->query.value(0).toString();
    }

    listPossibleTags->clear();
    listPossibleTags->addItems(listStems);

    theSarf->query.exec("SELECT raw_data FROM prefix_category");
    while(theSarf->query.next()) {
        if(!(theSarf->query.value(0).toString().isEmpty()))
            listPrefix << theSarf->query.value(0).toString();
    }

    theSarf->query.exec("SELECT raw_data FROM suffix_category");
    while(theSarf->query.next()) {
        if(!(theSarf->query.value(0).toString().isEmpty()))
            listSuffix << theSarf->query.value(0).toString();
    }

    theSarf->query.exec("SELECT name FROM description");
    while(theSarf->query.next()) {
        if(!(theSarf->query.value(0).toString().isEmpty()))
            listGloss << theSarf->query.value(0).toString();
    }

    theSarf->query.exec("SELECT POS FROM stem_category");
    while(theSarf->query.next()) {
        if(!(theSarf->query.value(0).toString().isEmpty()))
            listPOS << theSarf->query.value(0).toString();
    }
}

void CustomSTTView::btnGloss_clicked() {
    field = "Gloss";
    listPossibleTags->clear();
    listPossibleTags->addItems(listGloss);
}

void CustomSTTView::btnLoad_clicked() {

}

void CustomSTTView::btnPOS_clicked() {
    field = "POS";
    listPossibleTags->clear();
    listPossibleTags->addItems(listPOS);
}

void CustomSTTView::btnPrefix_clicked() {
    field = "Prefix";
    listPossibleTags->clear();
    listPossibleTags->addItems(listPrefix);
}

void CustomSTTView::btnAdd_clicked() {

    QString tag = editTagName->text();
    QVector < QPair< QString, QString > > tags;
    for(int i=0; i< (listSelectedTags->count()); i++) {
        QStringList items = listSelectedTags->item(i)->text().split(" -> ");
        QPair<QString,QString> pair(items[0],items[1]);
        tags.append(pair);
    }
    QString desc = editDescription->text();
    int id = _atagger->sarfTagTypeVector->count();
    QString fgcolor = colorfgcolor->color().name();
    QString bgcolor = colorbgcolor->color().name();
    int font = cbfont->currentText().toInt();
    bool underline = cbunderline->isChecked();
    bool bold = cbBold->isChecked();
    bool italic = cbItalic->isChecked();

    if(!tag.isEmpty() && !tags.isEmpty() && !desc.isEmpty()) {
        _atagger->insertSarfTagType(tag,tags,desc,id,fgcolor,bgcolor,font,underline,bold,italic);
        QMessageBox::information(this,"TagType Add",tag + " added successfully!");
        editTagName->clear();
        editPattern->clear();
        listSelectedTags->clear();
        editDescription->clear();
    }
    else {
        QMessageBox::warning(this,"Warning","One of the fields is Empty!!");
    }
}

void CustomSTTView::btnSelectAll_clicked() {
    listPossibleTags->selectAll();
}

void CustomSTTView::btnStem_clicked() {
    field = "Stem";
    listPossibleTags->clear();
    listPossibleTags->addItems(listStems);
}

void CustomSTTView::btnSelect_clicked() {
    foreach(QListWidgetItem* item, listPossibleTags->selectedItems()) {
        listSelectedTags->addItem(field + " -> " + item->text());
    }
    listSelectedTags->sortItems(Qt::AscendingOrder);
    listPossibleTags->clearSelection();
}

void CustomSTTView::btnSuffix_clicked() {
    field = "Suffix";
    listPossibleTags->clear();
    listPossibleTags->addItems(listSuffix);
}

void CustomSTTView::btnTagTypes_clicked() {

}

void CustomSTTView::btnUnselectAll_clicked() {
    listPossibleTags->clearSelection();
}

void CustomSTTView::btnUnselect_clicked() {
    qDeleteAll(listSelectedTags->selectedItems());
}

void CustomSTTView::editPattern_changed(QString text) {
    QStringList *list;
    if(btnStem->isChecked()) {
        list = &listStems;
    }
    else if(btnPrefix->isChecked()) {
        list = &listPrefix;
    }
    else if(btnSuffix->isChecked()) {
        list = &listSuffix;
    }
    else if(btnGloss->isChecked()) {
        list = &listGloss;
    }
    else if(btnPOS->isChecked()) {
        list = &listPOS;
    }

    QRegExp regExp(text);
    listPossibleTags->clear();
    if(editPattern->text().isEmpty()) {
        listPossibleTags->addItems(*list);
    }
    else {
        listPossibleTags->addItems(list->filter(regExp));
    }
}

void CustomSTTView::btnSave_clicked() {
    QByteArray sarftagtypeData = _atagger->dataInJsonFormat(sarfTTV);

    QString fileName;
    if(_atagger->sarftagtypeFile.isEmpty()) {
        fileName = QFileDialog::getSaveFileName(
                this,
                tr("Save Sarf Tag Types"), "",
                tr("Text (*.tagtypes);;All Files (*)"));
    }
    else {
        fileName = _atagger->sarftagtypeFile;
    }

    QFile tfile(fileName);
    if(_atagger->sarftagtypeFile.isEmpty() && tfile.exists()) {
        QMessageBox::warning(this,"Warning", "File already exists, please try another name");
        return;
    }
    if (!tfile.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this,"Warning","Can't open tagtypes file to Save");
        return;
    }
    _atagger->sarftagtypeFile = fileName;

    QTextStream outtags(&tfile);
    outtags << sarftagtypeData;
    tfile.close();
}

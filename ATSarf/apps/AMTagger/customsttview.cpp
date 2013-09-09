#include "customsttview.h"
#include "sstream"
#include "logger.h"
#include "global.h"
#include <QGridLayout>
#include <QScrollArea>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <qjson/parser.h>

CustomSTTView::CustomSTTView(QWidget *parent) :
    QMainWindow(parent)
{
    dirty = false;

    QGridLayout *grid = new QGridLayout();
    btnSelectAll = new QPushButton(tr("Select All"), this);
    btnUnselectAll = new QPushButton(tr("Unselect All"), this);
    btnSelect = new QPushButton(tr(">"), this);
    btnUnselect = new QPushButton(tr("<"), this);
    btnAdd = new QPushButton(tr("Add\nType"), this);
    btnLoad = new QPushButton(tr("Load"), this);
    //btnSave = new QPushButton(tr("Save File"), this);
    //btnCancel = new QPushButton(tr("Cancel"), this);
    btnClose = new QPushButton(tr("Close"), this);
    btnRemove = new QPushButton(tr("Remove\nType"), this);

    btnRemove->setEnabled(false);
    //btnSave->setEnabled(false);
    btnSelect->setEnabled(false);
    btnUnselect->setEnabled(false);

    connect(btnSelectAll,SIGNAL(clicked()),this,SLOT(btnSelectAll_clicked()));
    connect(btnUnselectAll,SIGNAL(clicked()),this,SLOT(btnUnselectAll_clicked()));
    connect(btnAdd,SIGNAL(clicked()),this,SLOT(btnAdd_clicked()));
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(btnRemove_clicked()));
    connect(btnLoad,SIGNAL(clicked()),this,SLOT(btnLoad_clicked()));
    //connect(btnSave, SIGNAL(clicked()), this, SLOT(btnSave_clicked()));
    //connect(btnCancel,SIGNAL(clicked()), this, SLOT(btnCancel_clicked()));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(btnClose_clicked()));

    //grid->addWidget(btnSave,14,5);
    grid->addWidget(btnSelect,6,2);
    grid->addWidget(btnUnselect,7,2);
    grid->addWidget(btnAdd,3,2);
    grid->addWidget(btnRemove,4,2);
    grid->addWidget(btnLoad,14,4,1,2);
    grid->addWidget(btnClose,14,6);
    //grid->addWidget(btnCancel,13,5);
    grid->addWidget(btnSelectAll,14,0);
    grid->addWidget(btnUnselectAll,14,1);

    lblPattern = new QLabel(tr("Pattern:"),this);
    lblFeatures = new QLabel(tr("Features:"), this);
    lblTagName = new QLabel(tr("TagName:"), this);
    lblDescription = new QLabel(tr("Description:"), this);
    lblFGColor = new QLabel(tr("Foregroud Color:"), this);
    lblBGColor = new QLabel(tr("Background Color:"), this);
    lblFont = new QLabel(tr("Font:"), this);
    lblStep = new QLabel(tr("Steps") , this);
    //lblBold = new QLabel(tr("Bold:"), this);
    //lblItalic = new QLabel(tr("Italic:"), this);
    //lblUnderline = new QLabel(tr("Underline:"), this);
    //lblSynStep = new QLabel(tr("Synonym within\nSteps"), this);

    grid->addWidget(lblPattern,1,0);
    grid->addWidget(lblFeatures,0,0);
    grid->addWidget(lblTagName,0,3);
    grid->addWidget(lblDescription,1,6);//7,3
    grid->addWidget(lblFGColor,4,6);
    grid->addWidget(lblBGColor,6,6);
    grid->addWidget(lblFont,8,6);
    //grid->addWidget(lblBold,6,7);
    //grid->addWidget(lblItalic,7,7);
    //grid->addWidget(lblUnderline,8,7);

    editPattern = new QLineEdit(this);
    connect(editPattern,SIGNAL(textChanged(QString)),this,SLOT(editPattern_changed(QString)));
    editDescription = new QTextEdit(this);

    grid->addWidget(editPattern,1,1);
    grid->addWidget(editDescription,2,6,2,1);

    /** Syn GroupBox **/

    cbSyn = new QCheckBox(tr("Synonym within"), this);
    sbSynStep = new QSpinBox(this);
    gbSyn = new QGroupBox(this);

    cbSyn->setChecked(false);
    connect(cbSyn,SIGNAL(clicked(bool)),this,SLOT(cbSynEnable_checked(bool)));
    sbSynStep->setMinimum(1);
    sbSynStep->setMaximum(7);
    sbSynStep->setValue(1);
    sbSynStep->setEnabled(false);
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(cbSyn);
    hbox->addWidget(sbSynStep);
    hbox->addWidget(lblStep);
    gbSyn->setLayout(hbox);
    gbSyn->setEnabled(false);

    grid->addWidget(gbSyn,15,0,1,2);

    /** End **/

    /** random color routine **/

    QStringList colorNames = QColor::colorNames();
    int size = colorNames.size();
    double randomNumber = ((double) rand() / (RAND_MAX));
    int index = size * randomNumber;
    QColor initColor = colorNames[index];
    colorfgcolor = new ColorListEditor(this);
    colorfgcolor->setColor(initColor);

    int contrastIndex = 147 - index;
    QColor contrastColor = colorNames[contrastIndex];

    colorbgcolor = new ColorListEditor(this);
    colorbgcolor->setColor(contrastColor);

    /** end of routine **/

    grid->addWidget(colorfgcolor,5,6);
    grid->addWidget(colorbgcolor,7,6);

    cbfont = new QComboBox(this);
    for(int i=5; i<20; i++) {
        std::stringstream out;
        out << i;
        std::string str = out.str();
        cbfont->addItem(QString::fromStdString(str));
    }

    cbTagName = new QComboBox(this);
    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        cbTagName->addItem(_atagger->tagTypeVector->at(i)->tag);
    }

    cbContain = new QComboBox(this);
    cbContain->addItem("isA");
    cbContain->addItem("contains");

    connect(cbTagName, SIGNAL(editTextChanged(QString)), this, SLOT(tagName_Edited(QString)));
    connect(cbTagName, SIGNAL(currentIndexChanged(QString)), this, SLOT(tagName_changed(QString)));

    cbTagType = new QComboBox(this);
    cbTagType->addItem("Prefix");
    cbTagType->addItem("Stem");
    cbTagType->addItem("Suffix");
    cbTagType->addItem("Prefix-POS");
    cbTagType->addItem("Stem-POS");
    cbTagType->addItem("Suffix-POS");
    cbTagType->addItem("Prefix-Gloss");
    cbTagType->addItem("Stem-Gloss");
    cbTagType->addItem("Suffix-Gloss");
    cbTagType->addItem("Category");

    grid->addWidget(cbfont,9,6);
    grid->addWidget(cbTagName,0,4);
    grid->addWidget(cbTagType,0,1);
    grid->addWidget(cbContain,9,2);

    //cbunderline = new QCheckBox(tr("Underline"), this);
    cbBold = new QCheckBox(tr("Bold"), this);
    cbItalic = new QCheckBox(tr("Italic"), this);
    cbCaseSensetive = new QCheckBox(this);
    cbCaseSensetive->setText("Case Sensetive");

    //connect(cbunderline, SIGNAL(clicked(bool)), this, SLOT(underline_clicked(bool)));
    connect(cbBold, SIGNAL(clicked(bool)), this, SLOT(bold_clicked(bool)));
    connect(cbItalic, SIGNAL(clicked(bool)), this, SLOT(italic_clicked(bool)));

    grid->addWidget(cbBold,10,6);
    grid->addWidget(cbItalic,11,6);
    //grid->addWidget(cbunderline,12,6);
    grid->addWidget(cbCaseSensetive,1,2);

    listPossibleTags = new QListWidget(this);
    listPossibleTags->setSelectionMode(QAbstractItemView::MultiSelection);
    listSelectedTags = new QTreeWidget(this);
    listSelectedTags->setColumnCount(4);
    QStringList columnsD;
    columnsD << QString() << "Feature" << QString() << "Value";
    QTreeWidgetItem* itemD=new QTreeWidgetItem(columnsD);
    listSelectedTags->setHeaderItem(itemD);
    listSelectedTags->setSelectionMode(QAbstractItemView::MultiSelection);
    connect(listSelectedTags,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(selectedTags_doubleclicked(QTreeWidgetItem*,int)));

    grid->addWidget(listPossibleTags,2,0,12,2);
    grid->addWidget(listSelectedTags,1,3,13,3);

    QWidget *widget = new QWidget(this);

    widget->setLayout(grid);
    setCentralWidget(widget);
    setWindowTitle(tr("Custom Sarf Tag Types"));
    //resize(480, 320);

    /** Do queries and Fill Tables**/
    theSarf->query.exec("SELECT raw_data FROM prefix_category");
    while(theSarf->query.next()) {
        if(!(theSarf->query.value(0).toString().isEmpty()))
            listPrefix << theSarf->query.value(0).toString();
    }
    listPrefix.removeDuplicates();

    theSarf->query.exec("SELECT id,name FROM category where abstract=1");
    while(theSarf->query.next()) {
        if(!(theSarf->query.value(0).toString().isEmpty())) {
            listCategoryId << theSarf->query.value(0).toString();
            listCategory << theSarf->query.value(1).toString();
        }
    }

    field = "Prefix";
    listPossibleTags->clear();
    listPossibleTags->addItems(listPrefix);

    if(_atagger->tagTypeVector->count() != 0) {
        //cbTagName->setEditable(true);

            const SarfTagType * stt = (SarfTagType*)(_atagger->tagTypeVector->at(0));

            cbTagName->setCurrentIndex(0);
            //cbunderline->setChecked(stt->underline);
            cbBold->setChecked(stt->bold);
            cbItalic->setChecked(stt->italic);
            colorfgcolor->setColor(QColor(stt->fgcolor));
            colorbgcolor->setColor(QColor(stt->bgcolor));
            editDescription->setText(stt->description);
            int index = cbfont->findText(QString::number(stt->font));
            if(index != -1) {
                cbfont->setCurrentIndex(index);
            }

            listSelectedTags->clear();
            if(stt->source == sarf) {
                for(int i=0; i<stt->tags.count(); i++) {
                    QStringList list;
                    if(stt->tags.at(i).first != "Category") {
                        QString four = stt->tags[i].fourth;
                        if(four.contains("Syn")) {
                            four.append(" of");
                        }
                        list << stt->tags.at(i).third <<stt->tags.at(i).first << four <<stt->tags.at(i).second;
                    }
                    else {
                        for(int j=0; j< listCategoryId.count();j++) {
                            if(listCategoryId[j] == stt->tags.at(i).second) {
                                list << stt->tags.at(i).third << stt->tags.at(i).first << stt->tags.at(i).fourth << listCategory[j];
                            }
                        }
                    }
                    listSelectedTags->addTopLevelItem(new QTreeWidgetItem(list));
                }
                btnSelect->setEnabled(true);
                btnUnselect->setEnabled(true);
            }
            else {
                btnSelect->setEnabled(false);
                btnUnselect->setEnabled(false);
            }

            btnRemove->setEnabled(true);
            //btnSave->setEnabled(true);
    }

    connect(colorfgcolor, SIGNAL(currentIndexChanged(QString)), this, SLOT(fgcolor_changed(QString)));
    connect(colorbgcolor, SIGNAL(currentIndexChanged(QString)), this, SLOT(bgcolor_changed(QString)));
    connect(cbTagType,SIGNAL(currentIndexChanged(QString)), this, SLOT(cbTagType_changed(QString)));
    connect(cbfont, SIGNAL(currentIndexChanged(QString)), this, SLOT(font_changed(QString)));
    connect(editDescription, SIGNAL(textChanged()), this, SLOT(desc_edited()));
    connect(btnSelect,SIGNAL(clicked()),this,SLOT(btnSelect_clicked()));
    connect(btnUnselect,SIGNAL(clicked()),this,SLOT(btnUnselect_clicked()));

    //sttVector = new QVector<TagType*>(*(_atagger->tagTypeVector));
    sttVector = new QVector<TagType*>();

    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        TagType* tt = _atagger->tagTypeVector->at(i);
        QString tag = tt->tag;
        QString desc = tt->description;
        QString fgcolor = tt->fgcolor;
        QString bgcolor = tt->bgcolor;
        int font = tt->font;
        //bool underline = tt->underline;
        bool underline = false;
        bool italic = tt->italic;
        bool bold = tt->bold;
        int id = tt->id;
        Source source = tt->source;

        if(_atagger->tagTypeVector->at(i)->source == sarf) {
            SarfTagType* _stt = (SarfTagType*)(_atagger->tagTypeVector->at(i));
            QVector< Quadruple< QString , QString , QString , QString > > tags = QVector< Quadruple< QString , QString , QString , QString > >(_stt->tags);
            SarfTagType* stt = new SarfTagType(tag,tags,desc,id,fgcolor,bgcolor,font,underline,bold,italic,source);
            sttVector->append(stt);

        }
        else {
            TagType* stt = new TagType(tag,desc,id,fgcolor,bgcolor,font,underline,bold,italic,source);
            sttVector->append(stt);
        }
    }
}

void CustomSTTView::selectedTags_doubleclicked(QTreeWidgetItem *item, int i) {
    if((item == NULL) || (i != 0)) {
        return;
    }
    SarfTagType * stt;
    for(int j=0; j<sttVector->count(); j++) {
        if(sttVector->at(j)->tag == cbTagName->currentText()) {
            stt = (SarfTagType*)((*(sttVector))[j]);
            break;
        }
    }

    QString text = item->text(0);
    if(text.compare("NOT") == 0) {
        item->setText(0,QString());
        for(int j=0; j < stt->tags.count(); j++) {
            if(stt->tags.at(j).first.compare("Category") == 0) {
                int index = -1;
                QString value;
                for(int k=0; k<listCategory.size(); k++) {
                    if(listCategory[k] == item->text(3)) {
                        index = k;
                        break;
                    }
                }
                if(index != -1) {
                    value = listCategoryId[index];
                }
                if(stt->tags.at(j).second.compare(value) == 0) {
                    stt->tags[j].third = "";
                }
            }
            else if((stt->tags.at(j).first == item->text(1)) && (stt->tags.at(j).second == item->text(3)) ) {
                stt->tags[j].third = "";
            }
        }
    }
    else {
        item->setText(0,"NOT");
        QFont serifFont("Times", 8, QFont::Bold);
        item->setFont(0,serifFont);
        for(int j=0; j < stt->tags.count(); j++) {
            if(stt->tags.at(j).first.compare("Category") == 0) {
                int index = -1;
                QString value;
                for(int k=0; k<listCategory.size(); k++) {
                    if(listCategory[k] == item->text(3)) {
                        index = k;
                        break;
                    }
                }
                if(index != -1) {
                    value = listCategoryId[index];
                }
                if(stt->tags.at(j).second.compare(value) == 0) {
                    stt->tags[j].third = "NOT";
                }
            }
            else if((stt->tags.at(j).first == item->text(1)) && (stt->tags.at(j).second == item->text(3)) ) {
                stt->tags[j].third = "NOT";
            }
        }
    }
    listSelectedTags->clearSelection();
    dirty = true;
}

void CustomSTTView::cbSynEnable_checked(bool isChecked) {
    if(isChecked) {
        sbSynStep->setEnabled(true);
        cbContain->setEnabled(false);
    }
    else {
        sbSynStep->setEnabled(false);
        cbContain->setEnabled(true);
    }
}

void CustomSTTView::cbTagType_changed(QString text) {
    field = text;
    listPossibleTags->clear();
    editPattern->clear();
    gbSyn->setEnabled(false);
    cbSyn->setChecked(false);
    cbContain->setCurrentIndex(cbContain->findText("isA"));
    cbContain->setEnabled(false);
    if(field == "Prefix") {
        listPossibleTags->addItems(listPrefix);
    }
    else if(field == "Prefix-POS") {
        if(listPrefixPOS.isEmpty()) {

            theSarf->query.exec("SELECT POS FROM prefix_category");
            while(theSarf->query.next()) {
                if(!(theSarf->query.value(0).toString().isEmpty())) {
                    listPrefixPOS << theSarf->query.value(0).toString().split('/').at(1);
                }
            }
            listPrefixPOS.removeDuplicates();
        }
        listPossibleTags->addItems(listPrefixPOS);
    }
    else if(field == "Stem") {
        if(listStems.isEmpty()) {

            theSarf->query.exec("SELECT raw_data FROM stem_category");
            while(theSarf->query.next()) {
                if(!(theSarf->query.value(0).toString().isEmpty()))
                    listStems << theSarf->query.value(0).toString();
            }
            listStems.removeDuplicates();
        }
        listPossibleTags->addItems(listStems);
        cbContain->setEnabled(true);
    }
    else if(field == "Stem-POS") {
        if(listStemPOS.isEmpty()) {

            theSarf->query.exec("SELECT POS FROM stem_category");
            while(theSarf->query.next()) {
                if(!(theSarf->query.value(0).toString().isEmpty())) {
                    listStemPOS << theSarf->query.value(0).toString().split('/').at(1);
                }
            }
            listStemPOS.removeDuplicates();
        }
        listPossibleTags->addItems(listStemPOS);
    }
    else if(field == "Suffix") {
        if(listSuffixPOS.isEmpty()) {

            theSarf->query.exec("SELECT raw_data FROM suffix_category");
            while(theSarf->query.next()) {
                if(!(theSarf->query.value(0).toString().isEmpty()))
                    listSuffix << theSarf->query.value(0).toString();
            }
            listSuffix.removeDuplicates();
        }
        listPossibleTags->addItems(listSuffix);
    }
    else if(field == "Suffix-POS") {
        if(listSuffixPOS.isEmpty()) {

            theSarf->query.exec("SELECT POS FROM suffix_category");
            while(theSarf->query.next()) {
                if(!(theSarf->query.value(0).toString().isEmpty())) {
                    listSuffixPOS << theSarf->query.value(0).toString().split('/').at(1);
                    listSuffixPOS.removeDuplicates();
                }
            }
        }
        listPossibleTags->addItems(listSuffixPOS);
    }
    else if(field == "Prefix-Gloss") {
        if(listPrefixGloss.isEmpty()) {

            theSarf->query.exec("SELECT DISTINCT d.name FROM description d, prefix_category p where p.description_id = d.id");
            while(theSarf->query.next()) {
                if(!(theSarf->query.value(0).toString().isEmpty()))
                    listPrefixGloss << theSarf->query.value(0).toString();
            }
        }
        listPossibleTags->addItems(listPrefixGloss);
        gbSyn->setEnabled(true);
        cbContain->setEnabled(true);
    }
    else if(field == "Stem-Gloss") {
        if(listStemGloss.isEmpty()) {

            theSarf->query.exec("SELECT DISTINCT d.name FROM description d, stem_category s where s.description_id = d.id");
            while(theSarf->query.next()) {
                if(!(theSarf->query.value(0).toString().isEmpty()))
                    listStemGloss << theSarf->query.value(0).toString();
            }
        }
        listPossibleTags->addItems(listStemGloss);
        gbSyn->setEnabled(true);
        cbContain->setEnabled(true);
    }
    else if(field == "Suffix-Gloss") {
        if(listSuffixGloss.isEmpty()) {

            theSarf->query.exec("SELECT DISTINCT d.name FROM description d, suffix_category s where s.description_id = d.id");
            while(theSarf->query.next()) {
                if(!(theSarf->query.value(0).toString().isEmpty()))
                    listSuffixGloss << theSarf->query.value(0).toString();
            }
        }
        listPossibleTags->addItems(listSuffixGloss);
        gbSyn->setEnabled(true);
        cbContain->setEnabled(true);
    }
    else if(field == "Category") {
        listPossibleTags->addItems(listCategory);
    }
}

void CustomSTTView::disconnect_Signals() {
    //disconnect(cbunderline, SIGNAL(clicked(bool)), this, SLOT(underline_clicked(bool)));
    disconnect(cbBold, SIGNAL(clicked(bool)), this, SLOT(bold_clicked(bool)));
    disconnect(cbItalic, SIGNAL(clicked(bool)), this, SLOT(italic_clicked(bool)));
    disconnect(cbTagName, SIGNAL(editTextChanged(QString)), this, SLOT(tagName_Edited(QString)));
    disconnect(cbTagName, SIGNAL(currentIndexChanged(QString)), this, SLOT(tagName_changed(QString)));
    disconnect(editPattern,SIGNAL(textChanged(QString)),this,SLOT(editPattern_changed(QString)));
    disconnect(colorfgcolor, SIGNAL(currentIndexChanged(QString)), this, SLOT(fgcolor_changed(QString)));
    disconnect(colorbgcolor, SIGNAL(currentIndexChanged(QString)), this, SLOT(bgcolor_changed(QString)));
    disconnect(cbTagType,SIGNAL(currentIndexChanged(QString)), this, SLOT(cbTagType_changed(QString)));
    disconnect(cbfont, SIGNAL(currentIndexChanged(QString)), this, SLOT(font_changed(QString)));
    disconnect(editDescription, SIGNAL(textChanged()), this, SLOT(desc_edited()));
}

void CustomSTTView::connect_Signals() {
    //connect(cbunderline, SIGNAL(clicked(bool)), this, SLOT(underline_clicked(bool)));
    connect(cbBold, SIGNAL(clicked(bool)), this, SLOT(bold_clicked(bool)));
    connect(cbItalic, SIGNAL(clicked(bool)), this, SLOT(italic_clicked(bool)));
    connect(cbTagName, SIGNAL(editTextChanged(QString)), this, SLOT(tagName_Edited(QString)));
    connect(cbTagName, SIGNAL(currentIndexChanged(QString)), this, SLOT(tagName_changed(QString)));
    connect(editPattern,SIGNAL(textChanged(QString)),this,SLOT(editPattern_changed(QString)));
    connect(colorfgcolor, SIGNAL(currentIndexChanged(QString)), this, SLOT(fgcolor_changed(QString)));
    connect(colorbgcolor, SIGNAL(currentIndexChanged(QString)), this, SLOT(bgcolor_changed(QString)));
    connect(cbTagType,SIGNAL(currentIndexChanged(QString)), this, SLOT(cbTagType_changed(QString)));
    connect(cbfont, SIGNAL(currentIndexChanged(QString)), this, SLOT(font_changed(QString)));
    connect(editDescription, SIGNAL(textChanged()), this, SLOT(desc_edited()));
}

void CustomSTTView::btnAdd_clicked() {

    bool ok;
    QString tagName = QInputDialog::getText(this, "Tag Name", "Please insert a Tag Type Name", QLineEdit::Normal, QString(), &ok);
    if(!ok) {
        return;
    }
    if(tagName.isEmpty()) {
        QMessageBox::warning(this,"Warning","Empty TagType Name!");
        return;
    }
    if(tagName.compare("NONE")==0) {
        QMessageBox::warning(this,"Warning","Invalid TagType Name!");
        return;
    }

    for(int i=0; i<sttVector->count(); i++) {
        const SarfTagType * stt = (SarfTagType*)(sttVector->at(i));
        if(stt->tag == tagName) {
            QMessageBox::warning(this, "Warning", "This Tag Name already exists!");
            return;
        }
    }
    disconnect_Signals();

    dirty = true;

    btnRemove->setEnabled(true);
    //btnSave->setEnabled(true);

    if(!btnSelect->isEnabled()) {
        btnSelect->setEnabled(true);
        btnUnselect->setEnabled(true);
        btnRemove->setEnabled(true);
    }

    listSelectedTags->clear();
    editDescription->clear();
    cbTagName->addItem(tagName);
    cbTagName->setCurrentIndex(cbTagName->findText(tagName));
    cbfont->setCurrentIndex(cbfont->findText("12"));
    //cbunderline->setChecked(false);
    cbBold->setChecked(false);
    cbItalic->setChecked(false);
    //cbTagName->setEditable(true);
    //editDescription->setText(tagDescription);

    /** Random Color Routine **/

    QStringList colorNames = QColor::colorNames();
    int size = colorNames.size();
    double randomNumber = ((double) rand() / (RAND_MAX));
    int index = size * randomNumber;
    QColor initColor = colorNames[index];
    colorfgcolor->setColor(initColor);

    int contrastIndex = 147 - index;
    QColor contrastColor = colorNames[contrastIndex];

    colorbgcolor->setColor(contrastColor);

    /** routine End **/

    QVector < Quadruple< QString , QString , QString , QString > > tags;
    int id = sttVector->count();
    QString fgcolor = colorfgcolor->color().name();
    QString bgcolor = colorbgcolor->color().name();
    int font = cbfont->currentText().toInt();
    //bool underline = cbunderline->isChecked();
    bool underline = false;
    bool bold = cbBold->isChecked();
    bool italic = cbItalic->isChecked();
    SarfTagType* sarftagtype = new SarfTagType(tagName,tags,QString(),id,fgcolor,bgcolor,font,underline,bold,italic,sarf);
    sttVector->append(sarftagtype);

    connect_Signals();
}

void CustomSTTView::btnSelectAll_clicked() {
    listPossibleTags->selectAll();
}

void CustomSTTView::btnSelect_clicked() {
    if(sttVector->count() == 0) {
        return;
    }

    dirty = true;

    SarfTagType * stt;
    for(int i=0; i<sttVector->count(); i++) {
        if(sttVector->at(i)->tag == cbTagName->currentText()) {
            stt = (SarfTagType*)((*(sttVector))[i]);
            break;
        }
    }

    foreach(QListWidgetItem* item, listPossibleTags->selectedItems()) {

        QStringList list;
        QString relation = "isA";
        list << "  " << field;
        if((field.contains("Gloss") == 0) || (field.compare("Stem") == 0)) {
            if(cbContain->isEnabled() && (cbContain->currentText().compare("contains") == 0)) {
                list << "contains" << item->text();
                relation = "contains";
            }
            else if(gbSyn->isEnabled() && cbSyn->isChecked()) {
                QString synStep = "Syn ";
                synStep.append(sbSynStep->text());
                synStep.append(" of");
                list << synStep << item->text();
                relation = "Syn";
                relation.append(sbSynStep->text());
            }
            else {
                list << relation << item->text();
            }
        }
        else {
            list << relation << item->text();
        }
        listSelectedTags->addTopLevelItem(new QTreeWidgetItem(list));
        QString value = item->text();
        if(field == "Category") {
            int index = -1;//listCategory.indexOf('^' + value + '$');
            for(int i=0; i<listCategory.size(); i++) {
                if(listCategory[i] == value) {
                    index = i;
                    break;
                }
            }
            if(index != -1) {
                value = listCategoryId[index];
            }
        }
        Quadruple< QString , QString , QString , QString > pair(field, value, QString() , relation);
        stt->tags.append(pair);
    }
    listSelectedTags->sortItems(0,Qt::AscendingOrder);
    listPossibleTags->clearSelection();
}

void CustomSTTView::btnUnselectAll_clicked() {
    listPossibleTags->clearSelection();
}

void CustomSTTView::btnUnselect_clicked() {
    dirty = true;

    qDeleteAll(listSelectedTags->selectedItems());

    SarfTagType * stt;
    for(int i=0; i<sttVector->count(); i++) {
        if(sttVector->at(i)->tag == cbTagName->currentText()) {
            stt = (SarfTagType*)((*(sttVector))[i]);
            break;
        }
    }

    stt->tags.clear();

    for(int i=0; i<listSelectedTags->topLevelItemCount(); i++) {
        QString data1 = listSelectedTags->topLevelItem(i)->text(0);
        QString data2 = listSelectedTags->topLevelItem(i)->text(1);
        QString data3 = listSelectedTags->topLevelItem(i)->text(2);
        QString data4 = listSelectedTags->topLevelItem(i)->text(3);
        Quadruple< QString , QString , QString , QString > quad(data2, data4, data1, data3);
        stt->tags.append(quad);
    }
}

void CustomSTTView::editPattern_changed(QString text) {
    QStringList *list;
    if(cbTagType->currentText() == "Stem") {
        list = &listStems;
    }
    else if(cbTagType->currentText() == "Prefix") {
        list = &listPrefix;
    }
    else if(cbTagType->currentText() == "Suffix") {
        list = &listSuffix;
    }
    else if(cbTagType->currentText() == "Prefix-Gloss") {
        list = &listPrefixGloss;
    }
    else if(cbTagType->currentText() == "Stem-Gloss") {
        list = &listStemGloss;
    }
    else if(cbTagType->currentText() == "Suffix-Gloss") {
        list = &listSuffixGloss;
    }
    else if(cbTagType->currentText() == "Prefix-POS") {
        list = &listPrefixPOS;
    }
    else if(cbTagType->currentText() == "Stem-POS") {
        list = &listStemPOS;
    }
    else if(cbTagType->currentText() == "Suffix-POS") {
        list = &listSuffixPOS;
    }
    else if(cbTagType->currentText() == "Category") {
        list = &listCategory;
    }

    Qt::CaseSensitivity sensetive;

    if(cbCaseSensetive->isChecked()) {
        sensetive = Qt::CaseSensitive;
    }
    else {
        sensetive = Qt::CaseInsensitive;
    }

    QRegExp regExp(text, sensetive);

    listPossibleTags->clear();
    if(editPattern->text().isEmpty()) {
        listPossibleTags->addItems(*list);
    }
    else {
        listPossibleTags->addItems(list->filter(regExp));
    }
}

void CustomSTTView::btnLoad_clicked() {

    QStringList dirList = _atagger->tagFile.split('/');
    dirList.removeLast();
    QString dir = dirList.join("/");
    dir.append('/');

    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Open Sarf Tag Types"), dir,
             tr("Sarf Tag Types (*.stt.json);;All Files (*)"));

    if (fileName.isEmpty()) {
        return;
    }
    else {
        QFile file(fileName);
         if (!file.open(QIODevice::ReadOnly)) {
             QMessageBox::information(this, tr("Unable to open file"),file.errorString());
             return;
         }

         disconnect_Signals();
         sttVector->clear();

         QByteArray sarfTT = file.readAll();
         file.close();

         tagtypePath = QDir(dir).relativeFilePath(fileName);

         QJson::Parser parser;
         bool ok;

         QVariantMap result = parser.parse(sarfTT,&ok).toMap();
         foreach(QVariant type, result["TagTypeSet"].toList()) {
             QVariantMap typeElements = type.toMap();

             QString tag = typeElements["Tag"].toString();
             QString desc = typeElements["Description"].toString();
             int id = typeElements["Legend"].toInt();
             QString foreground_color = typeElements["foreground_color"].toString();
             QString background_color = typeElements["background_color"].toString();
             int font = typeElements["font"].toInt();
             //bool underline = typeElements["underline"].toBool();
             bool underline = false;
             bool bold = typeElements["bold"].toBool();
             bool italic = typeElements["italic"].toBool();

             QVector < Quadruple< QString , QString , QString , QString > > tags;
             foreach(QVariant sarfTags, typeElements["Features"].toList()) {
                 QVariantMap st = sarfTags.toMap();
                 Quadruple< QString , QString , QString , QString > quad;
                 if(!(st.value("Prefix").isNull())) {
                     quad.first = "Prefix";
                     quad.second = st.value("Prefix").toString();
                     if(!(st.value("Negation").isNull())) {
                         quad.third = st.value("Negation").toString();
                     }
                     if(!(st.value("Relation").isNull())) {
                         quad.fourth = st.value("Relation").toString();
                     }
                     tags.append(quad);
                 }
                 else if(!(st.value("Stem").isNull())) {
                     quad.first = "Stem";
                     quad.second = st.value("Stem").toString();
                     if(!(st.value("Negation").isNull())) {
                         quad.third = st.value("Negation").toString();
                     }
                     if(!(st.value("Relation").isNull())) {
                         quad.fourth = st.value("Relation").toString();
                     }
                     tags.append(quad);
                 }
                 else if(!(st.value("Suffix").isNull())) {
                     quad.first = "Suffix";
                     quad.second = st.value("Suffix").toString();
                     if(!(st.value("Negation").isNull())) {
                         quad.third = st.value("Negation").toString();
                     }
                     if(!(st.value("Relation").isNull())) {
                         quad.fourth = st.value("Relation").toString();
                     }
                     tags.append(quad);
                 }
                 else if(!(st.value("Prefix-POS").isNull())) {
                     quad.first = "Prefix-POS";
                     quad.second = st.value("Prefix-POS").toString();
                     if(!(st.value("Negation").isNull())) {
                         quad.third = st.value("Negation").toString();
                     }
                     if(!(st.value("Relation").isNull())) {
                         quad.fourth = st.value("Relation").toString();
                     }
                     tags.append(quad);
                 }
                 else if(!(st.value("Stem-POS").isNull())) {
                     quad.first = "Stem-POS";
                     quad.second = st.value("Stem-POS").toString();
                     if(!(st.value("Negation").isNull())) {
                         quad.third = st.value("Negation").toString();
                     }
                     if(!(st.value("Relation").isNull())) {
                         quad.fourth = st.value("Relation").toString();
                     }
                     tags.append(quad);
                 }
                 else if(!(st.value("Suffix-POS").isNull())) {
                     quad.first = "Suffix-POS";
                     quad.second = st.value("Suffix-POS").toString();
                     if(!(st.value("Negation").isNull())) {
                         quad.third = st.value("Negation").toString();
                     }
                     if(!(st.value("Relation").isNull())) {
                         quad.fourth = st.value("Relation").toString();
                     }
                     tags.append(quad);
                 }
                 else if(!(st.value("Prefix-Gloss").isNull())) {
                     quad.first = "Prefix-Gloss";
                     quad.second = st.value("Prefix-Gloss").toString();
                     if(!(st.value("Negation").isNull())) {
                         quad.third = st.value("Negation").toString();
                     }
                     if(!(st.value("Relation").isNull())) {
                         quad.third = st.value("Relation").toString();
                     }
                     tags.append(quad);
                 }
                 else if(!(st.value("Stem-Gloss").isNull())) {
                     quad.first = "Stem-Gloss";
                     quad.second = st.value("Stem-Gloss").toString();
                     if(!(st.value("Negation").isNull())) {
                         quad.third = st.value("Negation").toString();
                     }
                     if(!(st.value("Relation").isNull())) {
                         quad.third = st.value("Relation").toString();
                     }
                     tags.append(quad);
                 }
                 else if(!(st.value("Suffix-Gloss").isNull())) {
                     quad.first = "Suffix-Gloss";
                     quad.second = st.value("Suffix-Gloss").toString();
                     if(!(st.value("Negation").isNull())) {
                         quad.third = st.value("Negation").toString();
                     }
                     if(!(st.value("Relation").isNull())) {
                         quad.third = st.value("Relation").toString();
                     }
                     tags.append(quad);
                 }
                 else if(!(st.value("Category").isNull())) {
                     quad.first = "Category";
                     quad.second = st.value("Category").toString();
                     if(!(st.value("Negation").isNull())) {
                         quad.third = st.value("Negation").toString();
                     }
                     if(!(st.value("Relation").isNull())) {
                         quad.fourth = st.value("Relation").toString();
                     }
                     tags.append(quad);
                 }
             }

             SarfTagType* stt = new SarfTagType(tag,tags,desc,id,foreground_color,background_color,font,underline,bold,italic,sarf);
             sttVector->append(stt);
         }

         cbTagName->clear();
         listSelectedTags->clear();

         for(int i=0; i<sttVector->count(); i++) {
             const SarfTagType * stt = (SarfTagType*)(sttVector->at(i));
             cbTagName->addItem(stt->tag);
         }

         const SarfTagType * stt = (SarfTagType*)(sttVector->at(0));

         cbTagName->setCurrentIndex(0);
         //cbunderline->setChecked(stt->underline);
         cbBold->setChecked(stt->bold);
         cbItalic->setChecked(stt->italic);
         colorfgcolor->setColor(QColor(stt->fgcolor));
         colorbgcolor->setColor(QColor(stt->bgcolor));
         editDescription->setText(stt->description);
         int index = cbfont->findText(QString::number(stt->font));
         if(index != -1) {
             cbfont->setCurrentIndex(index);
         }

         listSelectedTags->clear();
         if(stt->source == sarf) {
             for(int i=0; i<stt->tags.count(); i++) {
                 QStringList list;
                 if(stt->tags.at(i).first != "Category") {
                     QString four = stt->tags[i].fourth;
                     if(four.contains("Syn")) {
                         four.append(" of");
                     }
                     list << stt->tags.at(i).third << stt->tags.at(i).first << four << stt->tags.at(i).second;
                 }
                 else {
                     for(int j=0; j< listCategoryId.count();j++) {
                         if(listCategoryId[j] == stt->tags.at(i).second) {
                             list << stt->tags.at(i).third << stt->tags.at(i).first << stt->tags.at(i).fourth << listCategory[j];
                         }
                     }
                 }
                 listSelectedTags->addTopLevelItem(new QTreeWidgetItem(list));
             }
             btnSelect->setEnabled(true);
             btnUnselect->setEnabled(true);
         }
         else {
             btnSelect->setEnabled(false);
             btnUnselect->setEnabled(false);
         }

         btnRemove->setEnabled(true);
         dirty = true;
         connect_Signals();
     }
}

void CustomSTTView::btnSave_clicked() {
    QByteArray sarftagtypeData = _atagger->dataInJsonFormat(sarfTTV);

    QString fileName;
    if(_atagger->tagtypeFile.isEmpty()) {
        fileName = QFileDialog::getSaveFileName(
                this,
                tr("Save Sarf Tag Types"), "",
                tr("Text (*.stt.json);;All Files (*)"));
        if(fileName.isEmpty()) {
            QMessageBox::warning(this, "Warning", "The Sarf Tag Types file wasn't saved");
            return;
        }
        else {
            fileName += ".stt.json";
            _atagger->tagtypeFile = fileName;
        }
    }
    else {
        fileName = _atagger->tagtypeFile;
    }

    QStringList dirList = _atagger->tagFile.split('/');
    dirList.removeLast();
    QString dir = dirList.join("/");
    dir.append('/');

    QString filePath = dir + fileName;
    QFile tfile(filePath);
    if (!tfile.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this,"Warning","Can't open tagtypes file to Save");
        return;
    }
    _atagger->tagtypeFile = fileName;

    QTextStream outtags(&tfile);
    outtags << sarftagtypeData;
    tfile.close();
    dirty = false;
}

void CustomSTTView::fgcolor_changed(QString color) {
    if(color.isNull() || color.isEmpty()) {
        return;
    }
    if(sttVector->count() == 0) {
        return;
    }

    dirty = true;
    QString tag = cbTagName->currentText();
    for(int i=0; i<sttVector->count(); i++) {
        SarfTagType * stt = (SarfTagType*)((*(sttVector))[i]);
        if(stt->tag == tag) {
            stt->fgcolor = color;
        }
    }
}

void CustomSTTView::bgcolor_changed(QString color) {
    if(color.isNull() || color.isEmpty()) {
        return;
    }
    if(sttVector->count() == 0) {
        return;
    }

    dirty = true;
    QString tag = cbTagName->currentText();
    for(int i=0; i<sttVector->count(); i++) {
        SarfTagType * stt = (SarfTagType*)((*(sttVector))[i]);
        if(stt->tag == tag) {
            stt->bgcolor = color;
        }
    }
}

void CustomSTTView::font_changed(QString fontSize) {
    if(fontSize.isNull() || fontSize.isEmpty()) {
        return;
    }
    if(sttVector->count() == 0) {
        return;
    }

    dirty = true;
    QString tag = cbTagName->currentText();
    for(int i=0; i<sttVector->count(); i++) {
        SarfTagType * stt = (SarfTagType*)((*(sttVector))[i]);
        if(stt->tag == tag) {
            stt->font = fontSize.toInt();
        }
    }
}

void CustomSTTView::tagName_changed(QString name) {
    listSelectedTags->clear();
    if(name.isEmpty() || name.isNull()) {
        return;
    }

    disconnect_Signals();
    for(int i=0; i<sttVector->count(); i++) {
        SarfTagType * stt = (SarfTagType*)((*(sttVector))[i]);
        if(stt->tag == name) {
            editDescription->setText(stt->description);
            int index = cbfont->findText(QString::number(stt->font));
            if(index != -1) {
                cbfont->setCurrentIndex(index);
            }
            colorfgcolor->setColor(QColor(stt->fgcolor));
            colorbgcolor->setColor(QColor(stt->bgcolor));
            //cbunderline->setChecked(stt->underline);
            cbBold->setChecked(stt->bold);
            cbItalic->setChecked(stt->italic);
            if(stt->source == sarf) {
                for(int j=0; j<stt->tags.count(); j++) {
                    QStringList list;
                    if(stt->tags.at(j).first != "Category") {
                        list << stt->tags.at(j).third << stt->tags.at(j).first << stt->tags.at(j).fourth << stt->tags.at(j).second;
                    }
                    else {
                        for(int k=0; k< listCategoryId.count();k++) {
                            if(listCategoryId[k] == stt->tags.at(j).second) {
                                list << stt->tags.at(j).third << stt->tags.at(j).first << stt->tags.at(j).fourth << listCategory[k];
                            }
                        }
                    }
                    listSelectedTags->addTopLevelItem(new QTreeWidgetItem(list));
                }
                btnSelect->setEnabled(true);
                btnUnselect->setEnabled(true);
            }
            else {
                btnSelect->setEnabled(false);
                btnUnselect->setEnabled(false);
            }
        }
    }

    connect_Signals();
}

void CustomSTTView::tagName_Edited(QString name) {

}

/*
void CustomSTTView::underline_clicked(bool underline) {
    if(sttVector->count() == 0) {
        return;
    }

    dirty = true;
    QString tag = cbTagName->currentText();
    for(int i=0; i<sttVector->count(); i++) {
        SarfTagType * stt = (SarfTagType*)((*(sttVector))[i]);
        if(stt->tag == tag) {
            stt->underline = underline;
        }
    }
}
*/

void CustomSTTView::bold_clicked(bool bold) {
    if(sttVector->count() == 0) {
        return;
    }

    dirty = true;
    QString tag = cbTagName->currentText();
    for(int i=0; i<sttVector->count(); i++) {
        SarfTagType * stt = (SarfTagType*)((*(sttVector))[i]);
        if(stt->tag == tag) {
            stt->bold = bold;
        }
    }
}

void CustomSTTView::italic_clicked(bool italic) {
    if(sttVector->count() == 0) {
        return;
    }

    dirty = true;
    QString tag = cbTagName->currentText();
    for(int i=0; i<sttVector->count(); i++) {
        SarfTagType * stt = (SarfTagType*)((*(sttVector))[i]);
        if(stt->tag == tag) {
            stt->italic = italic;
        }
    }
}

void CustomSTTView::desc_edited() {
    if(sttVector->count() == 0) {
        return;
    }

    dirty = true;
    QString tag = cbTagName->currentText();
    for(int i=0; i<sttVector->count(); i++) {
        SarfTagType * stt = (SarfTagType*)((*(sttVector))[i]);
        if(stt->tag == tag) {
            stt->description = editDescription->toPlainText();
        }
    }
}

void CustomSTTView::btnRemove_clicked() {
    if(sttVector->count() == 0) {
        return;
    }
    dirty = true;
    QString tagRemoved = cbTagName->currentText();
    for(int i=0; i<sttVector->count(); i++) {
        const SarfTagType * stt = (SarfTagType*)(sttVector->at(i));
        if(stt->tag == tagRemoved) {
            sttVector->remove(i);
            break;
        }
    }
    cbTagName->removeItem(cbTagName->currentIndex());
    if(sttVector->count() == 0) {
        btnSelect->setEnabled(false);
        btnRemove->setEnabled(false);
    }
}

/*
void CustomSTTView::btnCancel_clicked() {
    dirty = false;
    this->close();
}
*/

void CustomSTTView::btnClose_clicked() {
    this->close();
}

void CustomSTTView::closeEvent(QCloseEvent *event) {

    if(dirty) {
        QMessageBox msgBox;
         msgBox.setText("The document has been modified.");
         msgBox.setInformativeText("Do you want to save your changes?");
         msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
         msgBox.setDefaultButton(QMessageBox::Save);
         int ret = msgBox.exec();

         switch (ret) {
         case QMessageBox::Save:
             /*
             _atagger->sarfTagTypeVector->clear();
             for(int i=0; i< sttVector->count(); i++) {
                 _atagger->sarfTagTypeVector->append(sttVector->at(i));
             }
             */
             //_atagger->tagTypeVector = new QVector<TagType*>(*(sttVector));
             _atagger->tagTypeVector->clear();
             for(int i=0; i<sttVector->count(); i++) {
                 const TagType* tt = sttVector->at(i);
                 QString tag = tt->tag;
                 QString desc = tt->description;
                 QString fgcolor = tt->fgcolor;
                 QString bgcolor = tt->bgcolor;
                 int font = tt->font;
                 //bool underline = tt->underline;
                 bool underline = false;
                 bool italic = tt->italic;
                 bool bold = tt->bold;
                 int id = tt->id;
                 Source source = tt->source;

                 if(sttVector->at(i)->source == sarf) {
                     SarfTagType* _stt = (SarfTagType*)(sttVector->at(i));
                     QVector< Quadruple< QString , QString , QString , QString > > tags = QVector< Quadruple< QString , QString , QString , QString > >(_stt->tags);
                     SarfTagType* stt = new SarfTagType(tag,tags,desc,id,fgcolor,bgcolor,font,underline,bold,italic,source);
                     _atagger->tagTypeVector->append(stt);

                 }
                 else {
                     TagType* stt = new TagType(tag,desc,id,fgcolor,bgcolor,font,underline,bold,italic,source);
                     _atagger->tagTypeVector->append(stt);
                 }
             }

             if(!(tagtypePath.isEmpty())) {
                 _atagger->tagtypeFile = tagtypePath;
             }

             btnSave_clicked();
             break;
         case QMessageBox::Discard:
             break;
         default:
             break;
         }
     }
}

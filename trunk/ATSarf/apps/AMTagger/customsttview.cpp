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
    QGridLayout *grid = new QGridLayout();

    btnSelectAll = new QPushButton(tr("Select All"), this);
    btnUnselectAll = new QPushButton(tr("Unselect All"), this);
    btnSelect = new QPushButton(tr(">"), this);
    btnUnselect = new QPushButton(tr("<"), this);
    btnAdd = new QPushButton(tr("Add\nTag"), this);
    btnLoad = new QPushButton(tr("Load"), this);
    btnSave = new QPushButton(tr("Save"), this);

    connect(btnSelectAll,SIGNAL(clicked()),this,SLOT(btnSelectAll_clicked()));
    connect(btnUnselectAll,SIGNAL(clicked()),this,SLOT(btnUnselectAll_clicked()));
    connect(btnSelect,SIGNAL(clicked()),this,SLOT(btnSelect_clicked()));
    connect(btnUnselect,SIGNAL(clicked()),this,SLOT(btnUnselect_clicked()));
    connect(btnAdd,SIGNAL(clicked()),this,SLOT(btnAdd_clicked()));
    connect(btnLoad,SIGNAL(clicked()),this,SLOT(btnLoad_clicked()));
    connect(btnSave, SIGNAL(clicked()), this, SLOT(btnSave_clicked()));

    grid->addWidget(btnSave,10,5);
    grid->addWidget(btnSelect,4,2);
    grid->addWidget(btnUnselect,5,2);
    grid->addWidget(btnAdd,6,2);
    grid->addWidget(btnLoad,10,6);
    grid->addWidget(btnSelectAll,14,0);
    grid->addWidget(btnUnselectAll,14,1);

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
    editDescription = new QLineEdit(this);
    connect(editDescription, SIGNAL(editingFinished()), this, SLOT(desc_edited()));

    grid->addWidget(editPattern,1,1);
    grid->addWidget(editDescription,1,6);

    colorfgcolor = new ColorListEditor(this);
    colorfgcolor->setColor("Red");
    colorbgcolor = new ColorListEditor(this);
    colorbgcolor->setColor("Yellow");

    connect(colorfgcolor, SIGNAL(currentIndexChanged(QString)), this, SLOT(fgcolor_changed(QString)));
    connect(colorbgcolor, SIGNAL(currentIndexChanged(QString)), this, SLOT(bgcolor_changed(QString)));

    grid->addWidget(colorfgcolor,2,6);
    grid->addWidget(colorbgcolor,3,6);

    cbfont = new QComboBox(this);
    for(int i=5; i<20; i++) {
        std::stringstream out;
        out << i;
        std::string str = out.str();
        cbfont->addItem(QString::fromStdString(str));
    }

    connect(cbfont, SIGNAL(currentIndexChanged(QString)), this, SLOT(font_changed(QString)));

    cbTagName = new QComboBox(this);
    for(int i=0; i<_atagger->sarfTagTypeVector->count(); i++) {
        cbTagName->addItem(_atagger->sarfTagTypeVector->at(i).tag);
    }

    connect(cbTagName, SIGNAL(editTextChanged(QString)), this, SLOT(tagName_Edited(QString)));
    connect(cbTagName, SIGNAL(currentIndexChanged(QString)), this, SLOT(tagName_changed(QString)));

    cbTagType = new QComboBox(this);
    cbTagType->addItem("Prefix");
    cbTagType->addItem("Prefix-POS");
    cbTagType->addItem("Stem");
    cbTagType->addItem("Stem-POS");
    cbTagType->addItem("Suffix");
    cbTagType->addItem("Suffix-POS");
    cbTagType->addItem("Gloss");
    connect(cbTagType,SIGNAL(currentIndexChanged(QString)), this, SLOT(cbTagType_changed(QString)));

    grid->addWidget(cbfont,4,6);
    grid->addWidget(cbTagName,1,4);
    grid->addWidget(cbTagType,0,0,1,2);

    cbunderline = new QCheckBox(this);
    cbBold = new QCheckBox(this);
    cbItalic = new QCheckBox(this);

    connect(cbunderline, SIGNAL(clicked(bool)), this, SLOT(underline_clicked(bool)));
    connect(cbBold, SIGNAL(clicked(bool)), this, SLOT(bold_clicked(bool)));
    connect(cbItalic, SIGNAL(clicked(bool)), this, SLOT(italic_clicked(bool)));

    grid->addWidget(cbBold,5,6);
    grid->addWidget(cbItalic,6,6);
    grid->addWidget(cbunderline,7,6);

    listPossibleTags = new QListWidget(this);
    listPossibleTags->setSelectionMode(QAbstractItemView::MultiSelection);
    listSelectedTags = new QTreeWidget(this);
    listSelectedTags->setColumnCount(2);
    QStringList columnsD;
    columnsD << "Feature" << "Value";
    QTreeWidgetItem* itemD=new QTreeWidgetItem(columnsD);
    listSelectedTags->setHeaderItem(itemD);
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

    theSarf->query.exec("SELECT raw_data FROM prefix_category");
    while(theSarf->query.next()) {
        if(!(theSarf->query.value(0).toString().isEmpty()))
            listPrefix << theSarf->query.value(0).toString();
    }

    field = "Prefix";
    listPossibleTags->clear();
    listPossibleTags->addItems(listPrefix);

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
        if(!(theSarf->query.value(0).toString().isEmpty())) {
            listStemPOS << theSarf->query.value(0).toString().split('/').at(1);
            listStemPOS.removeDuplicates();
        }
    }

    theSarf->query.exec("SELECT POS FROM prefix_category");
    while(theSarf->query.next()) {
        if(!(theSarf->query.value(0).toString().isEmpty())) {
            listPrefixPOS << theSarf->query.value(0).toString().split('/').at(1);
            listPrefixPOS.removeDuplicates();
        }
    }

    theSarf->query.exec("SELECT POS FROM suffix_category");
    while(theSarf->query.next()) {
        if(!(theSarf->query.value(0).toString().isEmpty())) {
            listSuffixPOS << theSarf->query.value(0).toString().split('/').at(1);
            listSuffixPOS.removeDuplicates();
        }
    }

    if(_atagger->sarfTagTypeVector->count() != 0) {
        //cbTagName->setEditable(true);
        const SarfTagType * stt = &(_atagger->sarfTagTypeVector->at(0));

        cbTagName->setCurrentIndex(0);
        cbunderline->setChecked(stt->underline);
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
        for(int i=0; i<stt->tags.count(); i++) {
            QStringList list;
            list << stt->tags.at(i).first << stt->tags.at(i).second;
            listSelectedTags->addTopLevelItem(new QTreeWidgetItem(list));
        }
    }
}

void CustomSTTView::cbTagType_changed(QString text) {
    field = text;
    listPossibleTags->clear();
    if(field == "Prefix") {
        listPossibleTags->addItems(listPrefix);
    }
    else if(field == "Prefix-POS") {
        listPossibleTags->addItems(listPrefixPOS);
    }
    else if(field == "Stem") {
        listPossibleTags->addItems(listStems);
    }
    else if(field == "Stem-POS") {
        listPossibleTags->addItems(listStemPOS);
    }
    else if(field == "Suffix") {
        listPossibleTags->addItems(listSuffixPOS);
    }
    else if(field == "Suffix-POS") {
        listPossibleTags->addItems(listSuffixPOS);
    }
    else if(field == "Gloss") {
        listPossibleTags->addItems(listGloss);
    }
}

void CustomSTTView::btnAdd_clicked() {

    QString tagName = QInputDialog::getText(this,"Tag Name","Please insert a Tag Type Name");
    if(tagName.isEmpty()) {
        QMessageBox::warning(this,"Warning","Empty TagType Name!");
        return;
    }

    for(int i=0; i<_atagger->sarfTagTypeVector->count(); i++) {
        const SarfTagType * stt = &(_atagger->sarfTagTypeVector->at(i));
        if(stt->tag == tagName) {
            QMessageBox::warning(this, "Warning", "This Tag Name already exists!");
            return;
        }
    }

    QString tagDescription = QInputDialog::getText(this, "Tag Description", "Please insert a Tag Type Description");
    if(tagDescription.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Empty TagType Description!");
        return;
    }

    listSelectedTags->clear();
    cbTagName->addItem(tagName);
    cbTagName->setCurrentIndex(cbTagName->findText(tagName));
    cbfont->setCurrentIndex(cbfont->findText("12"));
    cbunderline->setChecked(false);
    cbBold->setChecked(false);
    cbItalic->setChecked(false);
    //cbTagName->setEditable(true);
    editDescription->setText(tagDescription);

    QVector < QPair< QString, QString > > tags;
    int id = _atagger->sarfTagTypeVector->count();
    QString fgcolor = colorfgcolor->color().name();
    QString bgcolor = colorbgcolor->color().name();
    int font = cbfont->currentText().toInt();
    bool underline = cbunderline->isChecked();
    bool bold = cbBold->isChecked();
    bool italic = cbItalic->isChecked();
    _atagger->insertSarfTagType(tagName,tags,tagDescription,id,fgcolor,bgcolor,font,underline,bold,italic);
    //cbTagName->setCurrentIndex(cbTagName->findText(tagName));

    /*
    for(int i=0; i< listSelectedTags->topLevelItemCount(); i++) {
        QStringList items = listSelectedTags->item(i)->text().split(" -> ");
        QPair<QString,QString> pair(items[0],items[1]);
        tags.append(pair);
    }

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
    */
}

void CustomSTTView::btnSelectAll_clicked() {
    listPossibleTags->selectAll();
}

void CustomSTTView::btnSelect_clicked() {

    SarfTagType * stt;
    for(int i=0; i<_atagger->sarfTagTypeVector->count(); i++) {
        if(_atagger->sarfTagTypeVector->at(i).tag == cbTagName->currentText()) {
            stt = &((*(_atagger->sarfTagTypeVector))[i]);
            break;
        }
    }

    foreach(QListWidgetItem* item, listPossibleTags->selectedItems()) {

        QStringList list;
        list << field << item->text();
        listSelectedTags->addTopLevelItem(new QTreeWidgetItem(list));
        QPair <QString, QString> pair(field, item->text());
        stt->tags.append(pair);
    }
    listSelectedTags->sortItems(0,Qt::AscendingOrder);
    listPossibleTags->clearSelection();
}

void CustomSTTView::btnUnselectAll_clicked() {
    listPossibleTags->clearSelection();
}

void CustomSTTView::btnUnselect_clicked() {
    qDeleteAll(listSelectedTags->selectedItems());

    SarfTagType * stt;
    for(int i=0; i<_atagger->sarfTagTypeVector->count(); i++) {
        if(_atagger->sarfTagTypeVector->at(i).tag == cbTagName->currentText()) {
            stt = &((*(_atagger->sarfTagTypeVector))[i]);
            break;
        }
    }

    stt->tags.clear();

    for(int i=0; i<listSelectedTags->topLevelItemCount(); i++) {
        QString data1 = listSelectedTags->topLevelItem(i)->text(0);
        QString data2 = listSelectedTags->topLevelItem(i)->text(1);
        QPair <QString, QString> pair(data1, data2);
        stt->tags.append(pair);
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
    else if(cbTagType->currentText() == "Gloss") {
        list = &listGloss;
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

    QRegExp regExp(text);
    listPossibleTags->clear();
    if(editPattern->text().isEmpty()) {
        listPossibleTags->addItems(*list);
    }
    else {
        listPossibleTags->addItems(list->filter(regExp));
    }
}

void CustomSTTView::btnLoad_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Open Sarf Tag Types"), "",
             tr("Tag Types (*.sarftagtypes);;All Files (*)"));

    if (fileName.isEmpty()) {
        return;
    }
    else {
        QFile file(fileName);
         if (!file.open(QIODevice::ReadOnly)) {
             QMessageBox::information(this, tr("Unable to open file"),file.errorString());
             return;
         }

         QByteArray sarfTT = file.readAll();
         file.close();

         QJson::Parser parser;
         bool ok;

         QVariantMap result = parser.parse(sarfTT,&ok).toMap();
         foreach(QVariant type, result["TagSet"].toList()) {
             QVariantMap typeElements = type.toMap();

             QString tag = typeElements["Tag"].toString();
             QString desc = typeElements["Description"].toString();
             int id = typeElements["Legend"].toInt();
             QString foreground_color = typeElements["foreground_color"].toString();
             QString background_color = typeElements["background_color"].toString();
             int font = typeElements["font"].toInt();
             bool underline = typeElements["underline"].toBool();
             bool bold = typeElements["bold"].toBool();
             bool italic = typeElements["italic"].toBool();

             QVector < QPair < QString, QString> > tags;
             foreach(QVariant sarfTags, typeElements["Tags"].toList()) {
                 QVariantMap st = sarfTags.toMap();
                 QPair<QString, QString> pair;
                 if(!(st.value("Prefix").isNull())) {
                     pair.first = "Prefix";
                     pair.second = st.value("Prefix").toString();
                     tags.append(pair);
                 }
                 else if(!(st.value("Stem").isNull())) {
                     pair.first = "Stem";
                     pair.second = st.value("Stem").toString();
                     tags.append(pair);
                 }
                 else if(!(st.value("Suffix").isNull())) {
                     pair.first = "Suffix";
                     pair.second = st.value("Suffix").toString();
                     tags.append(pair);
                 }
                 else if(!(st.value("Prefix-POS").isNull())) {
                     pair.first = "Prefix-POS";
                     pair.second = st.value("Prefix-POS").toString();
                     tags.append(pair);
                 }
                 else if(!(st.value("Stem-POS").isNull())) {
                     pair.first = "Stem-POS";
                     pair.second = st.value("Stem-POS").toString();
                     tags.append(pair);
                 }
                 else if(!(st.value("Suffix-POS").isNull())) {
                     pair.first = "Suffix-POS";
                     pair.second = st.value("Suffix-POS").toString();
                     tags.append(pair);
                 }
                 else if(!(st.value("Gloss").isNull())) {
                     pair.first = "Gloss";
                     pair.second = st.value("Gloss").toString();
                     tags.append(pair);
                 }
             }

             _atagger->insertSarfTagType(tag,tags,desc,id,foreground_color,background_color,font,underline,bold,italic);
         }

         cbTagName->clear();
         listSelectedTags->clear();
         _atagger->sarfTagTypeVector->clear();

         for(int i=0; i<_atagger->sarfTagTypeVector->count(); i++) {
             const SarfTagType * stt = &(_atagger->sarfTagTypeVector->at(i));
             cbTagName->addItem(stt->tag);
         }

         cbTagName->setCurrentIndex(0);
     }
}

void CustomSTTView::btnSave_clicked() {
    QByteArray sarftagtypeData = _atagger->dataInJsonFormat(sarfTTV);

    QString fileName;
    if(_atagger->sarftagtypeFile.isEmpty()) {
        fileName = QFileDialog::getSaveFileName(
                this,
                tr("Save Sarf Tag Types"), "",
                tr("Text (*.sarftagtypes);;All Files (*)"));
    }
    else {
        fileName = _atagger->sarftagtypeFile;
    }

    if(fileName.isEmpty()) {
        QMessageBox::warning(this, "Warning", "The Sarf Tag Types file wasn't saved");
        return;
    }

    QFile tfile(fileName);
    /*
    if(_atagger->sarftagtypeFile.isEmpty() && tfile.exists()) {
        QMessageBox::warning(this,"Warning", "File already exists, please try another name");
        return;
    }
    */
    if (!tfile.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this,"Warning","Can't open tagtypes file to Save");
        return;
    }
    _atagger->sarftagtypeFile = fileName;

    QTextStream outtags(&tfile);
    outtags << sarftagtypeData;
    tfile.close();
}

void CustomSTTView::fgcolor_changed(QString color) {
    if(color.isNull() || color.isEmpty()) {
        return;
    }
    QString tag = cbTagName->currentText();
    for(int i=0; i<_atagger->sarfTagTypeVector->count(); i++) {
        SarfTagType * stt = &((*(_atagger->sarfTagTypeVector))[i]);
        if(stt->tag == tag) {
            stt->fgcolor = color;
        }
    }
}

void CustomSTTView::bgcolor_changed(QString color) {
    if(color.isNull() || color.isEmpty()) {
        return;
    }
    QString tag = cbTagName->currentText();
    for(int i=0; i<_atagger->sarfTagTypeVector->count(); i++) {
        SarfTagType * stt = &((*(_atagger->sarfTagTypeVector))[i]);
        if(stt->tag == tag) {
            stt->bgcolor = color;
        }
    }
}

void CustomSTTView::font_changed(QString fontSize) {
    if(fontSize.isNull() || fontSize.isEmpty()) {
        return;
    }
    QString tag = cbTagName->currentText();
    for(int i=0; i<_atagger->sarfTagTypeVector->count(); i++) {
        SarfTagType * stt = &((*(_atagger->sarfTagTypeVector))[i]);
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
    for(int i=0; i<_atagger->sarfTagTypeVector->count(); i++) {
        SarfTagType * stt = &((*(_atagger->sarfTagTypeVector))[i]);
        if(stt->tag == name) {
            editDescription->setText(stt->description);
            int index = cbfont->findText(QString::number(stt->font));
            if(index != -1) {
                cbfont->setCurrentIndex(index);
            }
            colorfgcolor->setColor(QColor(stt->fgcolor));
            colorbgcolor->setColor(QColor(stt->bgcolor));
            cbunderline->setChecked(stt->underline);
            cbBold->setChecked(stt->bold);
            cbItalic->setChecked(stt->italic);
            for(int j=0; j<stt->tags.count(); j++) {
                QStringList list;
                list << stt->tags.at(j).first << stt->tags.at(j).second;
                listSelectedTags->addTopLevelItem(new QTreeWidgetItem(list));
            }
        }
    }
}

void CustomSTTView::tagName_Edited(QString name) {

}

void CustomSTTView::underline_clicked(bool underline) {
    QString tag = cbTagName->currentText();
    for(int i=0; i<_atagger->sarfTagTypeVector->count(); i++) {
        SarfTagType * stt = &((*(_atagger->sarfTagTypeVector))[i]);
        if(stt->tag == tag) {
            stt->underline = underline;
        }
    }
}

void CustomSTTView::bold_clicked(bool bold) {
    QString tag = cbTagName->currentText();
    for(int i=0; i<_atagger->sarfTagTypeVector->count(); i++) {
        SarfTagType * stt = &((*(_atagger->sarfTagTypeVector))[i]);
        if(stt->tag == tag) {
            stt->bold = bold;
        }
    }
}

void CustomSTTView::italic_clicked(bool italic) {
    QString tag = cbTagName->currentText();
    for(int i=0; i<_atagger->sarfTagTypeVector->count(); i++) {
        SarfTagType * stt = &((*(_atagger->sarfTagTypeVector))[i]);
        if(stt->tag == tag) {
            stt->italic = italic;
        }
    }
}

void CustomSTTView::desc_edited() {
    QString tag = cbTagName->currentText();
    for(int i=0; i<_atagger->sarfTagTypeVector->count(); i++) {
        SarfTagType * stt = &((*(_atagger->sarfTagTypeVector))[i]);
        if(stt->tag == tag) {
            stt->description = editDescription->text();
        }
    }
}

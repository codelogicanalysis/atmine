#include <QtGui/QApplication>
#include "amtmainwindow.h"
#include <qjson/parser.h>

#include <QContextMenuEvent>
#include <QTextCodec>
#include <QTextStream>
#include<QDockWidget>
#include<QList>
#include <QMessageBox>
#include <QTextBlock>

#include "commonS.h"
#include <addtagview.h>
#include <addtagtypeview.h>
#include <removetagtypeview.h>
#include "global.h"
#include "edittagtypeview.h"
#include "customsttview.h"
#include "sarftag.h"
#include "textParsing.h"

bool parentCheck;
class SarfTag;

AMTMainWindow::AMTMainWindow(QWidget *parent) :
    QMainWindow(parent),
    browseFileDlg(NULL)
{   
    resize(800,600);

    // Used to check for parent Widget between windows (EditTagTypeView and AMTMainWindow)
    parentCheck = false;

    createActions();
    createMenus();
    //createDockWindows();

    setWindowTitle(tr("Arabic Morphological Tagger"));

    /** Initialize Sarf **/
    theSarf = new Sarf();
    bool all_set = theSarf->start(&output_str, &error_str, this);
    if(!all_set) {
        QMessageBox::warning(this,"Warning","Can't set up the Sarf Tool");
        return;
    }
    Sarf::use(theSarf);
    initialize_other();

    theSarf->out.setString(&output_str);
    theSarf->out.setCodec("utf-8");
    theSarf->displayed_error.setString(&error_str);
    theSarf->displayed_error.setCodec("utf-8");
}

void clearLayout(QLayout *layout) {
    QLayoutItem *item;
    while((item = layout->takeAt(0))) {
        if (item->layout()) {
            clearLayout(item->layout());
            delete item->layout();
        }
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

void AMTMainWindow::createDockWindows(bool open) {
    clearLayout(this->layout());

    QDockWidget *dock = new QDockWidget(tr("Text"), this);
    QHBoxLayout *hbox = new QHBoxLayout();
    lblTFName = new QLabel("Text File:",dock);
    lineEditTFName = new QLineEdit(dock);
    lineEditTFName->setReadOnly(true);
    btnTFName = new QPushButton("...",dock);
    connect(btnTFName, SIGNAL(clicked()), this, SLOT(loadText_clicked()));
    hbox->addWidget(lblTFName);
    hbox->addWidget(lineEditTFName);
    hbox->addWidget(btnTFName);

    QScrollArea *sa1 = new QScrollArea(dock);
    sa1->setLayout(hbox);
    sa1->setMaximumHeight(50);

    txtBrwsr = new QTextBrowser(dock);
    hbox->addWidget(txtBrwsr);
    txtBrwsr->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(txtBrwsr,SIGNAL(customContextMenuRequested(const QPoint&)), this,SLOT(showContextMenu(const QPoint&)));

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->addWidget(sa1);
    vbox->addWidget(txtBrwsr);

    QScrollArea *sa = new QScrollArea(dock);
    sa->setLayout(vbox);

    dock->setWidget(sa);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
    dock->setMinimumWidth(300);

    if(open) {
        lineEditTFName->setText(_atagger->textFile);
    }

    dock = new QDockWidget(tr("Tags/TagTypes"), this);
    hbox = new QHBoxLayout();
    lblTTFName = new QLabel("Tag Type File:",dock);
    lineEditTTFName = new QLineEdit(dock);
    lineEditTTFName->setReadOnly(true);
    btnTTFName = new QPushButton("...",dock);
    connect(btnTTFName, SIGNAL(clicked()), this, SLOT(loadTagTypes_clicked()));
    hbox->addWidget(lblTTFName);
    hbox->addWidget(lineEditTTFName);
    hbox->addWidget(btnTTFName);

    sa = new QScrollArea(dock);
    sa->setLayout(hbox);
    sa->setMaximumHeight(50);

    vbox = new QVBoxLayout();
    vbox->addWidget(sa);

    tagDescription = new QTreeWidget(dock);
    tagDescription->setColumnCount(5);
    QStringList columns;
    columns << "Word" << "Tag" << "Source" << "Start" << "Length";
    QTreeWidgetItem* item=new QTreeWidgetItem(columns);
    connect(tagDescription,SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(itemSelectionChanged(QTreeWidgetItem*,int)));
    tagDescription->setHeaderItem(item);
    vbox->addWidget(tagDescription);

    descBrwsr = new QTreeWidget(dock);
    descBrwsr->setColumnCount(2);
    QStringList columnsD;
    columnsD << "Field" << "Value";
    QTreeWidgetItem* itemD=new QTreeWidgetItem(columnsD);
    descBrwsr->setHeaderItem(itemD);

    vbox->addWidget(descBrwsr);

    sa = new QScrollArea(dock);
    sa->setLayout(vbox);

    dock->setWidget(sa);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
    dock->setMinimumWidth(300);

    if(open) {
        lineEditTFName->setText(_atagger->tagtypeFile);
    }

    /*
    dock = new QDockWidget(tr("Text View"), this);
    txtBrwsr = new QTextBrowser(dock);
    dock->setWidget(txtBrwsr);
    txtBrwsr->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(txtBrwsr,SIGNAL(customContextMenuRequested(const QPoint&)), this,SLOT(showContextMenu(const QPoint&)));
    //setCentralWidget(txtBrwsr);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
    */

    /*
    dock = new QDockWidget(tr("Tag View"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    tagDescription = new QTreeWidget(dock);
    tagDescription->setColumnCount(5);
    QStringList columns;
    columns << "Word" << "Tag" << "Source" << "Start" << "Length";
    QTreeWidgetItem* item=new QTreeWidgetItem(columns);
    connect(tagDescription,SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(itemSelectionChanged(QTreeWidgetItem*,int)));
    tagDescription->setHeaderItem(item);
    dock->setWidget(tagDescription);
    dock->setMinimumWidth(300);

    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());

    dock = new QDockWidget(tr("Description"), this);
    descBrwsr = new QTreeWidget(dock);
    descBrwsr->setColumnCount(2);
    QStringList columnsD;
    columnsD << "Field" << "Value";
    QTreeWidgetItem* itemD=new QTreeWidgetItem(columnsD);
    descBrwsr->setHeaderItem(itemD);

    dock->setWidget(descBrwsr);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
    */
}

void AMTMainWindow::showContextMenu(const QPoint &pt) {

    signalMapper = new QSignalMapper(this);
    QMenu * menu = new QMenu();
    QMenu * mTags;
    mTags = menu->addMenu(tr("&Tag"));
    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        QAction * taginstance;
        taginstance = new QAction((_atagger->tagTypeVector->at(i))->tag,this);
        signalMapper->setMapping(taginstance, (_atagger->tagTypeVector->at(i))->tag);
        connect(taginstance, SIGNAL(triggered()), signalMapper, SLOT(map()));
        mTags->addAction(taginstance);
    }
    connect(signalMapper, SIGNAL(mapped(const QString &)), this, SLOT(tag(QString)));
    menu->addAction(untagMAct);
    menu->addSeparator();
    menu->addAction(addtagAct);

    if(txtBrwsr->textCursor().selectedText().isEmpty()) {
        myTC = txtBrwsr->cursorForPosition(pt);
        myTC.select(QTextCursor::WordUnderCursor);
        QString word = myTC.selectedText();

        if(word.isEmpty()) {
            mTags->setEnabled(false);
            untagMAct->setEnabled(false);
        }
        else {
            untagMAct->setEnabled(true);
        }
    }
    else {
        myTC = txtBrwsr->textCursor();
        untagMAct->setEnabled(true);
    }
    menu->exec(txtBrwsr->mapToGlobal(pt));
    delete menu;
}

void AMTMainWindow::open() {

    /** Initialize Tagger instance again **/

    _atagger = NULL;
    _atagger = new ATagger();

    /** Get and open tags file **/

    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Open Tagged Text File"), "",
             tr("Tag Types (*.tags.json);;All Files (*)"));

    if(fileName.isEmpty()) {
        return;
    }

    QFile ITfile(fileName);
    if (!ITfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::about(this, tr("Input File"),
                     tr("The <b>File</b> can't be opened!"));
        return;
    }

    _atagger->tagFile = fileName;
    //_atagger->sarftagFile = fileName;

    QByteArray Tags = ITfile.readAll();
    ITfile.close();

    QJson::Parser parser;
    bool ok;

    QVariantMap result = parser.parse (Tags,&ok).toMap();

    if (!ok) {
        QMessageBox::about(this, tr("Input Tag File"),
                     tr("The <b>Tag File</b> has a wrong format"));
        return;
    }

    /** Read text file path **/

    _atagger->textFile = result["file"].toString();

    QFile Ifile(_atagger->textFile);
    if (!Ifile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::about(this, tr("Input text File"),
                     tr("The <b>Input Text File</b> can't be opened!"));
        return;
    }

    /** Get text and check if compatible with tag file through text check sum **/

    QString text = Ifile.readAll();
    int textchecksum = result["textchecksum"].toInt();
    if(textchecksum != text.count()) {
        QMessageBox::warning(this,"Warning","The input text file is Inconsistent with Tag Information!");
        _atagger->tagFile.clear();
        _atagger->textFile.clear();
        return;
    }

    createDockWindows(true);

    lineEditTFName->setText(_atagger->textFile);
    _atagger->text = text;
    Ifile.close();

    startTaggingText(text);
    process(Tags);
    finishTaggingText();

    if(!_atagger->text.isEmpty()) {
        sarfAct->setEnabled(true);
    }
    edittagtypesAct->setEnabled(true);
    sarftagsAct->setEnabled(true);
    tagremoveAct->setEnabled(true);
    mTags->setEnabled(true);
    saveAct->setEnabled(true);
    saveasAct->setEnabled(true);
}

void AMTMainWindow::startTaggingText(QString & text) {
    if (this==NULL)
        return;
    QTextBrowser * taggedBox=txtBrwsr;
    taggedBox->clear();
    taggedBox->setLayoutDirection(Qt::RightToLeft);
    QTextCursor c=taggedBox->textCursor();
    c.clearSelection();
    c.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
    taggedBox->setTextCursor(c);
    taggedBox->setTextBackgroundColor(Qt::white);
    taggedBox->setTextColor(Qt::black);
    taggedBox->setText(text);
    setLineSpacing(10);
}

void AMTMainWindow::process(QByteArray & json) {

    QJson::Parser parser;
    bool ok;

    QVariantMap result = parser.parse (json,&ok).toMap();

    QString tagtypeFile = result.value("TagTypeFile").toString();
    if(tagtypeFile.isEmpty()) {
        QMessageBox::warning(this,"Warning","The Tag Type File is not specified!");
        txtBrwsr->clear();
        _atagger = new ATagger();
        return;
    }

    /** Read Tags **/

    foreach(QVariant tag, result["TagArray"].toList()) {

        QVariantMap tagElements = tag.toMap();
        int start = tagElements["pos"].toInt();
        int length = tagElements["length"].toInt();
        QString tagtype = tagElements["type"].toString();
        Source source = (Source)(tagElements["source"].toInt());
        bool check;
        check = _atagger->insertTag(tagtype,start,length,source);
    }

    if(_atagger->tagVector->at(0).source == user) {
        _atagger->isSarf = false;
    }
    else {
        _atagger->isSarf = true;
    }

    //if(_atagger->isSarf) {
        //_atagger->sarftagtypeFile = tagtypeFile;
    //}
    //else {
    _atagger->tagtypeFile = tagtypeFile;
    //}
    lineEditTTFName->setText(_atagger->tagtypeFile);

    /** Read the TagType file and store it **/

    QString ttFName;
    //if(_atagger->isSarf) {
        //ttFName = _atagger->sarftagtypeFile;
    //}
    //else {
    ttFName = _atagger->tagtypeFile;
    //}

    QFile ITfile(ttFName);
    if (!ITfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::about(this, tr("Input Tag File"),
                     tr("The <b>Tag Type File</b> can't be opened!"));
        txtBrwsr->clear();
        _atagger = new ATagger();
        lineEditTFName->clear();
        lineEditTTFName->clear();
        return;
    }

    QByteArray tagtypedata = ITfile.readAll();
    ITfile.close();

    lineEditTTFName->setText(ttFName);
    process_TagTypes(tagtypedata);


    setWindowTitle("AM Tagger: " + _atagger->tagFile);
    /** Apply Tags on Input Text **/

    applyTags();

    /** Add Tags to tagDescription Tree **/

    fillTreeWidget(user);
    createTagMenu();
}

void AMTMainWindow::process_TagTypes(QByteArray &tagtypedata) {
    QJson::Parser parser;
    bool ok;

    QVariantMap result = parser.parse (tagtypedata,&ok).toMap();

    if (!ok) {
        QMessageBox::about(this, tr("Input Tag File"),
                     tr("The <b>Tag File</b> has a wrong format"));
    }

    if(result.value("TagSet").toList().at(0).toMap().value("Features").isNull()) {
        for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
            const TagType& tt = *(_atagger->tagTypeVector->at(i));
            for(int j=0; j<_atagger->tagVector->count(); j++) {
                const Tag & tag = (_atagger->tagVector->at(j));
                if(tt.tag == tag.type) {
                    int start = tag.pos;
                    int length = tag.length;
                    tagWord(start,length,QColor("black"),QColor("white"),12,false,false,false);
                    _atagger->tagVector->remove(i);
                }
            }
        }
        _atagger->tagTypeVector->clear();
        fillTreeWidget(user);
    }

    foreach(QVariant type, result["TagSet"].toList()) {
        QString tag;
        QString desc;
        int id;
        QString foreground_color;
        QString background_color;
        int font;
        bool underline;
        bool bold;
        bool italic;

        QVariantMap typeElements = type.toMap();

        tag = typeElements["Tag"].toString();
        desc = typeElements["Description"].toString();
        id = typeElements["Legend"].toInt();
        foreground_color = typeElements["foreground_color"].toString();
        background_color = typeElements["background_color"].toString();
        font = typeElements["font"].toInt();
        underline = typeElements["underline"].toBool();
        bold = typeElements["bold"].toBool();
        italic = typeElements["italic"].toBool();

        if(!typeElements.value("Features").isNull()) {

            QVector < QPair < QString, QString> > tags;
            foreach(QVariant sarfTags, typeElements["Features"].toList()) {
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

            _atagger->isSarf = true;
            _atagger->insertSarfTagType(tag,tags,desc,id,foreground_color,background_color,font,underline,bold,italic,sarf);
        }
        else {
            _atagger->isSarf = false;
            _atagger->insertTagType(tag,desc,id,foreground_color,background_color,font,underline,bold,italic,user);
        }
    }
}

void AMTMainWindow::applyTags() {

    /*
    if(_atagger->isSarf) {
        for(int i=0; i<_atagger->tagVector->count(); i++) {
            for(int j=0; j<_atagger->sarfTagTypeVector->count(); j++) {
                QString type = (_atagger->tagVector->at(i)).type;
                QString tag = (_atagger->sarfTagTypeVector->at(j)).tag;
                if(type == tag) {
                    int start = (_atagger->tagVector->at(i)).pos;
                    int length = (_atagger->tagVector->at(i)).length;
                    QColor fgcolor = QColor((_atagger->sarfTagTypeVector->at(j)).fgcolor);
                    QColor bgcolor = QColor((_atagger->sarfTagTypeVector->at(j)).bgcolor);
                    int font = (_atagger->sarfTagTypeVector->at(j)).font;
                    bool underline = (_atagger->sarfTagTypeVector->at(j)).underline;
                    bool bold = (_atagger->sarfTagTypeVector->at(j)).underline;
                    bool italic = (_atagger->sarfTagTypeVector->at(j)).italic;
                    tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold);
                }
            }
        }
        fillTreeWidget(sarf);
    }
    */
    //else {
        for(int i =0; i< _atagger->tagVector->count(); i++) {
            for(int j=0; j< _atagger->tagTypeVector->count(); j++) {
                if((_atagger->tagVector->at(i)).type == (_atagger->tagTypeVector->at(j))->tag) {
                    int start = (_atagger->tagVector->at(i)).pos;
                    int length = (_atagger->tagVector->at(i)).length;
                    QColor bgcolor((_atagger->tagTypeVector->at(j))->bgcolor);
                    QColor fgcolor((_atagger->tagTypeVector->at(j))->fgcolor);
                    int font = (_atagger->tagTypeVector->at(j))->font;
                    bool underline = (_atagger->tagTypeVector->at(j))->underline;
                    bool bold = (_atagger->tagTypeVector->at(j))->bold;
                    bool italic = (_atagger->tagTypeVector->at(j))->italic;
                    tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold);
                    break;
                }
            }
        }
        fillTreeWidget(user);
    //}
    createTagMenu();
}

void AMTMainWindow::tagWord(int start, int length, QColor fcolor, QColor  bcolor,int font, bool underline, bool italic, bool bold){
    if (this==NULL)
        return;
    QTextBrowser * taggedBox= txtBrwsr;
    //QTextEdit * taggedBox = txtBrwsr;
    QTextCursor c=taggedBox->textCursor();
#if 0
    int lastpos=c.position();
    int diff=start-lastpos;
    if (diff>=0)
        c.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,diff);
    else
        c.movePosition(QTextCursor::Left,QTextCursor::MoveAnchor,-diff);
    c.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor,length);
#else
    /*if (length>200) {
        start=start+length-1;
        length=5;
        color=Qt::red;
    }*/
    c.setPosition(start,QTextCursor::MoveAnchor);
    c.setPosition(start+length,QTextCursor::KeepAnchor);
#endif
    taggedBox->setTextCursor(c);
    taggedBox->setTextColor(fcolor);
    taggedBox->setTextBackgroundColor(bcolor);
    taggedBox->setFontUnderline(underline);
    taggedBox->setFontItalic(italic);
    if(bold)
        taggedBox->setFontWeight(QFont::Bold);
    else
        taggedBox->setFontWeight(QFont::Normal);
    taggedBox->setFontPointSize(font);
    //taggedBox->setFontPointSize();
}

void AMTMainWindow::finishTaggingText() {
    if (this==NULL)
        return;
    QTextBrowser * taggedBox= txtBrwsr;
    //QTextEdit * taggedBox = txtBrwsr;
    QTextCursor c=taggedBox->textCursor();
    c.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
    taggedBox->setTextCursor(c);
}

bool AMTMainWindow::saveFile(const QString &fileName, QByteArray &tagD) {

    QFile tfile(fileName);
    if (!tfile.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this,"Warning","Can't open tags file to Save");
        return false;
    }
    QTextStream outtags(&tfile);
    outtags << tagD;
    tfile.close();

    return true;
}

void AMTMainWindow::save() {
    QString fileName = _atagger->tagFile;

    if(_atagger->tagFile.isEmpty()) {
        QString fileName = QFileDialog::getSaveFileName(this,
                                                        tr("Save Tags File Name"), "",
                                                        tr("Tags (*.tags.json);;All Files (*)"));
        if(fileName.isEmpty()) {
            return;
        }
        else {
            _atagger->tagFile = fileName + ".tags.json";
            setWindowTitle("AMTagger: " + _atagger->tagFile);
        }
    }

    if(_atagger->textFile.isEmpty() || _atagger->tagtypeFile.isEmpty()) {
        QMessageBox::warning(this,"Warning","Couldn't save since text file or tag type file is not specified!");
        return;
    }

    if(_atagger->tagVector->isEmpty()) {
        QMessageBox::warning(this,"Warning","No tags to save!");
        return;
    }

    /** Save to Default Destination **/
    QByteArray tagData = _atagger->dataInJsonFormat(tagV);
    saveFile(fileName,tagData);
}

bool AMTMainWindow::saveas() {

    /** Save to specified Destination with .tag extension **/

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Tags File"), "",
                                                    tr("tags (*.tags.json);;All Files (*)"));

    if (fileName.isEmpty())
        return false;

    fileName += ".tags.json";

    QByteArray tagData;
    if(_atagger->isSarf) {
        tagData = _atagger->dataInJsonFormat(sarfTV);
    }
    else {
        tagData = _atagger->dataInJsonFormat(tagV);
    }
    return saveFile(fileName,tagData);
}

/** Not needed Anymore**/
void AMTMainWindow::tagadd() {
    //QTextCursor cursor = txtBrwsr->textCursor();
    //AddTagView * atv = new AddTagView(cursor.selectionStart(), cursor.selectionEnd(), this);
    if(txtBrwsr->textCursor().selectedText() != "") {
        AddTagView * atv = new AddTagView(txtBrwsr, this);
        atv->show();
    }
    else {
        switch( QMessageBox::information( this, "Add Tag","No word is selected for tagging!","&Ok",0,0) ) {
            return;
        }
    }
}

void AMTMainWindow::tagremove() {
    QTextCursor cursor = txtBrwsr->textCursor();
    int start = cursor.selectionStart();
    int length = cursor.selectionEnd() - cursor.selectionStart();
    for(int i=0; i < _atagger->tagVector->count(); i++) {
        if((_atagger->tagVector->at(i)).pos == start) {
            tagWord(start,length,QColor("black"),QColor("white"),9,false,false,false);
            _atagger->tagVector->remove(i);
            cursor.clearSelection();
            fillTreeWidget(user);
        }
    }
}

void AMTMainWindow::edittagtypes() {

    if(_atagger->tagtypeFile.isEmpty()) {
        QString ttFileName = QFileDialog::getSaveFileName(this,
                                                          tr("TagType File Name"), "",
                                                          tr("tag types (*.tt.json);;All Files (*)"));
        if(ttFileName.isEmpty())
        {
            return;
        }
        else {
            _atagger->tagtypeFile = ttFileName + ".tt.json";
            lineEditTTFName->setText(_atagger->tagtypeFile);
            btnTTFName->setEnabled(false);
        }
    }

    EditTagTypeView * ettv = new EditTagTypeView(this);
    ettv->show();
}

void AMTMainWindow::tagtypeadd() {
    parentCheck = false;
    AddTagTypeView * attv = new AddTagTypeView(this);
    attv->show();
}

void AMTMainWindow::tagtyperemove() {
    RemoveTagTypeView * rttv = new RemoveTagTypeView(txtBrwsr,tagDescription,this);
    rttv->show();
}

void AMTMainWindow::tag(QString tagValue) {

    if(!(myTC.selectedText().isEmpty())) {
        if(_atagger->tagFile.isEmpty()) {
            QString fileName = QFileDialog::getSaveFileName(this,
                                                            tr("Tags File"), "",
                                                            tr("Tags (*.tags.json);;All Files (*)"));
            if(fileName.isEmpty()) {
                QMessageBox::warning(this,"Warning","You have to add a tag file name before tagging the text!");
                return;
            }
            else {
                _atagger->tagFile = fileName + ".tags.json";
                setWindowTitle("AMTagger: " + _atagger->tagFile);
            }
        }
        QTextCursor cursor = myTC;
        int start = cursor.selectionStart();
        int length = cursor.selectionEnd() - cursor.selectionStart();
        _atagger->insertTag(tagValue, start, length, user);

        for(int i=0; i< _atagger->tagTypeVector->count(); i++) {
            if((_atagger->tagTypeVector->at(i))->tag == tagValue) {
                QColor bgcolor((_atagger->tagTypeVector->at(i))->bgcolor);
                QColor fgcolor((_atagger->tagTypeVector->at(i))->fgcolor);
                int font = (_atagger->tagTypeVector->at(i))->font;
                bool underline = (_atagger->tagTypeVector->at(i))->underline;
                bool bold = (_atagger->tagTypeVector->at(i))->bold;
                bool italic = (_atagger->tagTypeVector->at(i))->italic;

                tagWord(cursor.selectionStart(),cursor.selectionEnd()-cursor.selectionStart(),fgcolor,bgcolor,font,underline,italic,bold);
            }
        }
        cursor.clearSelection();
        fillTreeWidget(user);
    }
    else {
        switch( QMessageBox::information( this, "Add Tag","No word is selected for tagging!","&Ok",0,0) ) {
            return;
        }
    }
}

void AMTMainWindow::untag() {

    QTextCursor cursor = myTC;
    int start = cursor.selectionStart();
    int length = cursor.selectionEnd() - cursor.selectionStart();
    for(int i=0; i < _atagger->tagVector->count(); i++) {
        if((_atagger->tagVector->at(i)).pos == start) {
            tagWord(start,length,QColor("black"),QColor("white"),12,false,false,false);
            _atagger->tagVector->remove(i);
            cursor.clearSelection();
            fillTreeWidget(user);
        }
    }
}

void AMTMainWindow::addtagtype() {
    AddTagTypeView * attv = new AddTagTypeView();
    attv->show();

}

void AMTMainWindow::about() {

}

void AMTMainWindow::aboutQt() {

}

void AMTMainWindow::createActions()
{
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Start a new Tag Project"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(_new()));

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setEnabled(false);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveasAct = new QAction(tr("&SaveAs"), this);
    saveasAct->setEnabled(false);
    saveasAct->setShortcuts(QKeySequence::SaveAs);
    saveasAct->setStatusTip(tr("Print the document"));
    connect(saveasAct, SIGNAL(triggered()), this, SLOT(saveas()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    tagaddAct = new QAction(tr("&Tag"), this);
    //tagaddAct->setShortcuts(QKeySequence::);
    tagaddAct->setStatusTip(tr("Add a Tag"));
    connect(tagaddAct, SIGNAL(triggered()), this, SLOT(tagadd()));

    tagremoveAct = new QAction(tr("&Untag"), this);
    tagremoveAct->setEnabled(false);
    tagremoveAct->setStatusTip(tr("Remove a Tag"));
    connect(tagremoveAct, SIGNAL(triggered()), this, SLOT(tagremove()));

    edittagtypesAct = new QAction(tr("Edit TagTypes..."),this);
    edittagtypesAct->setEnabled(false);
    edittagtypesAct->setStatusTip(tr("Edit Tag Types"));
    connect(edittagtypesAct, SIGNAL(triggered()), this, SLOT(edittagtypes()));

    tagtypeaddAct = new QAction(tr("&TagType Add"), this);
    tagtypeaddAct->setStatusTip(tr("Add a TagType"));
    connect(tagtypeaddAct, SIGNAL(triggered()), this, SLOT(tagtypeadd()));

    tagtyperemoveAct = new QAction(tr("&TagType Remove"), this);
    tagtyperemoveAct->setStatusTip(tr("Remove a TagType"));
    connect(tagtyperemoveAct, SIGNAL(triggered()), this, SLOT(tagtyperemove()));

    sarftagsAct = new QAction(tr("Edit Sarf TagTypes..."), this);
    sarftagsAct->setEnabled(false);
    connect(sarftagsAct, SIGNAL(triggered()), this, SLOT(customizeSarfTags()));

    sarfAct = new QAction(tr("Run Sarf"), this);
    sarfAct->setEnabled(false);
    //tagAct->setShortcuts(QKeySequence::Cut);
    //tagAct->setStatusTip(tr("Tag selected word"));
    connect(sarfAct, SIGNAL(triggered()), this, SLOT(sarfTagging()));

    untagMAct = new QAction(tr("Untag"), this);
    //untagAct->setShortcuts(QKeySequence::Copy);
    untagMAct->setStatusTip(tr("Untag selected word"));
    connect(untagMAct, SIGNAL(triggered()), this, SLOT(untag()));

    addtagAct = new QAction(tr("&Add TagType"), this);
    //addtagAct->setShortcuts(QKeySequence::Paste);
    addtagAct->setStatusTip(tr("Add a TagType"));
    connect(addtagAct, SIGNAL(triggered()), this, SLOT(addtagtype()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}

void AMTMainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveasAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    tagMenu = menuBar()->addMenu(tr("&Tags"));
    mTags = tagMenu->addMenu(tr("&Tag"));
    mTags->setEnabled(false);
    createTagMenu();

    tagMenu->addAction(tagremoveAct);
    tagMenu->addSeparator();
    tagMenu->addAction(edittagtypesAct);

    /*
    tagtypeMenu = menuBar()->addMenu(tr("&TagType"));
    tagtypeMenu->addAction(tagtypeaddAct);
    tagtypeMenu->addAction(tagtyperemoveAct);
    tagtypeMenu->addSeparator();
    */

    sarfMenu = menuBar()->addMenu(tr("Sarf"));
    sarfMenu->addAction(sarftagsAct);
    sarfMenu->addAction(sarfAct);

    viewMenu = menuBar()->addMenu(tr("&View"));

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
}

void AMTMainWindow::createTagMenu() {
    mTags->clear();
    signalMapperM = new QSignalMapper(this);
    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        QAction * taginstance;
        taginstance = new QAction((_atagger->tagTypeVector->at(i))->tag,this);
        signalMapperM->setMapping(taginstance, (_atagger->tagTypeVector->at(i))->tag);
        connect(taginstance, SIGNAL(triggered()), signalMapperM, SLOT(map()));
        mTags->addAction(taginstance);
    }
    connect(signalMapperM, SIGNAL(mapped(const QString &)), this, SLOT(tag(QString)));
}

void AMTMainWindow::fillTreeWidget(Source Data) {
    tagDescription->clear();
     QList<QTreeWidgetItem *> items;
     QVector<Tag> *temp = new QVector<Tag>(*(_atagger->tagVector));
     for(int i=0; i < temp->count(); i++) {
         QStringList entry;
         QTextCursor cursor = txtBrwsr->textCursor();
         int pos = (temp->at(i)).pos;
         int length = (temp->at(i)).length;
         cursor.setPosition(pos,QTextCursor::MoveAnchor);
         cursor.setPosition(pos + length,QTextCursor::KeepAnchor);
         QString text = cursor.selectedText();
         entry<<text;
         entry<<(temp->at(i)).type;
         QString src;
         if((temp->at(i)).source == user) {
            src = "user";
        }
        else {
            src = "sarf";
        }
        entry<<src;
        entry<<QString::number(pos);
        entry<<QString::number(length);
        items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
    }
     tagDescription->insertTopLevelItems(0, items);
}

void AMTMainWindow::itemSelectionChanged(QTreeWidgetItem* item ,int i) {
    descBrwsr->clear();
    txtBrwsr->textCursor().clearSelection();
    QList<QTreeWidgetItem *> items;
    QString type = item->text(1);
/*
    if(_atagger->isSarf) {

        for(int j=0; j < _atagger->sarfTagTypeVector->count(); j++) {
            if((_atagger->sarfTagTypeVector->at(j)).tag == type) {
                QString desc = (_atagger->sarfTagTypeVector->at(j)).description;
                QColor bgcolor((_atagger->sarfTagTypeVector->at(j)).bgcolor);
                QColor fgcolor((_atagger->sarfTagTypeVector->at(j)).fgcolor);
                int font = (_atagger->sarfTagTypeVector->at(j)).font;
                bool underline = (_atagger->sarfTagTypeVector->at(j)).underline;
                bool bold = (_atagger->sarfTagTypeVector->at(j)).bold;
                bool italic = (_atagger->sarfTagTypeVector->at(j)).italic;

                QTextCursor c = txtBrwsr->textCursor();
                c.setPosition(item->text(3).toInt(),QTextCursor::MoveAnchor);
                c.setPosition(item->text(3).toInt() + item->text(4).toInt(),QTextCursor::KeepAnchor);
                txtBrwsr->setTextCursor(c);

                QStringList entry;
                entry << "Word" << item->text(0);
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                entry << "Description" <<desc;
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                entry << "TagType" << type;
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                entry << "Source" << item->text(2);
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                entry << "Position" << item->text(3);
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                entry << "Length" << item->text(4);
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                entry << "Background Color" << QString();//bgcolor.name();
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                items.last()->setBackgroundColor(1,bgcolor);
                entry.clear();
                entry << "Foreground Color" << QString();//fgcolor.name();
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                items.last()->setBackgroundColor(1,fgcolor);
                entry.clear();
                entry << "Font Size" << QString::number(font);
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                if(underline)
                    entry << "Underline" << "True";
                else
                    entry << "Underline" <<  "False";
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                if(bold)
                    entry << "Bold" << "True";
                else
                    entry << "Bold" << "False";
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                if(italic)
                    entry << "Italic" << "True";
                else
                    entry << "Italic" << "False";

                descBrwsr->insertTopLevelItems(0, items);
                break;
            }
        }
    }
    */
    //else {

        for(int j=0; j < _atagger->tagTypeVector->count(); j++) {
            if((_atagger->tagTypeVector->at(j))->tag == type) {
                QString desc = (_atagger->tagTypeVector->at(j))->description;
                QColor bgcolor((_atagger->tagTypeVector->at(j))->bgcolor);
                QColor fgcolor((_atagger->tagTypeVector->at(j))->fgcolor);
                int font = (_atagger->tagTypeVector->at(j))->font;
                bool underline = (_atagger->tagTypeVector->at(j))->underline;
                bool bold = (_atagger->tagTypeVector->at(j))->bold;
                bool italic = (_atagger->tagTypeVector->at(j))->italic;

                QTextCursor c = txtBrwsr->textCursor();
                c.setPosition(item->text(3).toInt(),QTextCursor::MoveAnchor);
                c.setPosition(item->text(3).toInt() + item->text(4).toInt(),QTextCursor::KeepAnchor);
                txtBrwsr->setTextCursor(c);

                QStringList entry;
                entry << "Word" << item->text(0);
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                entry << "Description" <<desc;
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                entry << "TagType" << type;
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                entry << "Source" << item->text(2);
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                entry << "Position" << item->text(3);
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                entry << "Length" << item->text(4);
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                entry << "Background Color" << QString();//bgcolor.name();
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                items.last()->setBackgroundColor(1,bgcolor);
                entry.clear();
                entry << "Foreground Color" << QString();//fgcolor.name();
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                items.last()->setBackgroundColor(1,fgcolor);
                entry.clear();
                entry << "Font Size" << QString::number(font);
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                if(underline)
                    entry << "Underline" << "True";
                else
                    entry << "Underline" <<  "False";
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                if(bold)
                    entry << "Bold" << "True";
                else
                    entry << "Bold" << "False";
                items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
                entry.clear();
                if(italic)
                    entry << "Italic" << "True";
                else
                    entry << "Italic" << "False";

                descBrwsr->insertTopLevelItems(0, items);
                break;
            }
        }
   //}
}

void AMTMainWindow::sarfTagging() {
    if(_atagger->text.isEmpty()) {
        QMessageBox::warning(this,"Warning","Add a text file before initializing the Sarf Analyzer!");
        return;
    }

    if(_atagger->tagFile.isEmpty()) {
        QString fileName = QFileDialog::getSaveFileName(this,
                                                        tr("Tags File"), "",
                                                        tr("Tags (*.tags.json);;All Files (*)"));
        if(fileName.isEmpty()) {
            QMessageBox::warning(this,"Warning","You have to add a tag file name before tagging the text!");
            return;
        }
        else {
            _atagger->tagFile = fileName + ".tags.json";
            setWindowTitle("AMTagger: " + _atagger->tagFile);
        }
    }

    _atagger->isSarf = true;
    startTaggingText(_atagger->text);
    _atagger->tagVector->clear();

    error_str = "";
    output_str = "";

    QString text = _atagger->text;
    QStringList textList = _atagger->text.split(' ',QString::SkipEmptyParts);
    int start = 0;
    int length = (textList[0]).count();
    for(int i=0; i< textList.count(); i++) {
        SarfTag sarftag(start, length, &(textList[i]), this);
        sarftag();
        if(i != (textList.count()-1)) {
            start = text.indexOf(textList[i+1],start + length);
            length = (textList[i+1]).count();
        }
    }

    for(int i=0; i<_atagger->tagVector->count(); i++) {
        for(int j=0; j<_atagger->tagTypeVector->count(); j++) {
            QString type = (_atagger->tagVector->at(i)).type;
            QString tag = (_atagger->tagTypeVector->at(j))->tag;
            if(type == tag) {
                int start = (_atagger->tagVector->at(i)).pos;
                int length = (_atagger->tagVector->at(i)).length;
                QColor fgcolor = QColor((_atagger->tagTypeVector->at(j))->fgcolor);
                QColor bgcolor = QColor((_atagger->tagTypeVector->at(j))->bgcolor);
                int font = (_atagger->tagTypeVector->at(j))->font;
                bool underline = (_atagger->tagTypeVector->at(j))->underline;
                bool bold = (_atagger->tagTypeVector->at(j))->bold;
                bool italic = (_atagger->tagTypeVector->at(j))->italic;
                tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold);
            }
        }
    }

    fillTreeWidget(sarf);
    finishTaggingText();
}

void AMTMainWindow::customizeSarfTags() {
    if(!_atagger->textFile.isEmpty()) {
        sarfAct->setEnabled(true);
    }

    if(_atagger->tagtypeFile.isEmpty()) {
        QString ttFileName = QFileDialog::getSaveFileName(this,
                                                          tr("Sarf TagType File Name"), "",
                                                          tr("tags (*.stt.json);;All Files (*)"));
        if(ttFileName.isEmpty())
        {
            return;
        }
        else {
            _atagger->tagtypeFile = ttFileName + ".stt.json";
            lineEditTTFName->setText(_atagger->tagtypeFile);
            btnTTFName->setEnabled(false);
        }
    }

    CustomSTTView * cttv = new CustomSTTView(this);
    //if(cttv->showWindow) {
    cttv->show();
    //}
}

void AMTMainWindow::loadText_clicked() {
    if(!(_atagger->textFile.isEmpty())) {
        QMessageBox msgBox;
        msgBox.setText("Loading a new text file will remove all previous tags");
        msgBox.setInformativeText("Do you want to proceed?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();

        switch (ret) {
           case QMessageBox::Yes:
               break;
           case QMessageBox::Discard:
               return;
               break;
           default:
               // should never be reached
               break;
         }
    }

    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Text File"), "",
             tr("Text File (*.txt);;All Files (*)"));

    if (fileName.isEmpty()) {
        return;
    }
    else {
        QFile file(fileName);
         if (!file.open(QIODevice::ReadOnly)) {
             QMessageBox::information(this, tr("Unable to open file"),file.errorString());
             return;
         }

         _atagger->tagVector->clear();
         tagDescription->clear();
         descBrwsr->clear();

         lineEditTFName->setText(fileName);
         sarfAct->setEnabled(true);
         //btnTFName->setEnabled(false);
         QString text = file.readAll();
         _atagger->textFile = fileName;
         _atagger->text = text;
         txtBrwsr->setText(text);
         setLineSpacing(10);
    }
}

void AMTMainWindow::loadTagTypes_clicked() {
    if(!(_atagger->tagTypeVector->isEmpty())) {
        QMessageBox msgBox;
        msgBox.setText("Loading a new tag type file will remove all previous tags");
        msgBox.setInformativeText("Do you want to proceed?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();

        switch (ret) {
           case QMessageBox::Yes:
               break;
           case QMessageBox::No:
               return;
               break;
           default:
               // should never be reached
               break;
         }
    }

    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Tag Type File"), "",
             tr("Tag Type File (*.tt.json *.stt.json);;All Files (*)"));

    if (fileName.isEmpty()) {
        return;
    }
    else {
        QFile file(fileName);
         if (!file.open(QIODevice::ReadOnly)) {
             QMessageBox::information(this, tr("Unable to open file"),file.errorString());
             return;
         }

         if(fileName.endsWith(".tt.json")) {
             _atagger->tagtypeFile = fileName;
         }
         else if(fileName.endsWith(".stt.json")) {
             _atagger->tagtypeFile = fileName;
             sarfAct->setEnabled(true);
         }
         else {
             QMessageBox::warning(this,"Warning","The selected file doesn't have the correct extension!");
             return;
         }

         _atagger->tagVector->clear();
         _atagger->tagTypeVector->clear();
         _atagger->tagtypeFile.clear();
         //_atagger->sarftagtypeFile.clear();
         tagDescription->clear();
         descBrwsr->clear();
         txtBrwsr->clear();
         txtBrwsr->setText(_atagger->text);
         applyTags();
         createTagMenu();

         lineEditTTFName->setText(fileName);
         QByteArray tagtypes = file.readAll();
         process_TagTypes(tagtypes);
     }
}

void AMTMainWindow::_new() {
	clearLayout(this->layout());

        _atagger = new ATagger();

	setWindowTitle("Arabic Morphological Tagger");

	QString tagFileName = QFileDialog::getSaveFileName(this,
			tr("Tags File"), "",
			tr("tags (*.tags.json);;All Files (*)"));
        if(tagFileName.isEmpty()) {
		return;
        }

        _atagger->tagFile = tagFileName + ".tags.json";
	setWindowTitle("AMTagger: " + _atagger->tagFile);


	createDockWindows(false);

	sarftagsAct->setEnabled(true);
	edittagtypesAct->setEnabled(true);
	tagremoveAct->setEnabled(true);
	mTags->setEnabled(true);
	saveAct->setEnabled(true);
	saveasAct->setEnabled(true);
}

void AMTMainWindow::setLineSpacing(int lineSpacing) {
    int lineCount = 0;
    for (QTextBlock block = txtBrwsr->document()->begin(); block.isValid();
            block = block.next(), ++lineCount) {
        QTextCursor tc = QTextCursor(block);
        QTextBlockFormat fmt = block.blockFormat();
        if (fmt.topMargin() != lineSpacing
                || fmt.bottomMargin() != lineSpacing) {
            fmt.setTopMargin(lineSpacing);
            tc.setBlockFormat(fmt);
        }
    }
}

void AMTMainWindow::resetActionDisplay() {

}

QString AMTMainWindow::getFileName() {
    return "";
}

void AMTMainWindow::tag(int start, int length,QColor color, bool textcolor=true) {

}

void AMTMainWindow::setCurrentAction(const QString & s) {

}

void AMTMainWindow::report(int value) {

}

AMTMainWindow::~AMTMainWindow() {

}

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForTr( QTextCodec::codecForName( "UTF-8" ) );
    QTextCodec::setCodecForCStrings( QTextCodec::codecForName( "UTF-8" ) );

    _atagger=new ATagger();

    //QString *test = new QString("ياكل  الولد التفاحة ويلعب استنشق الرجل الوردة أنف");
    //PunctuationInfo punc;
    //long result = next_positon(test,1,punc);

    QApplication a(argc, argv);
    AMTMainWindow w;
    w.show();

    int r = a.exec();

    if(_atagger != NULL) {
        delete _atagger;
    }

    if (theSarf != NULL) {
        theSarf->exit();
        delete theSarf;
    }

    return  r;
}

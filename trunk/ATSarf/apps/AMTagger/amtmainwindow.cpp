#include <QtGui/QApplication>
#include "amtmainwindow.h"
//#include "ui_amtmainwindow.h"
#include <qjson/parser.h>

#include <QContextMenuEvent>
#include <QTextCodec>
#include <QTextStream>
#include <QMessageBox>
#include<QDockWidget>

#include<common.h>
#include <addtagview.h>
#include <addtagtypeview.h>

AMTMainWindow::AMTMainWindow(QWidget *parent) :
    QMainWindow(parent),
    browseFileDlg(NULL)
{
    resize(800,600);

    //QDockWidget *dock = new QDockWidget(tr("Text View"), this);
    txtBrwsr = new QTextBrowser(this);
    //txtBrwsr = new QTextEdit();
    //dock->setWidget(txtBrwsr);

    //txtBrwsr->setContextMenuPolicy(Qt::CustomContextMenu);
    //connect(txtBrwsr,SIGNAL(customContextMenuRequested(const QPoint&)), this,SLOT(showContextMenu(const QPoint&)));
    setCentralWidget(txtBrwsr);

    createActions();
    createMenus();
    createDockWindows();

    setWindowTitle(tr("Arabic Morphological Tagger"));
}

void AMTMainWindow::createDockWindows() {

    QDockWidget *dock = new QDockWidget(tr("Tag View"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    tagDescription = new QTreeWidget(dock);
    dock->setWidget(tagDescription);

    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());

    dock = new QDockWidget(tr("Description"), this);
    descBrwsr = new QTextBrowser(dock);
    dock->setWidget(descBrwsr);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
}

void AMTMainWindow::showContextMenu(const QPoint &pt) {

    QMenu * menu = txtBrwsr->createStandardContextMenu();
    QMenu * tags;
    //menu = new QMenu();
    tags = menu->addMenu(tr("&Tag"));
    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        QAction * taginstance;
        char * tagValue = (_atagger->tagTypeVector->at(i)).tag.toLocal8Bit().data();
        taginstance = new QAction(tr(tagValue), this);
        connect(taginstance, SIGNAL(triggered()), this, SLOT(tag(tagValue)));
        tags->addAction(taginstance);
    }
    menu->addAction(untagAct);
    menu->addAction(addtagAct);
    menu->exec(txtBrwsr->mapToGlobal(pt));
    delete menu;
}

void AMTMainWindow::contextMenuEvent(QContextMenuEvent *event)
 {
     QMenu * menu;
     QMenu * tags;
     menu = new QMenu();
     tags = menu->addMenu(tr("&Tag"));
     for(int i=0; i<_atagger->tagTypeVector->count(); i++) {        
         QAction * taginstance;
         char * tagValue = (_atagger->tagTypeVector->at(i)).tag.toLocal8Bit().data();
         taginstance = new QAction(tr(tagValue), this);
         connect(taginstance, SIGNAL(triggered()), this, SLOT(tag(tagValue)));
         tags->addAction(taginstance);
     }
     menu->addAction(untagAct);
     menu->addAction(addtagAct);
     menu->exec(event->globalPos());
}

void AMTMainWindow::open()
{
    if (browseFileDlg == NULL) {
        QString dir = QDir::currentPath();
        browseFileDlg = new QFileDialog(NULL, QString("Open File"), dir, QString("All Files (*)"));
        browseFileDlg->setOptions(QFileDialog::DontUseNativeDialog);
        browseFileDlg->setFileMode(QFileDialog::ExistingFile);
        browseFileDlg->setViewMode(QFileDialog::Detail);
    }
     if (browseFileDlg->exec()){
        QStringList files = browseFileDlg->selectedFiles();
        QString fileName = files[0];

        QFile Ifile(fileName);
        if (!Ifile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::about(this, tr("Input File"),
                         tr("The <b>File</b> can't be opened!"));
        }

        QString text = Ifile.readAll();
        Ifile.close();

        _atagger->tagFile = fileName + ".tag";
        QFile ITfile(fileName+".tag");
        if (!ITfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::about(this, tr("Input Tag File"),
                         tr("The <b>Tag File</b> can't be opened!"));
        }

        QByteArray Tags = ITfile.readAll();

        ITfile.close();

        startTaggingText(text);
        process(Tags);
        finishTaggingText();
    }
}

void AMTMainWindow::startTaggingText(QString & text){
    if (this==NULL)
        return;
    QTextBrowser * taggedBox=txtBrwsr;
    //QTextEdit * taggedBox = txtBrwsr;
    taggedBox->clear();
    taggedBox->setLayoutDirection(Qt::RightToLeft);
    QTextCursor c=taggedBox->textCursor();
    c.clearSelection();
    c.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
    taggedBox->setTextCursor(c);
    taggedBox->setTextBackgroundColor(Qt::white);
    taggedBox->setTextColor(Qt::black);
    taggedBox->setText(text);
}

void AMTMainWindow::process(QByteArray & json) {

    QJson::Parser parser;
    bool ok;

    QVariantMap result = parser.parse (json,&ok).toMap();

    if (!ok) {
        QMessageBox::about(this, tr("Input Tag File"),
                     tr("The <b>Tag File</b> has a wrong format"));
    }

    _atagger->textFile = result["file"].toString();
    _atagger->tagtypeFile = result["TagTypeFile"].toString();

    foreach(QVariant tag, result["TagArray"].toList()) {

        QVariantMap tagElements = tag.toMap();
        int start = tagElements["pos"].toInt();
        int length = tagElements["length"].toInt();
        QString tagtype = tagElements["type"].toString();
        QString source = tagElements["source"].toString();
        bool check;
        if(source == "sarf") {
            check = _atagger->insertTag(tagtype,start,length,sarf);
        }
        else {
            check = _atagger->insertTag(tagtype,start,length,user);
        }
    }

    /** Read the TagType file and store it **/

    QFile ITfile(_atagger->tagtypeFile);
    if (!ITfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::about(this, tr("Input Tag File"),
                     tr("The <b>Tag Type File</b> can't be opened!"));
    }

    QByteArray tagtypedata = ITfile.readAll();
    ITfile.close();

    result = parser.parse (tagtypedata,&ok).toMap();

    if (!ok) {
        QMessageBox::about(this, tr("Input Tag File"),
                     tr("The <b>Tag File</b> has a wrong format"));
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
        _atagger->insertTagType(tag,desc,id,foreground_color,background_color,font,underline,bold,italic);
    }

    /** Apply Tags on Input Text **/

    for(int i =0; i< _atagger->tagVector->count(); i++) {
        for(int j=0; j< _atagger->tagTypeVector->count(); j++) {
            if((_atagger->tagVector->at(i)).type == (_atagger->tagTypeVector->at(j)).tag) {
                int start = (_atagger->tagVector->at(i)).pos;
                int length = (_atagger->tagVector->at(i)).length;
                QColor bgcolor((_atagger->tagTypeVector->at(j)).bgcolor);
                QColor fgcolor((_atagger->tagTypeVector->at(j)).fgcolor);
                int font = (_atagger->tagTypeVector->at(j)).font;
                bool underline = (_atagger->tagTypeVector->at(j)).underline;
                bool bold = (_atagger->tagTypeVector->at(j)).bold;
                bool italic = (_atagger->tagTypeVector->at(j)).italic;
                tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold);
                break;
            }
        }
    }

    /** Add Tags to tagDescription Tree **/

    fillTreeWidget();
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

void AMTMainWindow::save() {

    /** Convert TagType to QJSON **/

    QVariantMap tagtypedata;
    QVariantList tagtypeset;
    for(int i=0; i < _atagger->tagTypeVector->count(); i++) {
        QVariantMap data;
        data.insert("Tag",(_atagger->tagTypeVector->at(i)).tag);
        data.insert("Description",(_atagger->tagTypeVector->at(i)).description);
        data.insert("id",_atagger->tagTypeVector->count());
        data.insert("foreground_color",(_atagger->tagTypeVector->at(i)).fgcolor);
        data.insert("background_color",(_atagger->tagTypeVector->at(i)).bgcolor);
        data.insert("font",(_atagger->tagTypeVector->at(i)).font);
        data.insert("underline",(_atagger->tagTypeVector->at(i)).underline);
        data.insert("bold",(_atagger->tagTypeVector->at(i)).bold);
        data.insert("italic",(_atagger->tagTypeVector->at(i)).italic);

        tagtypeset << data;
    }
    tagtypedata.insert("TagSet",tagtypeset);

    /** Convert Tag to JSON **/

    QVariantMap tagdata;
    tagdata.insert("file",_atagger->textFile);
    tagdata.insert("TagTypeFile",_atagger->tagtypeFile);
    QVariantList tagset;
    for(int i=0; i<_atagger->tagVector->count(); i++) {
        QVariantMap data;
        data.insert("type",(_atagger->tagVector->at(i)).type);
        data.insert("pos",(_atagger->tagVector->at(i)).pos);
        data.insert("length",(_atagger->tagVector->at(i)).length);
        data.insert("source",(_atagger->tagVector->at(i)).source);
        tagset << data;
    }
    tagdata.insert("TagArray",tagset);

    /** Save to Default Destination **/
}

void AMTMainWindow::saveas() {

}

void AMTMainWindow::tagadd() {
    AddTagView * atv = new AddTagView(this);
    atv->show();
}

void AMTMainWindow::tagremove() {

}

void AMTMainWindow::tagtypeadd() {
    AddTagTypeView * attv = new AddTagTypeView(this);
    attv->show();
}

void AMTMainWindow::tagtyperemove() {

}

void AMTMainWindow::tag(char * tagValue) {

}

void AMTMainWindow::untag() {

}

void AMTMainWindow::addtagtype() {

}

void AMTMainWindow::about() {

}

void AMTMainWindow::aboutQt() {

}

void AMTMainWindow::createActions()
{
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveasAct = new QAction(tr("&SaveAs"), this);
    saveasAct->setShortcuts(QKeySequence::SaveAs);
    saveasAct->setStatusTip(tr("Print the document"));
    connect(saveasAct, SIGNAL(triggered()), this, SLOT(saveas()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    tagaddAct = new QAction(tr("&Tag Add"), this);
    //tagaddAct->setShortcuts(QKeySequence::);
    tagaddAct->setStatusTip(tr("Add a Tag"));
    connect(tagaddAct, SIGNAL(triggered()), this, SLOT(tagadd()));

    tagremoveAct = new QAction(tr("&Tag Remove"), this);
    tagremoveAct->setStatusTip(tr("Remove a Tag"));
    connect(tagremoveAct, SIGNAL(triggered()), this, SLOT(tagremove()));

    tagtypeaddAct = new QAction(tr("&TagType Add"), this);
    tagtypeaddAct->setStatusTip(tr("Add a TagType"));
    connect(tagtypeaddAct, SIGNAL(triggered()), this, SLOT(tagtypeadd()));

    tagtyperemoveAct = new QAction(tr("&TagType Remove"), this);
    tagtyperemoveAct->setStatusTip(tr("Remove a TagType"));
    connect(tagtyperemoveAct, SIGNAL(triggered()), this, SLOT(tagtyperemove()));

    tagAct = new QAction(tr("Tag"), this);
    //tagAct->setShortcuts(QKeySequence::Cut);
    tagAct->setStatusTip(tr("Tag selected word"));
    connect(tagAct, SIGNAL(triggered()), this, SLOT(tag()));

    untagAct = new QAction(tr("Untag"), this);
    //untagAct->setShortcuts(QKeySequence::Copy);
    untagAct->setStatusTip(tr("Untag selected word"));
    connect(untagAct, SIGNAL(triggered()), this, SLOT(untag()));

    addtagAct = new QAction(tr("&Add TagType"), this);
    //addtagAct->setShortcuts(QKeySequence::Paste);
    addtagAct->setStatusTip(tr("Add a TagType"));
    connect(addtagAct, SIGNAL(triggered()), this, SLOT(addtagtype()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(aboutQtAct, SIGNAL(triggered()), this, SLOT(aboutQt()));
}

void AMTMainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveasAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    tagMenu = menuBar()->addMenu(tr("&Tag"));
    tagMenu->addAction(tagaddAct);
    tagMenu->addAction(tagremoveAct);
    tagMenu->addSeparator();

    tagtypeMenu = menuBar()->addMenu(tr("&TagType"));
    tagtypeMenu->addAction(tagtypeaddAct);
    tagtypeMenu->addAction(tagtyperemoveAct);
    tagtypeMenu->addSeparator();

    viewMenu = menuBar()->addMenu(tr("&View"));

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void AMTMainWindow::fillTreeWidget() {
    QTreeWidgetItem * tags = new QTreeWidgetItem(tagDescription);
    tags->setText(0,tr("Tags"));
    for(int i=0; i < _atagger->tagVector->count(); i++) {
        QTreeWidgetItem * tag = new QTreeWidgetItem(tags);
        //char * _i = itoa(i,_i,10);
        tag->setText(0,tr("Tag "));
        QTreeWidgetItem * type = new QTreeWidgetItem(tag);
        type->setText(0,(_atagger->tagVector->at(i)).type);
        QTreeWidgetItem * source = new QTreeWidgetItem(tag);
        QString src;
        if((_atagger->tagVector->at(i)).source == user) {
            src = "user";
        }
        else {
            src = "sarf";
        }
        source->setText(0,src);
    }
}

AMTMainWindow::~AMTMainWindow() {

}

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForTr( QTextCodec::codecForName( "UTF-8" ) );
    QTextCodec::setCodecForCStrings( QTextCodec::codecForName( "UTF-8" ) );

    _atagger=new ATagger();

    QApplication a(argc, argv);
    AMTMainWindow w;
    w.show();

    return a.exec();
}

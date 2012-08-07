#include <QtGui/QApplication>
#include "amtmainwindow.h"
//#include "ui_amtmainwindow.h"
#include <qjson/parser.h>

#include <QContextMenuEvent>
#include <QTextCodec>
#include <QTextStream>
#include <QMessageBox>
#include<QDockWidget>

AMTMainWindow::AMTMainWindow(QWidget *parent) :
    QMainWindow(parent),
    //ui(new Ui::AMTMainWindow),
    browseFileDlg(NULL)
{
    //ui->setupUi(this);

    createActions();
    createMenus();

    QDockWidget *dock = new QDockWidget(tr("Text View"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    txtBrwsr = new QTextBrowser();
    dock->setWidget(txtBrwsr);
    addDockWidget(Qt::RightDockWidgetArea, dock);
//    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());

    dock = new QDockWidget(tr("Tag View"), this);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    tagDescription = new QTreeView (dock);

    dock->setWidget(tagDescription );

//    tagDescription ->setWidgetResizable(true);

    //addDockWidget(Qt::BottomDockWidgetArea, dock);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());

    setWindowTitle(tr("Arabic Morphological Tagger"));
}

void AMTMainWindow::contextMenuEvent(QContextMenuEvent *event)
 {
     QMenu menu(this);
     menu.addAction(cutAct);
     menu.addAction(copyAct);
     menu.addAction(pasteAct);
     menu.exec(event->globalPos());
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

        QFile ITfile(fileName+".tag");
        if (!ITfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::about(this, tr("Input Tag File"),
                         tr("The <b>Tag File</b> can't be opened!"));
        }

        QByteArray Tags = ITfile.readAll();

        startTaggingText(text);
        process(Tags);
        finishTaggingText();
    }
}

void AMTMainWindow::startTaggingText(QString & text){
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
}

void AMTMainWindow::process(QByteArray & json) {

    QJson::Parser parser;
    bool ok;

    QVariantMap result = parser.parse (json,&ok).toMap();

    if (!ok) {
        QMessageBox::about(this, tr("Input Tag File"),
                     tr("The <b>Tag File</b> has a wrong format"));
    }

    foreach(QVariant tag, result["TagArray"].toList()) {

        QString desc = "";
        int legendid;
        QString name = "";
        QString foreground_color = "";
        QString background_color = "";
        QString font = "";

        QVariantMap tagElements = tag.toMap();
        int start = tagElements["pos"].toInt();
        int length = tagElements["length"].toInt();
        QString tagtype = tagElements["type"].toString();

        foreach(QVariant type, result["TagSet"].toList()) {

            QVariantMap typeElements = type.toMap();
            if(QString::compare(typeElements["Tag"].toString(), tagtype, Qt::CaseInsensitive) == 0) {
                desc = typeElements["Description"].toString();
                legendid = typeElements["Legend"].toInt();

                foreach(QVariant legend, result["Legend"].toList()) {

                    QVariantMap legendElements = legend.toMap();
                    if(legendElements["id"].toInt() == legendid) {
                        name = legendElements["Name"].toString();
                        foreground_color = legendElements["foreground_color"].toString();
                        background_color = legendElements["background_color"].toString();
                        font = legendElements["font"].toString();

                        QColor fcolor(foreground_color);
                        QColor bcolor(background_color);
                        tagWord(start,length,fcolor,bcolor);
                        break;
                    }
                }
                break;
            }
        }
    }
}

void AMTMainWindow::tagWord(int start, int length, QColor fcolor, QColor  bcolor){
    if (this==NULL)
        return;
    QTextBrowser * taggedBox= txtBrwsr;
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
}

void AMTMainWindow::finishTaggingText(){
    if (this==NULL)
        return;
    QTextBrowser * taggedBox= txtBrwsr;
    QTextCursor c=taggedBox->textCursor();
    c.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
    taggedBox->setTextCursor(c);
}

void AMTMainWindow::save()
{

}

void AMTMainWindow::saveas()
{

}

void AMTMainWindow::cut()
{

}

void AMTMainWindow::copy()
{

}

void AMTMainWindow::paste()
{

}

void AMTMainWindow::about()
{

}

void AMTMainWindow::aboutQt()
{

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

    cutAct = new QAction(tr("Cu&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                             "clipboard"));
    connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));

    copyAct = new QAction(tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                              "clipboard"));
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

    pasteAct = new QAction(tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                               "selection"));
    connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));

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

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    editMenu->addSeparator();

    viewMenu = menuBar()->addMenu(tr("&View"));

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);


}

AMTMainWindow::~AMTMainWindow()
{
    //delete ui;
}

/*
int main(int argc, char *argv[])
{
    QTextCodec::setCodecForTr( QTextCodec::codecForName( "UTF-8" ) );
    QTextCodec::setCodecForCStrings( QTextCodec::codecForName( "UTF-8" ) );

    QApplication a(argc, argv);
    AMTMainWindow w;
    w.show();

    return a.exec();
}
*/

int AMT_main(int argc, char *argv[])
{
    QTextCodec::setCodecForTr( QTextCodec::codecForName( "UTF-8" ) );
    QTextCodec::setCodecForCStrings( QTextCodec::codecForName( "UTF-8" ) );

    QApplication a(argc, argv);
    AMTMainWindow w;
    w.show();

    return a.exec();
}

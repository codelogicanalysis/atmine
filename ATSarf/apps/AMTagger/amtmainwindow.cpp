#include <QtGui/QApplication>
#include "amtmainwindow.h"
#include <qjson/parser.h>

#include <QContextMenuEvent>
#include <QTextCodec>
#include <QTextStream>
#include <QDockWidget>
#include <QList>
#include <QMessageBox>
#include <QTextBlock>
#include <QtAlgorithms>
//#include <QtGui>
#include <math.h>
#include "graphviz/cgraph.h"
#include "graphviz/gvc.h"
#include <sstream>
#include <string.h>

bool parentCheck;
class SarfTag;

AMTMainWindow::AMTMainWindow(QWidget *parent) :
    QMainWindow(parent),
    browseFileDlg(NULL)
{
    resize(800,600);

    // Used to check for parent Widget between windows (EditTagTypeView and AMTMainWindow)
    parentCheck = false;

    // Boolean to check any change in tags
    dirty = false;

    createActions();
    createMenus();
    //createDockWindows();
    /** Create Hello Dock **/
    QLabel *label = new QLabel(this);
    label->setText("This is Morphology-based Automated Tagger for Arabic (MATAr)\n\n"
                   "Courtesy of the NLP/CL research team at AUB.\n\n"
                   "Use New in the File Menu to create a new project.\n"
                   "Use Open in the File Menu to open an existing project\n");
    label->setAlignment(Qt::AlignCenter);
    setCentralWidget(label);

    setWindowTitle(tr("Arabic Morphological Tagger"));

    /** Initialize Sarf **/
    theSarf = new Sarf();
    bool all_set = theSarf->start(&output_str, &error_str, this);
    if(!all_set) {
        QMessageBox::warning(this,"Warning","Can't set up the Sarf Tool");
        return;QMessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Open);
        msgBox.button(QMessageBox::Save)->setText("New");
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        switch (ret) {
            case QMessageBox::Save:
                _new();
            case QMessageBox::Open:
                open();
            default:
                _new();
                break;
        }
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
    //hbox->addWidget(txtBrwsr);
    txtBrwsr->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(txtBrwsr,SIGNAL(customContextMenuRequested(const QPoint&)), this,SLOT(showContextMenu(const QPoint&)));

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->addWidget(sa1);
    vbox->addWidget(txtBrwsr);

    QScrollArea *sa = new QScrollArea(dock);
    sa->setLayout(vbox);

    dock->setWidget(sa);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    paneMenu->addAction(dock->toggleViewAction());
    dock->setMinimumWidth(300);

    if(open) {
        lineEditTFName->setText(_atagger->textFile);
    }

    dock = new QDockWidget(tr("Tags"), this);
    tagDescription = new QTreeWidget(dock);
    tagDescription->setColumnCount(5);
    QStringList columns;
    columns << "Word" << "Tag" << "Source" << "Start" << "Length";
    QTreeWidgetItem* item=new QTreeWidgetItem(columns);
    connect(tagDescription,SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(itemSelectionChanged(QTreeWidgetItem*,int)));
    tagDescription->setHeaderItem(item);
    dock->setWidget(tagDescription);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    paneMenu->addAction(dock->toggleViewAction());

    //vbox->addWidget(tagDescription);

    dock = new QDockWidget(tr("Tag/Tagtype"), this);
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
    paneMenu->addAction(dock->toggleViewAction());
    dock->setMinimumWidth(300);

    if(open) {
        lineEditTFName->setText(_atagger->tagtypeFile);
    }

    dock = new QDockWidget(tr("Match"), this);
    graphics = new QGraphicsView(this);
    scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setSceneRect(-150, -150, 300, 300);
    graphics->setScene(scene);
    graphics->setCacheMode(QGraphicsView::CacheBackground);
    graphics->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    graphics->setRenderHint(QPainter::Antialiasing);
    graphics->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    graphics->scale(qreal(2), qreal(2));
    //graphics->setMinimumSize(400, 400);
    dock->setWidget(graphics);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    paneMenu->addAction(dock->toggleViewAction());
}

void AMTMainWindow::wheelEvent(QWheelEvent *event) {
    if(graphics->hasFocus()) {
        scaleView(pow((double)2, event->delta() / 240.0));
    }
}

void AMTMainWindow::scaleView(qreal scaleFactor)
{
    qreal factor = graphics->transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    graphics->scale(scaleFactor, scaleFactor);
}

void AMTMainWindow::showContextMenu(const QPoint &pt) {

    int pos;
    int length;
    if(txtBrwsr->textCursor().selectedText().isEmpty()) {
        if(!(_atagger->isTagMBF)) {
            QMessageBox::warning(this,"Warning","No text selected");
            return;
        }
        myTC = txtBrwsr->cursorForPosition(pt);
        myTC.select(QTextCursor::WordUnderCursor);
        QString word = myTC.selectedText();

        if(word.isEmpty()) {
            mTags->setEnabled(false);
            umTags->setEnabled(false);
        }
        else {
            umTags->setEnabled(true);
        }
    }
    else {
        myTC = txtBrwsr->textCursor();
        umTags->setEnabled(true);
    }
    pos = myTC.selectionStart();
    length = myTC.selectionEnd() - myTC.selectionStart();

    QStringList tagtypes;
    if(_atagger->isTagMBF) {
        for(int i=0; i < _atagger->tagVector.count(); i++) {
            const Tag * t = (Tag*)(&(_atagger->tagVector.at(i)));
            if(t->pos == pos) {
                tagtypes << t->type;
            }
        }
    }
    else {
        for(int i=0; i < _atagger->simulationVector.count(); i++) {
            const MERFTag * t = (MERFTag*)(&(_atagger->simulationVector.at(i)));
            if(t->pos == pos) {
                tagtypes << t->type;
            }
        }
    }
    signalMapper = new QSignalMapper(this);
    QMenu * menu = new QMenu();
    QMenu * mTags;
    mTags = menu->addMenu(tr("Tag"));
    if(_atagger->isTagMBF) {
        for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
            if(!(tagtypes.contains(_atagger->tagTypeVector->at(i)->tag))) {
                QAction * taginstance;
                taginstance = new QAction((_atagger->tagTypeVector->at(i))->tag,this);
                signalMapper->setMapping(taginstance, (_atagger->tagTypeVector->at(i))->tag);
                connect(taginstance, SIGNAL(triggered()), signalMapper, SLOT(map()));
                mTags->addAction(taginstance);
            }
        }
    }
    else {
        for(int i=0; i<_atagger->msfVector->count(); i++) {
            if(!(tagtypes.contains(_atagger->msfVector->at(i)->name))) {
                QAction * taginstance;
                taginstance = new QAction((_atagger->msfVector->at(i))->name,this);
                signalMapper->setMapping(taginstance, (_atagger->msfVector->at(i))->name);
                connect(taginstance, SIGNAL(triggered()), signalMapper, SLOT(map()));
                mTags->addAction(taginstance);
            }
        }
    }
    connect(signalMapper, SIGNAL(mapped(const QString &)), this, SLOT(tag(QString)));

    signalMapperU = new QSignalMapper(this);
    QMenu * muTags;
    muTags = menu->addMenu(tr("Untag"));
    if(_atagger->isTagMBF) {
        for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
            if(tagtypes.contains(_atagger->tagTypeVector->at(i)->tag)) {
                QAction * taginstance;
                taginstance = new QAction((_atagger->tagTypeVector->at(i))->tag,this);
                signalMapperU->setMapping(taginstance, (_atagger->tagTypeVector->at(i))->tag);
                connect(taginstance, SIGNAL(triggered()), signalMapperU, SLOT(map()));
                muTags->addAction(taginstance);
            }
        }
    }
    else {
        for(int i=0; i<_atagger->msfVector->count(); i++) {
            if(tagtypes.contains(_atagger->msfVector->at(i)->name)) {
                QAction * taginstance;
                taginstance = new QAction((_atagger->msfVector->at(i))->name,this);
                signalMapperU->setMapping(taginstance, (_atagger->msfVector->at(i))->name);
                connect(taginstance, SIGNAL(triggered()), signalMapperU, SLOT(map()));
                muTags->addAction(taginstance);
            }
        }
    }
    connect(signalMapperU, SIGNAL(mapped(const QString &)), this, SLOT(untag(QString)));
    //menu->addAction(untagMAct);
    menu->addSeparator();
    menu->addAction(addtagAct);

    menu->exec(txtBrwsr->mapToGlobal(pt));
    delete menu;
}

void AMTMainWindow::open() {

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

    /** Initialize Tagger instance again **/

    _atagger = NULL;
    _atagger = new ATagger();

    _atagger->tagFile = fileName;

    QByteArray Tags = ITfile.readAll();
    ITfile.close();

    QJson::Parser parser;
    bool ok;

    QVariantMap result = parser.parse(Tags,&ok).toMap();

    if (!ok) {
        QMessageBox::about(this, tr("Input Tag File"),
                     tr("The <b>Tag File</b> has a wrong format"));
        return;
    }

    /** Read text file path **/

    QStringList dirList = _atagger->tagFile.split('/');
    dirList.removeLast();
    QString dir = dirList.join("/");
    dir.append('/');

    _atagger->textFile = result["file"].toString();

    QString textPath = dir;
    textPath.append(_atagger->textFile);
    QFile Ifile(textPath);
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

    QString absoluteTextPath = QDir(_atagger->textFile).absolutePath();
    lineEditTFName->setText(absoluteTextPath);
    _atagger->text = text;
    Ifile.close();

    startTaggingText(text);
    process(Tags);
    finishTaggingText();

    if(!_atagger->text.isEmpty()) {
        sarfAct->setEnabled(true);
        simulatorAct->setEnabled(true);
    }
    edittagtypesAct->setEnabled(true);
    sarftagsAct->setEnabled(true);
    editMSFAct->setEnabled(true);
    tagremoveAct->setEnabled(true);
    mTags->setEnabled(true);
    umTags->setEnabled(true);
    viewMBFTagAct->setEnabled(true);
    viewMSFTagAct->setEnabled(true);
    saveAct->setEnabled(true);
    saveasAct->setEnabled(true);
    diffAct->setEnabled(true);
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
        check = _atagger->insertTag(tagtype,start,length,source,original);
    }

    if(_atagger->tagVector.at(0).source == user) {
        _atagger->isSarf = false;
    }
    else {
        _atagger->isSarf = true;
    }
    _atagger->isTagMBF = true;

    /** Read simulation Tags if found **/

    if(!(result.value("simulationTags").isNull())) {
        foreach(QVariant merfTag, result["simulationTags"].toList()) {

            QVariantMap merfTagElements = merfTag.toMap();
            QString type = merfTagElements.value("type").toString();
            int pos = merfTagElements.value("pos").toInt();
            int length = merfTagElements.value("length").toInt();
            MERFTag merftag(type,pos,length);
            merftag.tags = new QVector<Tag*>();

            foreach(QVariant tagData, merfTagElements["tags"].toList()) {

                QVariantMap tagElements = tagData.toMap();
                QString tagType = tagElements.value("type").toString();
                int tagpos = tagElements.value("pos").toInt();
                int taglength = tagElements.value("length").toInt();
                Tag *tag = new Tag(tagType,tagpos,taglength,sarf);

                if(!(tagElements.value("tagType").isNull())) {
                    foreach(QVariant tagTypeData, tagElements["tagType"].toList()) {
                        QString matchInfo = tagTypeData.toString();
                        tag->tType.append(matchInfo);
                    }
                }
                merftag.tags->append(tag);
            }

            _atagger->simulationVector.append(merftag);
        }
        _atagger->isTagMBF = false;
    }

    /** Read the TagType file and store it **/

    QStringList dirList = _atagger->tagFile.split('/');
    dirList.removeLast();
    QString dir = dirList.join("/");
    dir.append('/');

    QString tagtypePath = dir + tagtypeFile;

    QFile ITfile(tagtypePath);
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

    _atagger->tagtypeFile = tagtypeFile;
    QString absoluteTTPath = QDir(_atagger->tagtypeFile).absolutePath();
    lineEditTTFName->setText(absoluteTTPath);
    process_TagTypes(tagtypedata);


    setWindowTitle("MERF: " + _atagger->tagFile);
    /** Apply Tags on Input Text **/

    if(_atagger->simulationVector.isEmpty()) {
        applyTags();
    }
    else {
        applyTags(1);
    }

    /** Add Tags to tagDescription Tree **/

    if(_atagger->simulationVector.isEmpty()) {
        fillTreeWidget(user);
    }
    else {
        fillTreeWidget(sarf,1);
    }
    createTagMenu();
    createUntagMenu();
}

bool AMTMainWindow::readMSF(MSFormula* formula, QVariant data, MSF *parent) {
    /** Common variables in MSFs **/
    QString name;
    QString init;
    //QString after;
    QString actions;
    QString returns;
    QString parentName;
    QString type;

    QVariantMap msfData = data.toMap();
    name = msfData.value("name").toString();
    init = msfData.value("init").toString();
    //after = msfData.value("after").toString();
    actions = msfData.value("actions").toString();
    returns = msfData.value("returns").toString();
    parentName = msfData.value("parent").toString();
    type = msfData.value("type").toString();

    if(type == "mbf") {
        /** This is MBF **/
        QString bf = msfData.value("MBF").toString();
        bool isF = msfData.value("isFormula").toBool();

        /** initialize MBF **/
        MBF* mbf = new MBF(name,parent,bf,isF);
        mbf->init = init;
        //mbf->after = after;
        mbf->actions = actions;
        mbf->returns = returns;
        formula->map.insert(name, mbf);

        /** Check parent type and add accordingly **/
        if(parent->isFormula()) {
            MSFormula* prnt = (MSFormula*)parent;
            prnt->addMSF(mbf);
        }
        else if(parent->isUnary()) {
            UNARYF* prnt = (UNARYF*)parent;
            prnt->setMSF(mbf);
        }
        else if(parent->isBinary()) {
            BINARYF* prnt = (BINARYF*)parent;
            if(prnt->leftMSF == NULL) {
                prnt->setLeftMSF(mbf);
            }
            else {
                prnt->setRightMSF(mbf);
            }
        }
        else if(parent->isSequential()) {
            SequentialF* prnt = (SequentialF*)parent;
            prnt->addMSF(mbf);
        }
        else {
            return false;
        }

        return true;
    }
    else if(type == "unary") {
        /** This is a UNARY formula **/
        QString operation = msfData.value("op").toString();
        Operation op;
        if(operation == "?") {
            op = KUESTION;
        }
        else if(operation == "*") {
            op = STAR;
        }
        else if(operation == "+") {
            op = PLUS;
        }
        else if(operation.contains('^')) {
            op = UPTO;
        }
        else {
            return false;
        }
        int limit = -1;
        if(operation.contains('^')) {
            bool ok;
            limit = operation.mid(1).toInt(&ok);
            if(!ok) {
                return false;
            }
        }

        /** Initialize a UNARYF **/
        UNARYF* uf = new UNARYF(name,parent,op,limit);
        uf->init = init;
        //uf->after = after;
        uf->actions = actions;
        uf->returns = returns;
        formula->map.insert(name,uf);

        /** Check parent type and add accordingly **/
        if(parent->isFormula()) {
            MSFormula* prnt = (MSFormula*)parent;
            prnt->addMSF(uf);
        }
        else if(parent->isUnary()) {
            UNARYF* prnt = (UNARYF*)parent;
            prnt->setMSF(uf);
        }
        else if(parent->isBinary()) {
            BINARYF* prnt = (BINARYF*)parent;
            if(prnt->leftMSF == NULL) {
                prnt->setLeftMSF(uf);
            }
            else {
                prnt->setRightMSF(uf);
            }
        }
        else if(parent->isSequential()) {
            SequentialF* prnt = (SequentialF*)parent;
            prnt->addMSF(uf);
        }
        else {
            return false;
        }

        /** Proceed with child MSF **/
        return readMSF(formula,msfData.value("MSF"),uf);
    }
    else if(type == "binary") {
        /** This is a BINARY formula **/
        QString operation = msfData.value("op").toString();
        Operation op;
        if(operation == "&") {
            op = AND;
        }
        else if(operation == "|") {
            op = OR;
        }
        else {
            return false;
        }

        /** Initialize BINARYF **/
        BINARYF* bif = new BINARYF(name,parent,op);
        bif->init = init;
        //bif->after = after;
        bif->actions = actions;
        bif->returns = returns;
        formula->map.insert(name, bif);

        /** Check parent type and add accordingly **/
        if(parent->isFormula()) {
            MSFormula* prnt = (MSFormula*)parent;
            prnt->addMSF(bif);
        }
        else if(parent->isUnary()) {
            UNARYF* prnt = (UNARYF*)parent;
            prnt->setMSF(bif);
        }
        else if(parent->isBinary()) {
            BINARYF* prnt = (BINARYF*)parent;
            if(prnt->leftMSF == NULL) {
                prnt->setLeftMSF(bif);
            }
            else {
                prnt->setRightMSF(bif);
            }
        }
        else if(parent->isSequential()) {
            SequentialF* prnt = (SequentialF*)parent;
            prnt->addMSF(bif);
        }
        else {
            return false;
        }

        /** Proceed with child MSFs **/
        bool first = readMSF(formula,msfData.value("leftMSF"),bif);
        bool second = readMSF(formula,msfData.value("rightMSF"),bif);
        if(first && second) {
            return true;
        }
        else {
            return false;
        }
    }
    else if(type == "sequential") {
        /** This is a sequential formula **/
        /** Initialize a SequentialF **/
        SequentialF* sf = new SequentialF(name,parent);
        sf->init = init;
        //sf->after = after;
        sf->actions = actions;
        sf->returns = returns;
        formula->map.insert(name, sf);

        /** Check parent type and add accordingly **/
        if(parent->isFormula()) {
            MSFormula* prnt = (MSFormula*)parent;
            prnt->addMSF(sf);
        }
        else if(parent->isUnary()) {
            UNARYF* prnt = (UNARYF*)parent;
            prnt->setMSF(sf);
        }
        else if(parent->isBinary()) {
            BINARYF* prnt = (BINARYF*)parent;
            if(prnt->leftMSF == NULL) {
                prnt->setLeftMSF(sf);
            }
            else {
                prnt->setRightMSF(sf);
            }
        }
        else if(parent->isSequential()) {
            SequentialF* prnt = (SequentialF*)parent;
            prnt->addMSF(sf);
        }
        else {
            return false;
        }

        /** Proceed with children **/
        foreach(QVariant seqMSFData, msfData.value("MSFs").toList()) {
            if(!(readMSF(formula,seqMSFData,sf))) {
                return false;
            }
        }

        return true;
    }
    else {
        return false;
    }
}

void AMTMainWindow::process_TagTypes(QByteArray &tagtypedata) {
    QJson::Parser parser;
    bool ok;

    QVariantMap result = parser.parse (tagtypedata,&ok).toMap();

    if (!ok) {
        QMessageBox::about(this, tr("Input Tag File"),
                     tr("The <b>Tag File</b> has a wrong format"));
    }

    if(result.value("TagTypeSet").toList().at(0).toMap().value("Features").isNull()) {
        for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
            const TagType& tt = *(_atagger->tagTypeVector->at(i));
            for(int j=0; j<_atagger->tagVector.count(); j++) {
                const Tag & tag = (_atagger->tagVector.at(j));
                if(tt.tag == tag.type) {
                    int start = tag.pos;
                    int length = tag.length;
                    tagWord(start,length,QColor("black"),QColor("white"),12,false,false,false);
                    _atagger->tagVector.remove(i);
                }
            }
        }
        _atagger->tagTypeVector->clear();
        fillTreeWidget(user);
    }

    foreach(QVariant type, result["TagTypeSet"].toList()) {
        QString tag;
        QString desc;
        int id;
        QString foreground_color;
        QString background_color;
        int font;
        bool underline = false;
        bool bold;
        bool italic;

        QVariantMap typeElements = type.toMap();

        tag = typeElements["Tag"].toString();
        desc = typeElements["Description"].toString();
        id = typeElements["Legend"].toInt();
        foreground_color = typeElements["foreground_color"].toString();
        background_color = typeElements["background_color"].toString();
        font = typeElements["font"].toInt();
        //underline = typeElements["underline"].toBool();
        bold = typeElements["bold"].toBool();
        italic = typeElements["italic"].toBool();

        if(!typeElements.value("Features").isNull()) {

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
                        quad.fourth = st.value("Relation").toString();
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
                        quad.fourth = st.value("Relation").toString();
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
                        quad.fourth = st.value("Relation").toString();
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

            _atagger->isSarf = true;
            _atagger->insertSarfTagType(tag,tags,desc,id,foreground_color,background_color,font,underline,bold,italic,sarf,original);
        }
        else {
            _atagger->isSarf = false;
            _atagger->insertTagType(tag,desc,id,foreground_color,background_color,font,underline,bold,italic,user,original);
        }
    }

    if(!(result.value("MSFs").isNull())) {
        /** The tagtype file contains MSFs **/

        foreach(QVariant msfsData, result.value("MSFs").toList()) {

            /** List of variables for each MSFormula **/
            QString bgcolor;
            QString fgcolor;
            QString name;
            QString init;
            //QString after;
            QString actions;
            QString includes;
            QString members;
            QString description;
            int i;
            int usedCount;

            /** This is an MSFormula **/
            QVariantMap msformulaData = msfsData.toMap();

            name = msformulaData.value("name").toString();
            init = msformulaData.value("init").toString();
            //after = msformulaData.value("after").toString();
            actions = msformulaData.value("actions").toString();
            includes = msformulaData.value("includes").toString();
            members = msformulaData.value("members").toString();
            description = msformulaData.value("description").toString();
            fgcolor = msformulaData.value("fgcolor").toString();
            bgcolor = msformulaData.value("bgcolor").toString();
            i = msformulaData.value("i").toInt();
            usedCount = msformulaData.value("usedCount").toInt();

            MSFormula* msf = new MSFormula(name, NULL);
            msf->includes = includes;
            msf->members = members;
            msf->fgcolor = fgcolor;
            msf->bgcolor = bgcolor;
            msf->description = description;
            msf->i = i;
            msf->usedCount = usedCount;
            _atagger->msfVector->append(msf);

            /** Get MSFormula MSFs **/
            foreach(QVariant msfData, msformulaData.value("MSFs").toList()) {
                readMSF(msf, msfData, msf);
            }
        }
    }
}

bool compare(const Tag &tag1, const Tag &tag2) {
    if(tag1.pos != tag2.pos) {
        return tag1.pos < tag2.pos;
    }
    else {
        return tag1.type < tag2.type;
    }
}

void AMTMainWindow::applyTags(int basic) {

    startTaggingText(_atagger->text);
    if(basic == 0) {
        qSort(_atagger->tagVector.begin(), _atagger->tagVector.end(), compare);

        for(int i =0; i< _atagger->tagVector.count(); i++) {
            const Tag * pt = NULL;
            if(i>0) {
                pt = (Tag*)(&(_atagger->tagVector.at(i-1)));
            }
            const Tag * t = (Tag*)(&(_atagger->tagVector.at(i)));

            if(pt != NULL && pt->pos == t->pos) {
                continue;
            }
            const Tag * nt = NULL;
            if(i<(_atagger->tagVector.count()-1)) {
                nt = (Tag*)(&(_atagger->tagVector.at(i+1)));
            }

            for(int j=0; j< _atagger->tagTypeVector->count(); j++) {
                const TagType * tt = (TagType*)(_atagger->tagTypeVector->at(j));

                if(t->type == tt->tag) {
                    int start = t->pos;
                    int length = t->length;
                    QColor bgcolor(tt->bgcolor);
                    QColor fgcolor(tt->fgcolor);
                    int font = tt->font;
                    //bool underline = (_atagger->tagTypeVector->at(j))->underline;
                    bool underline = false;
                    if(nt!=NULL && nt->pos == start) {
                        underline = true;
                    }
                    bool bold = tt->bold;
                    bool italic = tt->italic;
                    tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold);
                    break;
                }
            }
        }

        fillTreeWidget(user);
        createTagMenu();
        createUntagMenu();
    }
    else {
        for(int i=0; i<_atagger->simulationVector.count(); i++) {
            const MERFTag tag = _atagger->simulationVector.at(i);
            for(int j=0; j< _atagger->msfVector->count(); j++) {
                const MSFormula* msf = _atagger->msfVector->at(j);
                if(msf->name == tag.type) {
                    tagWord(tag.pos,tag.length,msf->fgcolor,msf->bgcolor,12,false,false,false);
                    break;
                }
            }
        }
        fillTreeWidget(sarf,1);
    }
}

void AMTMainWindow::tagWord(int start, int length, QColor fcolor, QColor  bcolor,int font, bool underline, bool italic, bool bold){
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
            setWindowTitle("MERF: " + _atagger->tagFile);
        }
    }

    if(_atagger->textFile.isEmpty() || _atagger->tagtypeFile.isEmpty()) {
        QMessageBox::warning(this,"Warning","Couldn't save since text file or tag type file is not specified!");
        return;
    }

    if(_atagger->tagVector.isEmpty()) {
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
    dirty = true;
    QTextCursor cursor = txtBrwsr->textCursor();
    int start = cursor.selectionStart();
    int length = cursor.selectionEnd() - cursor.selectionStart();

    qSort(_atagger->tagVector.begin(), _atagger->tagVector.end(), compare);
    for(int i=0; i < _atagger->tagVector.count(); i++) {
        const Tag * t = (Tag*)(&(_atagger->tagVector.at(i)));
        const Tag * nt = NULL;
        if(i < (_atagger->tagVector.count()-1)) {
            nt = (Tag*)(&(_atagger->tagVector.at(i+1)));
        }
        if(t->pos == start) {
            if(nt != NULL && nt->pos == start) {
                for(int j=0; j< _atagger->tagTypeVector->count(); j++) {
                    const TagType * tt = (TagType*)(_atagger->tagTypeVector->at(j));

                    if(nt->type == tt->tag) {
                        QColor bgcolor(tt->bgcolor);
                        QColor fgcolor(tt->fgcolor);
                        int font = tt->font;
                        //bool underline = (_atagger->tagTypeVector->at(j))->underline;
                        bool underline = false;
                        const Tag* nnt = NULL;
                        if(i < (_atagger->tagVector.count()-2)) {
                            nnt = (Tag*)(&(_atagger->tagVector.at(i+2)));
                        }
                        if(nnt!=NULL && nnt->pos == start) {
                            underline = true;
                        }
                        bool bold = tt->bold;
                        bool italic = tt->italic;
                        tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold);
                        break;
                    }
                }
            }
            else {
                tagWord(start,length,QColor("black"),QColor("white"),9,false,false,false);
            }
            _atagger->tagVector.remove(i);
            cursor.clearSelection();
            fillTreeWidget(user);
            break;
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
                setWindowTitle("MERF: " + _atagger->tagFile);
            }
        }

        dirty = true;
        QTextCursor cursor = myTC;
        int start = cursor.selectionStart();
        int length = cursor.selectionEnd() - cursor.selectionStart();
        if(_atagger->isTagMBF) {
            /// MBF based tags
            _atagger->insertTag(tagValue, start, length, user, original);
            //applyTags(0);
            qSort(_atagger->tagVector.begin(), _atagger->tagVector.end(), compare);
            for(int i =0; i< _atagger->tagVector.count(); i++) {
                const Tag * t = (Tag*)(&(_atagger->tagVector.at(i)));
                if(t->pos != start) {
                    continue;
                }

                const Tag * nt = NULL;
                if(i<(_atagger->tagVector.count()-1)) {
                    nt = (Tag*)(&(_atagger->tagVector.at(i+1)));
                }

                for(int j=0; j< _atagger->tagTypeVector->count(); j++) {
                    const TagType * tt = (TagType*)(_atagger->tagTypeVector->at(j));
                    if(tt->tag == tagValue) {
                        QColor bgcolor(tt->bgcolor);
                        QColor fgcolor(tt->fgcolor);
                        int font = tt->font;
                        //bool underline = (_atagger->tagTypeVector->at(i))->underline;
                        bool underline = false;
                        if(nt!=NULL && nt->pos == start) {
                            underline = true;
                        }
                        bool bold = tt->bold;
                        bool italic = tt->italic;

                        tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold);
                    }
                }
                cursor.clearSelection();
                fillTreeWidget(user);
                break;
            }
        }
        else {
            /// MSF based tags
        }
    }
    else {
        switch( QMessageBox::information( this, "Add Tag","No word is selected for tagging!","&Ok",0,0) ) {
            return;
        }
    }
}

void AMTMainWindow::untag(QString tagValue) {

    QTextCursor cursor = myTC;
    int start = cursor.selectionStart();
    int length = cursor.selectionEnd() - cursor.selectionStart();
    if(length <= 0) {
        return;
    }

    if(_atagger->isTagMBF) {
        /// MBF based tags
        qSort(_atagger->tagVector.begin(), _atagger->tagVector.end(), compare);
        for(int i=0; i < _atagger->tagVector.count(); i++) {
            const Tag * t = (Tag*)(&(_atagger->tagVector.at(i)));

            if(t->pos == start && t->type == tagValue) {
                int counter = 0;
                bool previous = false;
                const Tag* ppt = NULL;
                if(i > 1) {
                    ppt = (Tag*)(&(_atagger->tagVector.at(i-2)));
                    if(ppt->pos == start) {
                        counter++;
                    }
                }
                const Tag* pt = NULL;
                if(i > 0) {
                    pt = (Tag*)(&(_atagger->tagVector.at(i-1)));
                    if(pt->pos == start) {
                        counter++;
                        previous = true;
                    }
                }
                const Tag* nt = NULL;
                if(i < (_atagger->tagVector.count()-1)) {
                    nt = (Tag*)(&(_atagger->tagVector.at(i+1)));
                    if(nt->pos == start) {
                        counter++;
                    }
                }
                const Tag* nnt = NULL;
                if(i < (_atagger->tagVector.count()-2)) {
                    nnt = (Tag*)(&(_atagger->tagVector.at(i+2)));
                    if(nnt->pos == start) {
                        counter++;
                    }
                }
                if(counter == 0) {
                    tagWord(start,length,QColor("black"),QColor("white"),9,false,false,false);
                }
                else {
                    QString targetTag;
                    if(previous) {
                        targetTag = pt->type;
                    }
                    else {
                        targetTag = nt->type;
                    }
                    for(int j=0; j< _atagger->tagTypeVector->count(); j++) {
                        const TagType * tt = (TagType*)(_atagger->tagTypeVector->at(j));

                        if(tt->tag == targetTag) {
                            QColor bgcolor(tt->bgcolor);
                            QColor fgcolor(tt->fgcolor);
                            int font = tt->font;
                            //bool underline = (_atagger->tagTypeVector->at(j))->underline;
                            bool underline = false;
                            if(counter >= 2) {
                                underline = true;
                            }
                            bool bold = tt->bold;
                            bool italic = tt->italic;
                            tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold);
                            break;
                        }
                    }
                }
                _atagger->tagVector.remove(i);
                cursor.clearSelection();
                fillTreeWidget(user);
                dirty = true;
                break;
            }
        }

        /*
        for(int i =0; i< _atagger->tagVector.count(); i++) {
            const Tag * t = (Tag*)(&(_atagger->tagVector.at(i)));
            if(t->pos != start) {
                continue;
            }

            const Tag * nt = NULL;
            if(i<(_atagger->tagVector.count()-1)) {
                nt = (Tag*)(&(_atagger->tagVector.at(i+1)));
            }

            for(int j=0; j< _atagger->tagTypeVector->count(); j++) {
                const TagType * tt = (TagType*)(_atagger->tagTypeVector->at(j));
                if(tt->tag == tagValue) {
                    QColor bgcolor(tt->bgcolor);
                    QColor fgcolor(tt->fgcolor);
                    int font = tt->font;
                    //bool underline = (_atagger->tagTypeVector->at(i))->underline;
                    bool underline = false;
                    if(nt!=NULL && nt->pos == start) {
                        underline = true;
                    }
                    bool bold = tt->bold;
                    bool italic = tt->italic;

                    tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold);
                }
            }
            cursor.clearSelection();
            fillTreeWidget(user);
            break;
        }
        */
    }
    else {
        /// MSF based tags
        for(int i=0; i<_atagger->simulationVector.count(); i++) {
            if(_atagger->simulationVector.at(i).pos == start && _atagger->simulationVector.at(i).type == tagValue) {
                _atagger->simulationVector.remove(i);
                dirty = true;
                break;
            }
        }
        applyTags(1);
    }
}

void AMTMainWindow::addtagtype() {
    AddTagTypeView * attv = new AddTagTypeView();
    attv->show();

}

void AMTMainWindow::viewMBFTags() {
    if(_atagger->tagVector.isEmpty()) {
        return;
    }
    startTaggingText(_atagger->text);
    scene->clear();
    descBrwsr->clear();

    qSort(_atagger->tagVector.begin(), _atagger->tagVector.end(), compare);

    for(int i =0; i< _atagger->tagVector.count(); i++) {
        const Tag * pt = NULL;
        if(i>0) {
            pt = (Tag*)(&(_atagger->tagVector.at(i-1)));
        }
        const Tag * t = (Tag*)(&(_atagger->tagVector.at(i)));

        if(pt != NULL && pt->pos == t->pos) {
            continue;
        }
        const Tag * nt = NULL;
        if(i<(_atagger->tagVector.count()-1)) {
            nt = (Tag*)(&(_atagger->tagVector.at(i+1)));
        }

        for(int j=0; j< _atagger->tagTypeVector->count(); j++) {
            const TagType * tt = (TagType*)(_atagger->tagTypeVector->at(j));

            if(t->type == tt->tag) {
                int start = t->pos;
                int length = t->length;
                QColor bgcolor(tt->bgcolor);
                QColor fgcolor(tt->fgcolor);
                int font = tt->font;
                //bool underline = (_atagger->tagTypeVector->at(j))->underline;
                bool underline = false;
                if(nt!=NULL && nt->pos == start) {
                    underline = true;
                }
                bool bold = tt->bold;
                bool italic = tt->italic;
                tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold);
                break;
            }
        }
    }

    fillTreeWidget(user);
    createTagMenu();
    createUntagMenu();
    _atagger->isTagMBF = true;
    finishTaggingText();
}

void AMTMainWindow::viewMSFTags() {
    if(_atagger->simulationVector.isEmpty()) {
        return;
    }
    startTaggingText(_atagger->text);
    scene->clear();
    descBrwsr->clear();

    for(int i=0; i<_atagger->simulationVector.count(); i++) {
        const MERFTag tag = _atagger->simulationVector.at(i);
        for(int j=0; j< _atagger->msfVector->count(); j++) {
            const MSFormula* msf = _atagger->msfVector->at(j);
            if(msf->name == tag.type) {
                tagWord(tag.pos,tag.length,msf->fgcolor,msf->bgcolor,12,false,false,false);
                break;
            }
        }
    }
    fillTreeWidget(sarf,1);
    _atagger->isTagMBF = false;
    finishTaggingText();
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

    sarftagsAct = new QAction(tr("Morphology-based Boolean Formulae..."), this);
    sarftagsAct->setEnabled(false);
    connect(sarftagsAct, SIGNAL(triggered()), this, SLOT(customizeSarfTags()));

    sarfAct = new QAction(tr("Simulate with Sarf"), this);
    sarfAct->setEnabled(false);
    connect(sarfAct, SIGNAL(triggered()), this, SLOT(sarfTagging()));

    editMSFAct = new QAction(tr("Morphology-based Sequential Formulae..."), this);
    editMSFAct->setEnabled(false);
    connect(editMSFAct, SIGNAL(triggered()), this, SLOT(customizeMSFs()));

    simulatorAct = new QAction(tr("Simulate with Sarf"), this);
    simulatorAct->setEnabled(false);
    connect(simulatorAct, SIGNAL(triggered()), this, SLOT(runMERFSimulator()));

    diffAct = new QAction(tr("diff..."),this);
    diffAct->setEnabled(false);
    connect(diffAct, SIGNAL(triggered()), this, SLOT(difference()));

    /*
    untagMAct = new QAction(tr("Untag"), this);
    //untagAct->setShortcuts(QKeySequence::Copy);
    untagMAct->setStatusTip(tr("Untag selected word"));
    connect(untagMAct, SIGNAL(triggered()), this, SLOT(untag()));
    */

    addtagAct = new QAction(tr("&Add TagType"), this);
    //addtagAct->setShortcuts(QKeySequence::Paste);
    addtagAct->setStatusTip(tr("Add a TagType"));
    connect(addtagAct, SIGNAL(triggered()), this, SLOT(addtagtype()));

    viewMBFTagAct = new QAction(tr("&View MBF-based Tags"), this);
    viewMBFTagAct->setEnabled(false);
    connect(viewMBFTagAct, SIGNAL(triggered()), this, SLOT(viewMBFTags()));

    viewMSFTagAct = new QAction(tr("&View MSF-based Tags"), this);
    viewMSFTagAct->setEnabled(false);
    connect(viewMSFTagAct, SIGNAL(triggered()), this, SLOT(viewMSFTags()));

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

    //tagMenu->addAction(tagremoveAct);
    umTags = tagMenu->addMenu(tr("&Untag"));
    umTags->setEnabled(false);
    createUntagMenu();

    tagMenu->addSeparator();
    tagMenu->addAction(edittagtypesAct);

    /*
    tagtypeMenu = menuBar()->addMenu(tr("&TagType"));
    tagtypeMenu->addAction(tagtypeaddAct);
    tagtypeMenu->addAction(tagtyperemoveAct);
    tagtypeMenu->addSeparator();
    */

    sarfMenu = menuBar()->addMenu(tr("Tagtypes"));
    sarfMenu->addAction(sarftagsAct);
    sarfMenu->addAction(sarfAct);
    sarfMenu->addSeparator();
    sarfMenu->addAction(editMSFAct);
    sarfMenu->addAction(simulatorAct);

    analyseMenu = menuBar()->addMenu(tr("Analyse"));
    analyseMenu->addAction(diffAct);

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(viewMBFTagAct);
    viewMenu->addAction(viewMSFTagAct);

    paneMenu = menuBar()->addMenu(tr("&Panes"));

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

void AMTMainWindow::createUntagMenu() {
    umTags->clear();
    signalMapperUM = new QSignalMapper(this);
    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        QAction * taginstance;
        taginstance = new QAction((_atagger->tagTypeVector->at(i))->tag,this);
        signalMapperUM->setMapping(taginstance, (_atagger->tagTypeVector->at(i))->tag);
        connect(taginstance, SIGNAL(triggered()), signalMapperUM, SLOT(map()));
        umTags->addAction(taginstance);
    }
    connect(signalMapperUM, SIGNAL(mapped(const QString &)), this, SLOT(untag(QString)));
}

void AMTMainWindow::fillTreeWidget(Source Data, int basic) {
    tagDescription->clear();
     QList<QTreeWidgetItem *> items;
     if(basic == 0) {
         QVector<Tag> *temp = new QVector<Tag>(_atagger->tagVector);
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
     }
     else {
         QVector<MERFTag> temp(_atagger->simulationVector);
         for(int i=0; i<_atagger->simulationVector.count(); i++) {
             QStringList entry;
             QTextCursor cursor = txtBrwsr->textCursor();
             int pos = (temp.at(i)).pos;
             int length = (temp.at(i)).length;
             cursor.setPosition(pos,QTextCursor::MoveAnchor);
             cursor.setPosition(pos + length,QTextCursor::KeepAnchor);
             QString text = cursor.selectedText();
             entry<<text;
             entry<<(temp.at(i)).type;
             entry<<"MERF";
             entry<<QString::number(pos);
             entry<<QString::number(length);
             items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
         }
     }
     tagDescription->insertTopLevelItems(0, items);
}

void AMTMainWindow::itemSelectionChanged(QTreeWidgetItem* item ,int i) {
    descBrwsr->clear();
    scene->clear();
    txtBrwsr->textCursor().clearSelection();
    QList<QTreeWidgetItem *> items;
    QString type = item->text(1);
    bool done = false;

    for(int j=0; j < _atagger->tagTypeVector->count(); j++) {
        if((_atagger->tagTypeVector->at(j))->tag == type) {
            QString desc = (_atagger->tagTypeVector->at(j))->description;
            QColor bgcolor((_atagger->tagTypeVector->at(j))->bgcolor);
            QColor fgcolor((_atagger->tagTypeVector->at(j))->fgcolor);
            int font = (_atagger->tagTypeVector->at(j))->font;
            //bool underline = (_atagger->tagTypeVector->at(j))->underline;
            //bool underline = false;
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
            //if(underline)
            //    entry << "Underline" << "True";
            //else
            //    entry << "Underline" <<  "False";
            //items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
            //entry.clear();
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
            done = true;
            break;
        }
    }

    if(done) {
        return;
    }

    for(int j=0; j < _atagger->msfVector->count(); j++) {
        if((_atagger->msfVector->at(j))->name == type) {
            const MSFormula* msf = _atagger->msfVector->at(j);
            QString desc = msf->description;
            QColor bgcolor(msf->bgcolor);
            QColor fgcolor(msf->fgcolor);

            QTextCursor c = txtBrwsr->textCursor();
            c.setPosition(item->text(3).toInt(),QTextCursor::MoveAnchor);
            c.setPosition(item->text(3).toInt() + item->text(4).toInt(),QTextCursor::KeepAnchor);
            txtBrwsr->setTextCursor(c);

            QStringList entry;
            entry << "Text" << item->text(0);
            items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
            entry.clear();
            entry << "Description" <<desc;
            items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
            entry.clear();
            entry << "Formula" << type;
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
            entry << "Background Color" << QString();
            items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
            items.last()->setBackgroundColor(1,bgcolor);
            entry.clear();
            entry << "Foreground Color" << QString();
            items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
            items.last()->setBackgroundColor(1,fgcolor);
            descBrwsr->insertTopLevelItems(0, items);

            /** Create the match graph of graphviz
                We use this graph to create the match graph
                and calculate the coordinates of each node for better visualization
                **/
            GVC_t* gvc = gvContext();
            Agedge_t *edge = NULL;//,*edge1;
            char* args[] = {
            const_cast<char *>("dot"),
            const_cast<char *>("-Tsvg"),    /* svg output */
            const_cast<char *>("-oabc.svg") /* output to file abc.svg */
            };
            gvParseArgs(gvc, sizeof(args)/sizeof(char*), args);
            Agraph_t* G = agopen(const_cast<char *>("matchGraph"), Agstrictdirected, 0);
            //Set graph attributes
            agsafeset(G, const_cast<char *>("overlap"), const_cast<char *>("prism"),const_cast<char *>(""));
            agsafeset(G, const_cast<char *>("splines"), const_cast<char *>("true"),const_cast<char *>("true"));
            agsafeset(G, const_cast<char *>("pad"), const_cast<char *>("0,2"),const_cast<char *>("0,2"));
            agsafeset(G, const_cast<char *>("dpi"), const_cast<char *>("96,0"),const_cast<char *>("96,0"));
            agsafeset(G, const_cast<char *>("nodesep"), const_cast<char *>("0,4"),const_cast<char *>("0,4"));
            agattr(G,AGNODE,const_cast<char *>("label"),const_cast<char *>(""));
            agattr(G,AGNODE,const_cast<char *>("fixedsize"), const_cast<char *>("true"));
            agattr(G,AGNODE,const_cast<char *>("regular"), const_cast<char *>("true"));
            int id = 0;

            /// Used to track each node's source node
            QMap<Agnode_t *,Agnode_t *> parentNode;
            /// Add the match structure to description
            QStringList mSData;
            mSData << "Match" << QString();
            QTreeWidgetItem* matchSItem = new QTreeWidgetItem(descBrwsr,mSData);
            QTreeWidgetItem* tempItem = matchSItem;
            Agnode_t * tempNode = NULL;
            int index = tagDescription->indexOfTopLevelItem(item);
            const MERFTag& merftag = _atagger->simulationVector.at(index);
            for(int m=0; m<merftag.tags->count(); m++) {
                Tag* tag = merftag.tags->at(m);
                for(int n=0; n<tag->tType.count(); n++) {
                    QString structType = tag->tType[n];

                    if(structType == "mbf|") {
                        QStringList data;
                        data << tag->type << _atagger->text.mid(tag->pos,tag->length);
                        QTreeWidgetItem* newItem = new QTreeWidgetItem(tempItem,data);

                        QString text = tag->type + '\n' + _atagger->text.mid(tag->pos,tag->length);
                        char * writable = strdup(text.toStdString().c_str());
                        if(tempNode == NULL) {
                            stringstream strs;
                            strs << id;
                            string temp_str = strs.str();
                            char* nodeID = strdup(temp_str.c_str());
                            Agnode_t * newNode = agnode(G,nodeID, 1);
                            agset(G,const_cast<char *>("root"),nodeID);
                            id = id+1;
                            agset(newNode,const_cast<char *>("label"),writable);
                            parentNode.insert(newNode,NULL);
                            tempNode = newNode;
                        }
                        else {
                            stringstream strs;
                            strs << id;
                            string temp_str = strs.str();
                            char* nodeID = strdup(temp_str.c_str());
                            Agnode_t * newNode = agnode(G,nodeID, 1);
                            id = id+1;
                            agset(newNode,const_cast<char *>("label"),writable);
                            edge = agedge(G, tempNode, newNode, 0, 1);
                            parentNode.insert(newNode, tempNode);

                        }
                        free(writable);
                    }
                    else if(structType.contains("STAR")) {
                        if(structType.contains("pre")) {
                            QStringList data;
                            data << "Operation" << "*";
                            QTreeWidgetItem* newItem = new QTreeWidgetItem(tempItem,data);
                            tempItem = newItem;

                            if(tempNode == NULL) {
                                stringstream strs;
                                strs << id;
                                string temp_str = strs.str();
                                char* nodeID = strdup(temp_str.c_str());
                                Agnode_t * newNode = agnode(G,nodeID, 1);
                                agset(G,const_cast<char *>("root"),nodeID);
                                id = id+1;
                                agset(newNode,const_cast<char *>("label"),const_cast<char *>("*"));
                                parentNode.insert(newNode,NULL);
                                tempNode = newNode;
                            }
                            else {
                                stringstream strs;
                                strs << id;
                                string temp_str = strs.str();
                                char* nodeID = strdup(temp_str.c_str());
                                Agnode_t * newNode = agnode(G,nodeID, 1);
                                id = id+1;
                                agset(newNode,const_cast<char *>("label"),const_cast<char *>("*"));
                                edge = agedge(G, tempNode, newNode, 0, 1);
                                parentNode.insert(newNode, tempNode);
                                tempNode = newNode;
                            }
                        }
                        else {
                            tempItem = tempItem->parent();
                            tempNode = parentNode.value(tempNode);
                        }
                    }
                    else if(structType.contains("PLUS")) {
                        if(structType.contains("pre")) {
                            QStringList data;
                            data << "Operation" << "+";
                            QTreeWidgetItem* newItem = new QTreeWidgetItem(tempItem,data);
                            tempItem = newItem;

                            if(tempNode == NULL) {
                                stringstream strs;
                                strs << id;
                                string temp_str = strs.str();
                                char* nodeID = strdup(temp_str.c_str());
                                Agnode_t * newNode = agnode(G,nodeID, 1);
                                agset(G,const_cast<char *>("root"),nodeID);
                                id = id+1;
                                agset(newNode,const_cast<char *>("label"),const_cast<char *>("+"));
                                parentNode.insert(newNode,NULL);
                                tempNode = newNode;
                            }
                            else {
                                stringstream strs;
                                strs << id;
                                string temp_str = strs.str();
                                char* nodeID = strdup(temp_str.c_str());
                                Agnode_t * newNode = agnode(G,nodeID, 1);
                                id = id+1;
                                agset(newNode,const_cast<char *>("label"),const_cast<char *>("+"));
                                edge = agedge(G, tempNode, newNode, 0, 1);
                                parentNode.insert(newNode, tempNode);
                                tempNode = newNode;
                            }
                        }
                        else {
                            tempItem = tempItem->parent();
                            tempNode = parentNode.value(tempNode);
                        }
                    }
                    else if(structType.contains("QUESTION")) {
                        if(structType.contains("pre")) {
                            QStringList data;
                            data << "Operation" << "?";
                            QTreeWidgetItem* newItem = new QTreeWidgetItem(tempItem,data);
                            tempItem = newItem;

                            if(tempNode == NULL) {
                                stringstream strs;
                                strs << id;
                                string temp_str = strs.str();
                                char* nodeID = strdup(temp_str.c_str());
                                Agnode_t * newNode = agnode(G,nodeID, 1);
                                agset(G,const_cast<char *>("root"),nodeID);
                                id = id+1;
                                agset(newNode,const_cast<char *>("label"),const_cast<char *>("?"));
                                parentNode.insert(newNode,NULL);
                                tempNode = newNode;
                            }
                            else {
                                stringstream strs;
                                strs << id;
                                string temp_str = strs.str();
                                char* nodeID = strdup(temp_str.c_str());
                                Agnode_t * newNode = agnode(G,nodeID, 1);
                                id = id+1;
                                agset(newNode,const_cast<char *>("label"),const_cast<char *>("?"));
                                edge = agedge(G, tempNode, newNode, 0, 1);
                                parentNode.insert(newNode, tempNode);
                                tempNode = newNode;
                            }
                        }
                        else {
                            tempItem = tempItem->parent();
                            tempNode = parentNode.value(tempNode);
                        }
                    }
                    else if(structType.contains("UPTO")) {
                        if(structType.contains("pre")) {
                            QString limit = structType.section('|',2,2);
                            limit.prepend('^');
                            QStringList data;
                            data << "Operation" << limit;
                            QTreeWidgetItem* newItem = new QTreeWidgetItem(tempItem,data);
                            tempItem = newItem;

                            char * writable = strdup(limit.toStdString().c_str());
                            if(tempNode == NULL) {
                                stringstream strs;
                                strs << id;
                                string temp_str = strs.str();
                                char* nodeID = strdup(temp_str.c_str());
                                Agnode_t * newNode = agnode(G,nodeID, 1);
                                agset(G,const_cast<char *>("root"),nodeID);
                                id = id+1;
                                agset(newNode,const_cast<char *>("label"),writable);
                                parentNode.insert(newNode,NULL);
                                tempNode = newNode;
                            }
                            else {
                                stringstream strs;
                                strs << id;
                                string temp_str = strs.str();
                                char* nodeID = strdup(temp_str.c_str());
                                Agnode_t * newNode = agnode(G,nodeID, 1);
                                id = id+1;
                                agset(newNode,const_cast<char *>("label"),writable);
                                edge = agedge(G, tempNode, newNode, 0, 1);
                                parentNode.insert(newNode, tempNode);
                                tempNode = newNode;
                            }
                            free(writable);
                        }
                        else {
                            tempItem = tempItem->parent();
                            tempNode = parentNode.value(tempNode);
                        }
                    }
                    else if(structType.contains("OR")) {
                        if(structType.contains("pre")) {
                            QStringList data;
                            data << "Operation" << "|";
                            QTreeWidgetItem* newItem = new QTreeWidgetItem(tempItem,data);
                            tempItem = newItem;

                            if(tempNode == NULL) {
                                stringstream strs;
                                strs << id;
                                string temp_str = strs.str();
                                char* nodeID = strdup(temp_str.c_str());
                                Agnode_t * newNode = agnode(G,nodeID, 1);
                                agset(G,const_cast<char *>("root"),nodeID);
                                id = id+1;
                                agset(newNode,const_cast<char *>("label"),const_cast<char *>("|"));
                                parentNode.insert(newNode,NULL);
                                tempNode = newNode;
                            }
                            else {
                                stringstream strs;
                                strs << id;
                                string temp_str = strs.str();
                                char* nodeID = strdup(temp_str.c_str());
                                Agnode_t * newNode = agnode(G,nodeID, 1);
                                id = id+1;
                                agset(newNode,const_cast<char *>("label"),const_cast<char *>("|"));
                                edge = agedge(G, tempNode, newNode, 0, 1);
                                parentNode.insert(newNode, tempNode);
                                tempNode = newNode;
                            }
                        }
                        else {
                            tempItem = tempItem->parent();
                            tempNode = parentNode.value(tempNode);
                        }
                    }
                    else if(structType.contains("AND")) {
                        if(structType.contains("pre")) {
                            QStringList data;
                            data << "Operation" << "&&";
                            QTreeWidgetItem* newItem = new QTreeWidgetItem(tempItem,data);
                            tempItem = newItem;

                            if(tempNode == NULL) {
                                stringstream strs;
                                strs << id;
                                string temp_str = strs.str();
                                char* nodeID = strdup(temp_str.c_str());
                                Agnode_t * newNode = agnode(G,nodeID, 1);
                                agset(G,const_cast<char *>("root"),nodeID);
                                id = id+1;
                                agset(newNode,const_cast<char *>("label"),const_cast<char *>("&&"));
                                parentNode.insert(newNode,NULL);
                                tempNode = newNode;
                            }
                            else {
                                stringstream strs;
                                strs << id;
                                string temp_str = strs.str();
                                char* nodeID = strdup(temp_str.c_str());
                                Agnode_t * newNode = agnode(G,nodeID, 1);
                                id = id+1;
                                agset(newNode,const_cast<char *>("label"),const_cast<char *>("&&"));
                                edge = agedge(G, tempNode, newNode, 0, 1);
                                parentNode.insert(newNode, tempNode);
                                tempNode = newNode;
                            }
                        }
                        else {
                            tempItem = tempItem->parent();
                            tempNode = parentNode.value(tempNode);
                        }
                    }
                }
            }

            /** Set layout of the graph and get coordinates **/
            /* Compute a layout using layout engine from command line args */
            gvLayoutJobs(gvc, G);
            /* Write the graph according to -T and -o options */
            //gvRenderJobs(gvc, G);

            /** Set scene rectangle **/
            int left = GD_bb(G).LL.x;
            int top = GD_bb(G).LL.y;
            int width = GD_bb(G).UR.x;
            int height = GD_bb(G).UR.y;
            QRectF rect(left,top,width,height);
            scene->setSceneRect(rect);

            /** Get coordinates of nodes and draw nodes in scene **/
            QMap<Agnode_t*,GraphNode *> nodes;
            QMapIterator<Agnode_t *,Agnode_t *> iterator(parentNode);
            while (iterator.hasNext()) {
                iterator.next();
                /// Use this string to know if an edge label should be added
                QString edgeLabel;
                Agnode_t* tmpNode1 = iterator.key();
                char* label1 = agget(tmpNode1,const_cast<char *>("label"));
                qreal nodeX1 = ND_coord(tmpNode1).x;
                qreal nodeY1 = (GD_bb(G).UR.y - ND_coord(tmpNode1).y);
                GraphNode *node1;
                if(!(nodes.contains(tmpNode1))) {
                    QString stringlabel1(label1);
                    if(stringlabel1.contains('\n')) {
                        QString text = stringlabel1.section('\n',1,1);
                        edgeLabel = stringlabel1.section('\n',0,0);
                        QString fgcolor;
                        QString bgcolor;
                        for(int k=0; k<_atagger->tagTypeVector->count(); k++) {
                            const TagType* tt = (TagType*)(_atagger->tagTypeVector->at(k));
                            if(edgeLabel == tt->tag) {
                                fgcolor = tt->fgcolor;
                                bgcolor = tt->bgcolor;
                            }
                        }
                        node1 = new GraphNode(text,fgcolor,bgcolor);
                    }
                    else {
                        node1 = new GraphNode(stringlabel1,"","");
                    }
                    scene->addItem(node1);
                    node1->setPos(nodeX1,nodeY1);
                    nodes.insert(tmpNode1,node1);
                }
                else {
                    node1 = nodes.value(tmpNode1);
                }

                Agnode_t* tmpNode2 = iterator.value();
                if(tmpNode2 == NULL) {
                    continue;
                }
                char* label2 = agget(tmpNode2,const_cast<char *>("label"));
                qreal nodeX2 = ND_coord(tmpNode2).x;
                qreal nodeY2 = (GD_bb(G).UR.y - ND_coord(tmpNode2).y);
                GraphNode *node2;
                if(!(nodes.contains(tmpNode2))) {
                    QString stringlabel2(label2);
                    node2 = new GraphNode(stringlabel2,"","");
                    scene->addItem(node2);
                    node2->setPos(nodeX2,nodeY2);
                    nodes.insert(tmpNode2,node2);
                    if(edgeLabel.isEmpty()) {
                        scene->addItem(new GraphEdge(node2,node1));
                    }
                    else {
                        scene->addItem(new GraphEdge(node2,node1,edgeLabel));
                        edgeLabel.clear();
                    }
                }
                else {
                    node2 = nodes.value(tmpNode2);
                    if(edgeLabel.isEmpty()) {
                        scene->addItem(new GraphEdge(node2,node1));
                    }
                    else {
                        scene->addItem(new GraphEdge(node2,node1,edgeLabel));
                        edgeLabel.clear();
                    }
                }
            }

            /* Free layout data */
            gvFreeLayout(gvc, G);
            /* Free graph structures */
            agclose(G);
            /* close output file and free context */
            gvFreeContext(gvc);
            break;
        }
    }
}

void AMTMainWindow::sarfTagging() {
    /*
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
            setWindowTitle("MERF: " + _atagger->tagFile);
        }
    }
    */
    if(_atagger->tagTypeVector->isEmpty()) {
        QMessageBox::warning(this,"Warning","No tag types with MBF defined!");
        return;
    }

    if(!(_atagger->simulationVector.isEmpty())) {
        QMessageBox msgBox;
         msgBox.setText("The MSF Simulation matches will be cleared.");
         msgBox.setInformativeText("Do you want to Continue?");
         msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
         msgBox.setDefaultButton(QMessageBox::No);
         int ret = msgBox.exec();

         switch (ret) {
         case QMessageBox::Yes:
             _atagger->simulationVector.clear();
             break;
         case QMessageBox::No:
             return;
         default:
             return;
         }
    }

    _atagger->isSarf = true;
    startTaggingText(_atagger->text);
    scene->clear();
    descBrwsr->clear();
    _atagger->tagVector.clear();

    error_str = "";
    output_str = "";
    dirty = true;

    QString text = _atagger->text;

    /** Process Tag Type and Create Hash function for Synonymity Sets **/

    QHash< QString, QSet<QString> > synSetHash;

    for( int i=0; i< (_atagger->tagTypeVector->count()); i++) {

        /** Check if tag source is sarf tag types **/
        if(_atagger->tagTypeVector->at(i)->source != sarf) {
            continue;
        }

        const SarfTagType * tagtype = (SarfTagType*)(_atagger->tagTypeVector->at(i));
        for(int j=0; j < (tagtype->tags.count()); j++) {
            const Quadruple< QString , QString , QString , QString > * tag = &(tagtype->tags.at(j));
            if(tag->fourth.contains("Syn")) {

                int order = tag->fourth.mid(3).toInt();
                GER ger(tag->second,1,order);
                ger();

                QString gloss_order = tag->second;
                gloss_order.append(QString::number(order));
                QSet<QString> glossSynSet;
                for(int i=0; i<ger.descT.count(); i++) {
                    const IGS & igs = ger.descT[i];
                    glossSynSet.insert(igs.getGloss());
                }
                synSetHash.insert(gloss_order,QSet<QString>(glossSynSet));
            }
        }
    }

    /** Synonymity Sets Created **/

    /** Process Text and analyse each work using sarf **/

    AutoTagger autotag(&text,&synSetHash);
    autotag();

    _atagger->isTagMBF = true;

    /** Analysis Done **/

    applyTags();

    fillTreeWidget(sarf);
    finishTaggingText();
}

void AMTMainWindow::customizeSarfTags() {
    if(!_atagger->textFile.isEmpty()) {
        sarfAct->setEnabled(true);
        simulatorAct->setEnabled(true);
    }

    if(_atagger->tagtypeFile.isEmpty()) {

        QStringList dirList = _atagger->tagFile.split('/');
        dirList.removeLast();
        QString dir = dirList.join("/");
        dir.append('/');

        QString ttFileName = QFileDialog::getSaveFileName(this,
                                                          tr("Sarf TagType File Name"), dir,
                                                          tr("tag types (*.stt.json);;All Files (*)"));
        if(ttFileName.isEmpty())
        {
            return;
        }
        else {

            QString relativePaths = QDir(dir).relativeFilePath(ttFileName);
            _atagger->tagtypeFile = relativePaths + ".stt.json";
            lineEditTTFName->setText(_atagger->tagtypeFile);
            //btnTTFName->setEnabled(false);
        }
    }

    CustomSTTView * cttv = new CustomSTTView(this);
    cttv->show();
}

void AMTMainWindow::difference() {

#if 1
    _atagger->compareToTagFile.clear();
    _atagger->compareToTagTypeFile.clear();
    _atagger->compareToTagTypeVector->clear();
    _atagger->compareToTagVector.clear();
    if(_atagger->tagVector.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText("There are no tags present to compare to");
        return;
    }

    /** Get and open tags file **/

    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Open tag file B to compare to"), "",
             tr("Tag Types (*.tags.json);;All Files (*)"));

    if(fileName.isEmpty()) {
        return;
    }

    QFile ITfile(fileName);
    if (!ITfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::about(this, tr("Input File"),
                     tr("The <b>Tag File</b> can't be opened!"));
        return;
    }

    _atagger->compareToTagFile = fileName;

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

    QString textFile = result["file"].toString();

    QFile Ifile(textFile);
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
        _atagger->compareToTagFile.clear();
        return;
    }

    if(text.compare(_atagger->text,Qt::CaseSensitive) !=0 ) {
        QMessageBox::warning(this,"Warning","The input text file is different than the original one!");
        _atagger->compareToTagFile.clear();
        return;
    }
    Ifile.close();

    /** process difference tags **/

    QString tagtypeFile = result.value("TagTypeFile").toString();
    if(tagtypeFile.isEmpty()) {
        QMessageBox::warning(this,"Warning","The Tag Type File is not specified!");
        _atagger->compareToTagFile.clear();
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
        check = _atagger->insertTag(tagtype,start,length,source,compareTo);
    }

    if(_atagger->tagVector.at(0).source == user) {
        _atagger->compareToIsSarf = false;
    }
    else {
        _atagger->compareToIsSarf = true;
    }
    _atagger->compareToTagTypeFile = tagtypeFile;

    /** Read the TagType file and store it **/

    QFile ITTfile(tagtypeFile);
    if (!ITTfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::about(this, tr("Input Tag File"),
                     tr("The <b>Tag Type File</b> can't be opened!"));
        _atagger->compareToTagFile.clear();
        _atagger->compareToTagTypeFile.clear();
        _atagger->compareToTagVector.clear();
        return;
    }

    QByteArray tagtypedata = ITTfile.readAll();
    ITTfile.close();

    /** Process Tag Types **/

    QJson::Parser parsertt;

    QVariantMap resulttt = parsertt.parse(tagtypedata,&ok).toMap();

    if (!ok) {
        QMessageBox::about(this, tr("Input Tag File"),
                     tr("The <b>Tag Types File</b> has a wrong format"));
    }

    foreach(QVariant type, resulttt["TagTypeSet"].toList()) {
        QString tag;
        QString desc;
        int id;
        QString foreground_color;
        QString background_color;
        int font;
        bool underline = false;
        bool bold;
        bool italic;

        QVariantMap typeElements = type.toMap();

        tag = typeElements["Tag"].toString();
        desc = typeElements["Description"].toString();
        id = typeElements["Legend"].toInt();
        foreground_color = typeElements["foreground_color"].toString();
        background_color = typeElements["background_color"].toString();
        font = typeElements["font"].toInt();
        //underline = typeElements["underline"].toBool();
        bold = typeElements["bold"].toBool();
        italic = typeElements["italic"].toBool();

        if(!typeElements.value("Features").isNull()) {

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
                        quad.fourth = st.value("Relation").toString();
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
                        quad.fourth = st.value("Relation").toString();
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
                        quad.fourth = st.value("Relation").toString();
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

            _atagger->isSarf = true;
            _atagger->insertSarfTagType(tag,tags,desc,id,foreground_color,background_color,font,underline,bold,italic,sarf,compareTo);
        }
        else {
            _atagger->isSarf = false;
            _atagger->insertTagType(tag,desc,id,foreground_color,background_color,font,underline,bold,italic,user,compareTo);
        }
    }
#endif
    /** Show the difference view **/

    DiffView * diff = new DiffView(this);
    diff->show();
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
            case QMessageBox::No:
               return;
           default:
               return;
         }
    }

    QStringList dirList = _atagger->tagFile.split('/');
    dirList.removeLast();
    QString dir = dirList.join("/");
    dir.append('/');

    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Text File"), dir,
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

         _atagger->tagVector.clear();
         tagDescription->clear();
         descBrwsr->clear();

         lineEditTFName->setText(fileName);
         sarfAct->setEnabled(true);
         simulatorAct->setEnabled(true);
         //btnTFName->setEnabled(false);
         QString text = file.readAll();

         QString relativePaths = QDir(dir).relativeFilePath(fileName);
         _atagger->textFile = relativePaths;
         _atagger->text = text;
         txtBrwsr->setText(text);
         setLineSpacing(10);
    }
}

void AMTMainWindow::loadTagTypes_clicked() {
    if(!(_atagger->tagtypeFile.isEmpty())) {
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
           default:
               return;
         }
    }

    QStringList dirList = _atagger->tagFile.split('/');
    dirList.removeLast();
    QString dir = dirList.join("/");
    dir.append('/');

    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Tag Type File"), dir,
             tr("Tag Type File (*.tt.json *.stt.json);;All Files (*)"));

    if(fileName.isEmpty()) {
        return;
    }
    else {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),file.errorString());
            return;
        }

        QString relativePaths = QDir(dir).relativeFilePath(fileName);
        if(fileName.endsWith(".tt.json")) {
            _atagger->tagtypeFile = relativePaths;
        }
        else if(fileName.endsWith(".stt.json")) {
            _atagger->tagtypeFile = relativePaths;
            sarfAct->setEnabled(true);
            simulatorAct->setEnabled(true);
        }
        else {
            QMessageBox::warning(this,"Warning","The selected file doesn't have the correct extension!");
            return;
        }

        _atagger->tagVector.clear();
        _atagger->tagTypeVector->clear();
        tagDescription->clear();
        descBrwsr->clear();
        txtBrwsr->clear();
        txtBrwsr->setText(_atagger->text);

        lineEditTTFName->setText(fileName);
        QByteArray tagtypes = file.readAll();
        process_TagTypes(tagtypes);

        applyTags();
        //createTagMenu();
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
        setWindowTitle("MERF: " + _atagger->tagFile);


	createDockWindows(false);

	sarftagsAct->setEnabled(true);
        editMSFAct->setEnabled(true);
	edittagtypesAct->setEnabled(true);
	tagremoveAct->setEnabled(true);
	mTags->setEnabled(true);
        umTags->setEnabled(true);
        viewMBFTagAct->setEnabled(true);
        viewMSFTagAct->setEnabled(true);
	saveAct->setEnabled(true);
	saveasAct->setEnabled(true);
        diffAct->setEnabled(true);
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

void AMTMainWindow::customizeMSFs() {

    if(!_atagger->textFile.isEmpty()) {
        simulatorAct->setEnabled(true);
    }

    if(_atagger->tagtypeFile.isEmpty()) {
        QString msfFileName = QFileDialog::getSaveFileName(this,
                                                           tr("Sarf TagType File Name"), "",
                                                           tr("tag types (*.stt.json);;All Files (*)"));
        if(msfFileName.isEmpty())
        {
            return;
        }
        else {
            _atagger->tagtypeFile = msfFileName + ".stt.json";
        }
    }

    CustomizeMSFView * cmsfv = new CustomizeMSFView(this);
    cmsfv->show();
}

void AMTMainWindow::runMERFSimulator() {
    if(_atagger->msfVector->isEmpty()) {
        return;
    }

    /*
    if(_atagger->text.isEmpty()) {
        QMessageBox::warning(this,"Warning","Add a text file before initializing the Sarf Analyzer!");
        return;
    }

    if(_atagger->tagFile.isEmpty()) {
        QMessageBox::warning(this,"Warning","You have to add tag types first!");
        return;
    }
    */
    if(_atagger->tagTypeVector->isEmpty()) {
        QMessageBox::warning(this,"Warning","No tag types with MBF defined!");
        return;
    }
    _atagger->simulationVector.clear();
    if(_atagger->tagVector.isEmpty()) {
        sarfTagging();
    }

    _atagger->isSarf = true;
    startTaggingText(_atagger->text);
    _atagger->simulationVector.clear();
    for(int i=0; i<_atagger->nfaVector->count(); i++) {
        delete (_atagger->nfaVector->at(i));
    }
    _atagger->nfaVector->clear();

    error_str = "";
    output_str = "";
    dirty = true;

    //QString text = _atagger->text;

    if(!(_atagger->runSimulator())) {
        return;
    }

    _atagger->isTagMBF = false;

    applyTags(1);
    fillTreeWidget(sarf,1);
    finishTaggingText();
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

void AMTMainWindow::closeEvent(QCloseEvent *event) {

    if(dirty) {
        QMessageBox msgBox;
         msgBox.setText("The document has been modified.");
         msgBox.setInformativeText("Do you want to save your changes?");
         msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
         msgBox.setDefaultButton(QMessageBox::Save);
         int ret = msgBox.exec();

         switch (ret) {
         case QMessageBox::Save:
             save();
             break;
         case QMessageBox::Discard:
             break;
         default:
             break;
         }
     }
}

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForTr( QTextCodec::codecForName( "UTF-8" ) );
    QTextCodec::setCodecForCStrings( QTextCodec::codecForName( "UTF-8" ) );

    _atagger=new ATagger();

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

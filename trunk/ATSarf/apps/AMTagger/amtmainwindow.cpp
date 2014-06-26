#include <QtGui/QApplication>
#include "amtmainwindow.h"
#include <qjson/parser.h>

#include <QContextMenuEvent>
#include <QTextCodec>
#include <QDockWidget>
#include <QList>
#include <QMessageBox>
#include <QTextBlock>
#include <QtAlgorithms>
#include <sys/time.h>
#include "Triplet.h"

bool parentCheck;
class SarfTag;

AMTMainWindow::AMTMainWindow(QWidget *parent) :
    QMainWindow(parent),
    browseFileDlg(NULL)
{
    resize(800,600);

    /// Used to check for parent Widget between windows (EditTagTypeView and AMTMainWindow)
    parentCheck = false;

    /// Boolean to check any change in tags
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
    tagDescription->setColumnCount(6);
    QStringList columns;
    columns << "index" << "Word" << "Tag" << "Source" << "Start" << "Length";
    QTreeWidgetItem* item=new QTreeWidgetItem(columns);
    tagDescription->hideColumn(0);
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

    dock = new QDockWidget(this);
    tab = new QTabWidget(this);
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

    /** Scene of entity-relation Graph**/
    relationGraphics = new QGraphicsView(this);
    relationScene = new QGraphicsScene(this);
    relationScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    relationScene->setSceneRect(-150, -150, 300, 300);
    relationGraphics->setScene(relationScene);
    relationGraphics->setCacheMode(QGraphicsView::CacheBackground);
    relationGraphics->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    relationGraphics->setRenderHint(QPainter::Antialiasing);
    relationGraphics->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    relationGraphics->scale(qreal(2), qreal(2));

    tab->addTab(graphics,"Match Tree");
    tab->addTab(relationGraphics,"Entity-Relation Graph");
    dock->setWidget(tab);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    paneMenu->addAction(dock->toggleViewAction());
}

void AMTMainWindow::wheelEvent(QWheelEvent *event) {
    if(graphics->hasFocus()|| relationGraphics->hasFocus()) {
        int currentIndex = tab->currentIndex();
        if(currentIndex == 0) {
            scaleView(pow((double)2, event->delta() / 240.0));
        }
        else {
            relationScaleView(pow((double)2, event->delta() / 240.0));
        }
    }

}

void AMTMainWindow::scaleView(qreal scaleFactor)
{
    qreal factor = graphics->transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    graphics->scale(scaleFactor, scaleFactor);
}

void AMTMainWindow::relationScaleView(qreal scaleFactor)
{
    qreal factor = relationGraphics->transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    relationGraphics->scale(scaleFactor, scaleFactor);
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
        int wordIndex = _atagger->wordIndexMap.value(pos);
        if(wordIndex == 0) {
            return;
        }
        QList<Tag*> values = _atagger->tagHash.values(wordIndex);
        for(int i=0; i<values.size();i++) {
            tagtypes << values.at(i)->tagtype->name;
        }
    }
    else {
        for(int i=0; i < _atagger->simulationVector.count(); i++) {
            const MERFTag * t = (MERFTag*)(&(_atagger->simulationVector.at(i)));
            if(t->pos == pos) {
                tagtypes << t->msf->name;
            }
        }
    }
    signalMapper = new QSignalMapper(this);
    QMenu * menu = new QMenu();
    QMenu * mTags;
    mTags = menu->addMenu(tr("Tag"));
    if(_atagger->isTagMBF) {
        for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
            if(!(tagtypes.contains(_atagger->tagTypeVector->at(i)->name))) {
                QAction * taginstance;
                taginstance = new QAction((_atagger->tagTypeVector->at(i))->name,this);
                signalMapper->setMapping(taginstance, (_atagger->tagTypeVector->at(i))->name);
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
            if(tagtypes.contains(_atagger->tagTypeVector->at(i)->name)) {
                QAction * taginstance;
                taginstance = new QAction((_atagger->tagTypeVector->at(i))->name,this);
                signalMapperU->setMapping(taginstance, (_atagger->tagTypeVector->at(i))->name);
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
    menu->addSeparator();
    menu->addAction(addtagAct);

    menu->exec(txtBrwsr->mapToGlobal(pt));
    delete menu;
}

void processText(QString *text) {

    _atagger->wordIndexMap.clear();
    _atagger->isStatementEndFSSet.clear();
    _atagger->isStatementEndPSet.clear();
    int start = 0;
    int wordIndex = 1;
    while(start != text->count()) {
        Word word = nextWord(*text, start);
        if(word.isStatementStartFS && wordIndex != 1) {
            _atagger->isStatementEndFSSet.insert(wordIndex-1);
        }
        if(word.isStatementStartPunct && wordIndex != 1) {
            _atagger->isStatementEndPSet.insert(wordIndex-1);
        }
        if(word.word.isEmpty()) {
            break;
        }

        _atagger->wordIndexMap.insert(word.start,wordIndex);
        start = word.end + 1;
        wordIndex = wordIndex + 1;
    }
    _atagger->wordCount = wordIndex - 1;
}

void AMTMainWindow::open() {

    if(dirty == true) {
        /// Save current data
    }
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

    /** Check tag file version **/

    double version;
    if(result.value("version").isNull()) {
        QMessageBox::warning(this, "Warning", "Tool doesn't support this tag version anymore");
        return;
    }
    else {
        version = result.value("version").toDouble();
        if(version<1.0) {
            QMessageBox::warning(this, "Warning", "Tool doesn't support this tag version anymore");
            return;
        }
    }

    /** End of Check **/

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
    processText(&text);
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

bool AMTMainWindow::readMatch(MSFormula* formula, QVariant data, Match* parent) {
    /** Common variables in a match **/
    QString type;
    QString msfName;

    QVariantMap matchData = data.toMap();
    type = matchData.value("type").toString();
    msfName = matchData.value("msf").toString();

    if(type == "key") {
        int pos = matchData.value("pos").toInt();
        int length = matchData.value("length").toInt();
        QString key = matchData.value("key").toString();
        QString word = matchData.value("word").toString();
        MSF* msf = formula->map.value(msfName);
        KeyM* keym = new KeyM(parent,key,pos,length);
        keym->word = word;
        keym->msf = msf;
        parent->setMatch(keym);
        return true;
    }
    else if(type == "unary") {
        MSF* msf = formula->map.value(msfName);
        int limit = matchData.value("limit").toInt();
        Operation op = (Operation)(matchData.value("op").toInt());
        UnaryM* unary = new UnaryM(op,parent,limit);
        unary->msf = msf;
        foreach(QVariant unaryMatchData, matchData.value("matches").toList()) {
            if(!(readMatch(formula,unaryMatchData,unary))) {
                return false;
            }
        }
        parent->setMatch(unary);
        return true;
    }
    else if(type == "binary") {
        MSF* msf = formula->map.value(msfName);
        Operation op = (Operation)(matchData.value("op").toInt());
        BinaryM* binary = new BinaryM(op,parent);
        binary->msf = msf;
        QVariant leftMatchData = matchData.value("leftMatch");
        if(!(readMatch(formula,leftMatchData,binary))) {
            return false;
        }
        if(!(matchData.value("rightMatch").isNull())) {
            QVariant rightMatchData = matchData.value("rightMatch");
            if(!(readMatch(formula,rightMatchData,binary))) {
                return false;
            }
        }
        parent->setMatch(binary);
        return true;
    }
    else if(type == "sequential") {
        SequentialM* seq = new SequentialM(parent);
        if(msfName == "_NULL_") {
            seq->msf = NULL;
        }
        else {
            MSF* msf = formula->map.value(msfName);
            seq->msf = msf;
        }
        foreach(QVariant seqMatchData, matchData.value("matches").toList()) {
            if(!(readMatch(formula,seqMatchData,seq))) {
                return false;
            }
        }
        parent->setMatch(seq);
        return true;
    }
    else {
        /// MERFTag case
        MERFTag* merftag = new MERFTag();
        MSF* msf = formula->map.value(msfName);
        merftag->msf = msf;
        QString formulaName = matchData.value("formula").toString();
        for(int i=0; i<_atagger->msfVector->count(); i++) {
            if(_atagger->msfVector->at(i)->name == formulaName) {
                merftag->formula = _atagger->msfVector->at(i);
                break;
            }
        }
        QVariant merftagMatchData = matchData.value("match");
        if(!(readMatch(formula,merftagMatchData,merftag))) {
            return false;
        }
        parent->setMatch(merftag);
        return true;
    }
    return false;
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

    /** Read Tags **/

    foreach(QVariant tag, result["TagArray"].toList()) {

        QVariantMap tagElements = tag.toMap();
        int start = tagElements["pos"].toInt();
        int length = tagElements["length"].toInt();
        int wordIndex = tagElements["wordIndex"].toInt();
        QString tagtype = tagElements["type"].toString();
        Source source = (Source)(tagElements["source"].toInt());
        bool check;
        const TagType* type = NULL;
        for(int i=0;i<_atagger->tagTypeVector->count(); i++) {
            if(_atagger->tagTypeVector->at(i)->name == tagtype) {
                type = _atagger->tagTypeVector->at(i);
                break;
            }
        }
        if(type == NULL) {
            QMessageBox::warning(this,"warning","Something went wrong in tag file processing!");
            _atagger = new ATagger();
            clearLayout(this->layout());
            return;
        }
        check = _atagger->insertTag(type,start,length,wordIndex,source,original);
    }

    if(_atagger->tagHash.begin().value()->source == user) {
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
            QString formulaName = merfTagElements.value("formula").toString();
            int pos = merfTagElements.value("pos").toInt();
            int length = merfTagElements.value("length").toInt();
            Source source = (Source)(merfTagElements.value("source").toInt());
            MSFormula* formula = NULL;
            for(int i=0; i<_atagger->msfVector->count(); i++) {
                if(_atagger->msfVector->at(i)->name == formulaName) {
                    formula = _atagger->msfVector->at(i);
                    break;
                }
            }
            if(formula == NULL) {
                QMessageBox::warning(this,"warning","Something went wrong in tag file processing!");
                _atagger = new ATagger();
                clearLayout(this->layout());
                return;
            }
            MERFTag* merftag = new MERFTag(formula);
            merftag->pos = pos;
            merftag->length = length;
            merftag->source = source;
            if(!(merfTagElements.value("match").isNull())) {
                if(!(readMatch(formula,merfTagElements.value("match"),merftag))) {
                    QMessageBox::warning(this,"warning","Something went wrong in tag file processing!");
                    _atagger = new ATagger();
                    clearLayout(this->layout());
                    return;
                }
            }
            else {
                merftag->match = NULL;
            }

            for(int j=0; j<formula->relationVector.count(); j++) {
                Relation* relation = formula->relationVector[j];
                QVector<Match*> entity1;
                QVector<Match*> entity2;
                QVector<Match*> edge;
                merftag->match->constructRelation(relation,entity1,entity2,edge);
                if(!(entity1.isEmpty()) && !(entity2.isEmpty()) && (!(edge.isEmpty()) || (relation->edge == NULL))) {
                    if((entity1.count() == 1) && (entity2.count() == 1) && (edge.count() == 1)) {
                        /// single relation
                        RelationM* relM = new RelationM(relation,entity1[0],entity2[0],edge[0]);
                        merftag->relationMatchVector.append(relM);
                    }
                    else if((entity1.count() == 1) && (entity2.count() == 1) && (edge.isEmpty())) {
                        /// single with no edge
                        RelationM* relM = new RelationM(relation,entity1[0],entity2[0],NULL);
                        merftag->relationMatchVector.append(relM);
                    }
                    else if(edge.count() > 1) {
                        /// cases where the edge has multiple matches

                        /// extract the edge label first
                        QString edgeLabel;
                        if(relation->edgeLabel == "text") {
                            for(int k=0; k<edge.count(); k++) {
                                edgeLabel.append(edge.at(k)->getText() + ", ");
                            }
                            edgeLabel.chop(2);
                        }
                        else if(relation->edgeLabel == "position") {
                            for(int k=0; k<edge.count(); k++) {
                                edgeLabel.append(QString::number(edge.at(k)->getPOS()) + ", ");
                            }
                            edgeLabel.chop(2);
                        }
                        else if(relation->edgeLabel == "length") {
                            for(int k=0; k<edge.count(); k++) {
                                edgeLabel.append(QString::number(edge.at(k)->getLength()) + ", ");
                            }
                            edgeLabel.chop(2);
                        }
                        else if(relation->edgeLabel == "number") {
                            for(int k=0; k<edge.count(); k++) {
                                QString text = edge.at(k)->getText();
                                NumNorm nn(&text);
                                nn();
                                int number = NULL;
                                if(nn.extractedNumbers.count()!=0) {
                                    number = nn.extractedNumbers[0].getNumber();
                                     edgeLabel.append(QString::number(number) + ", ");
                                }
                            }
                            if(!(edgeLabel.isEmpty())) {
                                edgeLabel.chop(2);
                            }
                        }

                        if(entity1.count() == 1 && entity2.count() == 1) {
                            RelationM* relM = new RelationM(relation,entity1[0],entity2[0],NULL);
                            relM->edgeLabel = edgeLabel;
                            merftag->relationMatchVector.append(relM);
                        }
                        else if(relation->entity1->name == relation->entity2->name) {
                            /// Relation entities belong to a + or *
                            for(int k=1; k< entity1.count(); k++) {
                                RelationM* relM = new RelationM(relation,entity1[k-1],entity2[k],NULL);
                                relM->edgeLabel = edgeLabel;
                                merftag->relationMatchVector.append(relM);
                            }
                        }
                        else if(entity1.count() > 1 && entity2.count() == 1) {
                            /// multiple matches for entity1 and 1 for entity 2
                            for(int k=0; k<entity1.count(); k++) {
                                RelationM* relM = new RelationM(relation,entity1[k],entity2[0],NULL);
                                relM->edgeLabel = edgeLabel;
                                merftag->relationMatchVector.append(relM);
                            }
                        }
                        else if(entity1.count() == 1 && entity2.count() > 1) {
                            /// multiple matches for entity2 and 1 for entity 1
                            for(int k=0; k<entity2.count(); k++) {
                                RelationM* relM = new RelationM(relation,entity1[0],entity2[k],NULL);
                                relM->edgeLabel = edgeLabel;
                                merftag->relationMatchVector.append(relM);
                            }
                        }
                        else {
                            /// multiple matches for entities 1 and 2
                            for(int m=0; m<entity1.count(); m++) {
                                for(int n=0; n<entity2.count(); n++) {
                                    RelationM* relM = new RelationM(relation,entity1[m],entity2[n],NULL);
                                    relM->edgeLabel = edgeLabel;
                                    merftag->relationMatchVector.append(relM);
                                }
                            }
                        }
                    }
                    else {
                        /// cases where the edge has only one match

                        if(relation->entity1->name == relation->entity2->name) {
                            /// Relation entities belong to a + or *
                            for(int k=1; k< entity1.count(); k++) {
                                RelationM* relM;
                                if(edge.count() == 1) {
                                    relM = new RelationM(relation,entity1[k-1],entity2[k],edge[0]);
                                }
                                else {
                                    relM = new RelationM(relation,entity1[k-1],entity2[k],NULL);
                                }
                                merftag->relationMatchVector.append(relM);
                            }
                        }
                        else if(entity1.count() > 1 && entity2.count() == 1) {
                            /// multiple matches for entity1 and 1 for entity 2
                            for(int k=0; k<entity1.count(); k++) {
                                RelationM* relM;
                                if(edge.count() == 1) {
                                    relM = new RelationM(relation,entity1[k],entity2[0],edge[0]);
                                }
                                else {
                                    relM = new RelationM(relation,entity1[k],entity2[0],NULL);
                                }
                                merftag->relationMatchVector.append(relM);
                            }
                        }
                        else if(entity1.count() == 1 && entity2.count() > 1) {
                            /// multiple matches for entity2 and 1 for entity 1
                            for(int k=0; k<entity2.count(); k++) {
                                RelationM* relM;
                                if(edge.count() == 1) {
                                    relM = new RelationM(relation,entity1[0],entity2[k],edge[0]);
                                }
                                else {
                                    relM = new RelationM(relation,entity1[0],entity2[k],NULL);
                                }
                                merftag->relationMatchVector.append(relM);
                            }
                        }
                        else {
                            /// multiple matches for entities 1 and 2
                            for(int m=0; m<entity1.count(); m++) {
                                for(int n=0; n<entity2.count(); n++) {
                                    RelationM* relM;
                                    if(edge.count() == 1) {
                                        relM = new RelationM(relation,entity1[m],entity2[n],edge[0]);
                                    }
                                    else {
                                        relM = new RelationM(relation,entity1[m],entity2[n],NULL);
                                    }
                                    merftag->relationMatchVector.append(relM);
                                }
                            }
                        }
                    }
                }
            }
            /*
            if(!(merfTagElements.value("relationMatches").isNull())) {
                foreach(QVariant relMatch, merfTagElements.value("relationMatches").toList()) {
                    QVariantMap relmData = relMatch.toMap();
                    QString relationName = relmData.value("relation").toString();
                    QString e1Label = relmData.value("e1Label").toString();
                    QString e2Label = relmData.value("e2Label").toString();
                    QString edgeLabel = relmData.value("edgeLabel").toString();
                    Relation* rel = NULL;
                    for(int i=0; i<merftag->formula->relationVector.count();i++) {
                        if(merftag->formula->relationVector.at(i)->name == relationName) {
                            rel = merftag->formula->relationVector[i];
                        }
                    }
                    RelationM* relm = new RelationM(rel,NULL,e1Label,NULL,e2Label,NULL,edgeLabel);
                    merftag->relationMatchVector.append(relm);
                }
            }
            */

#if 0
            foreach(QVariant tagData, merfTagElements["tags"].toList()) {

                QVariantMap tagElements = tagData.toMap();
                QString tagType = tagElements.value("type").toString();
                int tagpos = tagElements.value("pos").toInt();
                int taglength = tagElements.value("length").toInt();
                int wordIndex = tagElements.value("wordIndex").toInt();
                const TagType* type = NULL;
                for(int i=0;i<_atagger->tagTypeVector->count(); i++) {
                    if(_atagger->tagTypeVector->at(i)->name == tagType) {
                        type = _atagger->tagTypeVector->at(i);
                        break;
                    }
                }
                if(type == NULL) {
                    QMessageBox::warning(this,"warning","Something went wrong in tag file processing!");
                    _atagger = new ATagger();
                    clearLayout(this->layout());
                    return;
                }
                Tag *tag = new Tag(type,tagpos,taglength,wordIndex,sarf);

                if(!(tagElements.value("tagType").isNull())) {
                    foreach(QVariant tagTypeData, tagElements["tagType"].toList()) {
                        QString matchInfo = tagTypeData.toString();
                        tag->tType.append(matchInfo);
                    }
                }
                merftag.tags->append(tag);
            }
#endif

            _atagger->simulationVector.append(merftag);
        }
        _atagger->isTagMBF = false;
    }

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

    foreach(QVariant type, result["TagTypeSet"].toList()) {
        QString tag;
        QString desc;
        QString foreground_color;
        QString background_color;
        int font;
        bool underline = false;
        bool bold;
        bool italic;

        QVariantMap typeElements = type.toMap();

        tag = typeElements["Tag"].toString();
        desc = typeElements["Description"].toString();
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
            _atagger->insertSarfTagType(tag,tags,desc,foreground_color,background_color,font,underline,bold,italic,sarf,original);
        }
        else {
            _atagger->isSarf = false;
            _atagger->insertTagType(tag,desc,foreground_color,background_color,font,underline,bold,italic,user,original);
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
            bool isFullStop;

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
            if(!(msformulaData.value("delimiter").isNull())) {
                isFullStop = msformulaData.value("delimiter").toBool();
            }
            else {
                isFullStop = true;
            }

            MSFormula* msf = new MSFormula(name, NULL);
            msf->isFullStop = isFullStop;
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

            /** Get relations **/
            if(!(msformulaData.value("Relations").isNull())) {
                foreach(QVariant relationsData, msformulaData.value("Relations").toList()) {
                    QVariantMap relationData = relationsData.toMap();
                    QString relationName = relationData.value("name").toString();
                    QString e1Label = relationData.value("e1Label").toString();
                    QString e2Label = relationData.value("e2Label").toString();
                    QString edgeLabel = relationData.value("edgeLabel").toString();

                    QString entity1_Name = relationData.value("entity1").toString();
                    MSF* entity1 = msf->map.value(entity1_Name);
                    QString entity2_Name = relationData.value("entity2").toString();
                    MSF* entity2 = msf->map.value(entity2_Name);
                    MSF* edge = NULL;
                    if(!(relationData.value("edge").isNull())) {
                        QString edge_Name = relationData.value("edge").toString();
                        edge = msf->map.value(edge_Name);
                    }
                    Relation* relation = new Relation(relationName,entity1,e1Label,entity2,e2Label,edge,edgeLabel);
                    msf->relationVector.append(relation);
                }
            }
        }
    }
}

bool compare(const Tag *tag1, const Tag *tag2) {
    if(tag1->pos != tag2->pos) {
        return tag1->pos < tag2->pos;
    }
    else {
        return tag1->tagtype->name < tag2->tagtype->name;
    }
}

void AMTMainWindow::applyTags(int basic) {

    startTaggingText(_atagger->text);
    if(basic == 0) {
        /*
        for(int j=1; j <= _atagger->wordCount; j++) {
            QMultiHash<int, Tag*>::iterator i = _atagger->tagHash.find(j);
             while (i != _atagger->tagHash.end() && i.key() == j) {
                 int start = i.value().pos;
                 int length = i.value().length;
                 QColor bgcolor(i.value().tagtype->bgcolor);
                 QColor fgcolor(i.value().tagtype->fgcolor);
                 int font = i.value().tagtype->font;
                 bool underline = false;
                 int count = _atagger->tagHash.count(j);
                 if(count > 1) {
                     underline = true;
                 }
                 bool bold = i.value().tagtype->bold;
                 bool italic = i.value().tagtype->italic;
                 tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold);
                 ++i;
             }
        }
        */

        QList<int> keys = _atagger->tagHash.uniqueKeys();
        for(int i=0; i<keys.count(); i++) {
            QList<Tag*> values = _atagger->tagHash.values(keys[i]);
            for(int j = 0; j<values.size(); j++) {
                int start = values.at(j)->pos;
                int length = values.at(j)->length;
                QColor bgcolor(values.at(j)->tagtype->bgcolor);
                QColor fgcolor(values.at(j)->tagtype->fgcolor);
                int font = values.at(j)->tagtype->font;
                bool underline = false;
                if(values.count() > 1) {
                    underline = true;
                }
                bool bold = values.at(j)->tagtype->bold;
                bool italic = values.at(j)->tagtype->italic;
                tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold);
            }
        }
        fillTreeWidget(user);
        createTagMenu();
        createUntagMenu();
    }
    else {
        for(int i=0; i<_atagger->simulationVector.count(); i++) {
            const MERFTag* tag = (const MERFTag*)_atagger->simulationVector.at(i);
            MSFormula* formula = tag->formula;
            tagWord(tag->pos,tag->length,formula->fgcolor,formula->bgcolor,12,false,false,false);
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

    /** Output the user-defined relation matches in a file **/
    if(_atagger->simulationVector.isEmpty()) {
        return;
    }
    QString path = fileName.left(fileName.lastIndexOf('/')+1);
    QString relationMatchFile = path + "relation_match.txt";
    QFile rfile(relationMatchFile);
    if (!rfile.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this,"Warning","Can't open relation match file to Save");
        return false;
    }
    QTextStream outrelations(&rfile);
    for(int i=0; i<_atagger->simulationVector.count(); i++) {
        MERFTag* merftag = (MERFTag*)(_atagger->simulationVector[i]);
        for(int j=0; j<merftag->relationMatchVector.count(); j++) {
            outrelations << merftag->relationMatchVector.at(j)->e1Label
                    << " , "
                    << merftag->relationMatchVector.at(j)->edgeLabel
                    << " , "
                    << merftag->relationMatchVector.at(j)->e2Label
                    << '\n';
        }
    }
    rfile.close();

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

    if(_atagger->tagHash.isEmpty()) {
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

        QTextCursor cursor = myTC;
        int start = cursor.selectionStart();
        int wordIndex;
        if(_atagger->wordIndexMap.contains(start)) {
            wordIndex = _atagger->wordIndexMap.value(start);
        }
        else {
            QMessageBox::warning(this,"Warning","Invalid tag: doesn't start by a word");
            return;
        }
        int length = cursor.selectionEnd() - cursor.selectionStart();
        if(_atagger->isTagMBF) {
            /// MBF based tags
            const TagType* type = NULL;
            for(int i=0;i<_atagger->tagTypeVector->count(); i++) {
                if(_atagger->tagTypeVector->at(i)->name == tagValue) {
                    type = _atagger->tagTypeVector->at(i);
                    break;
                }
            }
            if(type == NULL) {
                QMessageBox::warning(this,"warning","Tag type not found!");
                return;
            }
            _atagger->insertTag(type, start, length, wordIndex, user, original);
            QList<Tag*> values = _atagger->tagHash.values(wordIndex);

            QColor bgcolor(values.at(0)->tagtype->bgcolor);
            QColor fgcolor(values.at(0)->tagtype->fgcolor);
            int font = values.at(0)->tagtype->font;
            bool bold = values.at(0)->tagtype->bold;
            bool italic = values.at(0)->tagtype->italic;

            if(values.size() > 1) {
                tagWord(start,length,fgcolor,bgcolor,font,true,italic,bold);
            }
            else {
                tagWord(start,length,fgcolor,bgcolor,font,false,italic,bold);
            }
            cursor.clearSelection();
            fillTreeWidget(user);
        }
        else {
            /// MSF based tags
        }
        dirty = true;
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
    int wordIndex;
    if(_atagger->wordIndexMap.contains(start)) {
        wordIndex = _atagger->wordIndexMap.value(start);
    }
    else {
        QMessageBox::warning(this,"Warning","Invalid tag: doesn't start by a word");
        return;
    }
    int length = cursor.selectionEnd() - cursor.selectionStart();
    if(length <= 0) {
        return;
    }

    if(_atagger->isTagMBF) {
        /// MBF based tags
        QList<Tag*> values = _atagger->tagHash.values(wordIndex);
        for(int i=0; i<values.size(); i++) {
            if(values.at(i)->tagtype->name == tagValue) {
                delete (values[i]);
                _atagger->tagHash.remove(wordIndex,values[i]);
            }
        }

        QList<Tag*> _values = _atagger->tagHash.values(wordIndex);
        if(_values.count() > 0) {
                QColor bgcolor(_values.at(0)->tagtype->bgcolor);
                QColor fgcolor(_values.at(0)->tagtype->fgcolor);
                int font = _values.at(0)->tagtype->font;
                bool underline = false;
                if(_values.count() > 1) {
                    underline = true;
                }
                bool bold = _values.at(0)->tagtype->bold;
                bool italic = _values.at(0)->tagtype->italic;
                tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold);
        }
        else {
            tagWord(start,length,QColor("black"),QColor("white"),9,false,false,false);
        }
        cursor.clearSelection();
        fillTreeWidget(user);
        dirty = true;
    }
    else {
        /// MSF based tags
        for(int i=0; i<_atagger->simulationVector.count(); i++) {
            const MERFTag* merftag = (const MERFTag*)_atagger->simulationVector.at(i);
            MSFormula* formula = merftag->formula;
            if(merftag->pos == start && formula->name == tagValue) {
                delete _atagger->simulationVector.at(i);
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
    if(_atagger->tagHash.isEmpty()) {
        return;
    }
    startTaggingText(_atagger->text);
    scene->clear();
    descBrwsr->clear();

    QMultiHash<int,Tag*> & th = _atagger->tagHash;
    QMultiHash<int, Tag*>::iterator i = th.begin();
    int lastkey = -1;
    while(i != th.end()) {
        int key = i.key();
        Tag* tag = i.value();
        const TagType* tt = tag->tagtype;
        if(key == lastkey) {
            int start = tag->pos;
            int length = tag->length;
            QColor bgcolor(tt->bgcolor);
            QColor fgcolor(tt->fgcolor);
            int font = tt->font;
            bool bold = tt->bold;
            bool italic = tt->italic;
            tagWord(start,length,fgcolor,bgcolor,font,true,italic,bold);
            do {
                i++;
            }
            while(i.key() == key);
        }
        else {
            int start = tag->pos;
            int length = tag->length;
            QColor bgcolor(tt->bgcolor);
            QColor fgcolor(tt->fgcolor);
            int font = tt->font;
            bool bold = tt->bold;
            bool italic = tt->italic;
            tagWord(start,length,fgcolor,bgcolor,font,false,italic,bold);
            lastkey = key;
            i++;
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
        const MERFTag* tag =(const MERFTag*)_atagger->simulationVector.at(i);
        MSFormula* formula = tag->formula;
        tagWord(tag->pos,tag->length,formula->fgcolor,formula->bgcolor,12,false,false,false);
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
        taginstance = new QAction((_atagger->tagTypeVector->at(i))->name,this);
        signalMapperM->setMapping(taginstance, (_atagger->tagTypeVector->at(i))->name);
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
        taginstance = new QAction((_atagger->tagTypeVector->at(i))->name,this);
        signalMapperUM->setMapping(taginstance, (_atagger->tagTypeVector->at(i))->name);
        connect(taginstance, SIGNAL(triggered()), signalMapperUM, SLOT(map()));
        umTags->addAction(taginstance);
    }
    connect(signalMapperUM, SIGNAL(mapped(const QString &)), this, SLOT(untag(QString)));
}

void AMTMainWindow::fillTreeWidget(Source Data, int basic) {
    tagDescription->clear();
     QList<QTreeWidgetItem *> items;
     int wordCountCharCount = QString::number(_atagger->wordCount).size();
     if(basic == 0) {
         QHashIterator<int, Tag*> iTag(_atagger->tagHash);
         while (iTag.hasNext()) {
             iTag.next();
             QStringList entry;
             QString wordIndexString = QString::number(iTag.key());
             int wordIndexCharCount = wordIndexString.size();
             while(wordIndexCharCount < wordCountCharCount) {
                 wordIndexString.prepend('0');
                 wordIndexCharCount++;
             }
             entry << wordIndexString;
             QTextCursor cursor = txtBrwsr->textCursor();
             int pos = iTag.value()->pos;
             int length = iTag.value()->length;
             cursor.setPosition(pos,QTextCursor::MoveAnchor);
             cursor.setPosition(pos + length,QTextCursor::KeepAnchor);
             QString text = cursor.selectedText();
             entry<<text;
             entry<<iTag.value()->tagtype->name;
             QString src;
             if(iTag.value()->source == user) {
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
         for(int i=0; i<_atagger->simulationVector.count(); i++) {
             const MERFTag* merftag = (const MERFTag*)_atagger->simulationVector.at(i);
             MSFormula* formula = merftag->formula;
             QStringList entry;
             int wordIndex = _atagger->wordIndexMap.value(merftag->pos);
             QString wordIndexString = QString::number(wordIndex);
             int wordIndexCharCount = wordIndexString.size();
             while(wordIndexCharCount < wordCountCharCount) {
                 wordIndexString.prepend('0');
                 wordIndexCharCount++;
             }
             entry << wordIndexString;
             QTextCursor cursor = txtBrwsr->textCursor();
             int pos = merftag->pos;
             int length = merftag->length;
             cursor.setPosition(pos,QTextCursor::MoveAnchor);
             cursor.setPosition(pos + length,QTextCursor::KeepAnchor);
             QString text = cursor.selectedText();
             entry<<text;
             entry<<formula->name;
             entry<<"MERF";
             entry<<QString::number(pos);
             entry<<QString::number(length);
             items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
         }
     }
     tagDescription->insertTopLevelItems(0, items);
     tagDescription->sortByColumn(0,Qt::AscendingOrder);
}

void AMTMainWindow::itemSelectionChanged(QTreeWidgetItem* item ,int i) {
    descBrwsr->clear();
    scene->clear();
    relationScene->clear();
    txtBrwsr->textCursor().clearSelection();
    QList<QTreeWidgetItem *> items;
    QString type = item->text(2);
    bool done = false;

    for(int j=0; j < _atagger->tagTypeVector->count(); j++) {
        if((_atagger->tagTypeVector->at(j))->name == type) {
            QString desc = (_atagger->tagTypeVector->at(j))->description;
            QColor bgcolor((_atagger->tagTypeVector->at(j))->bgcolor);
            QColor fgcolor((_atagger->tagTypeVector->at(j))->fgcolor);
            int font = (_atagger->tagTypeVector->at(j))->font;
            //bool underline = (_atagger->tagTypeVector->at(j))->underline;
            //bool underline = false;
            bool bold = (_atagger->tagTypeVector->at(j))->bold;
            bool italic = (_atagger->tagTypeVector->at(j))->italic;

            QTextCursor c = txtBrwsr->textCursor();
            c.setPosition(item->text(4).toInt(),QTextCursor::MoveAnchor);
            c.setPosition(item->text(4).toInt() + item->text(5).toInt(),QTextCursor::KeepAnchor);
            txtBrwsr->setTextCursor(c);

            QStringList entry;
            entry << "Word" << item->text(1);
            items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
            entry.clear();
            entry << "Description" <<desc;
            items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
            entry.clear();
            entry << "TagType" << type;
            items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
            entry.clear();
            entry << "Source" << item->text(3);
            items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
            entry.clear();
            entry << "Position" << item->text(4);
            items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
            entry.clear();
            entry << "Length" << item->text(5);
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
            c.setPosition(item->text(4).toInt(),QTextCursor::MoveAnchor);
            c.setPosition(item->text(4).toInt() + item->text(5).toInt(),QTextCursor::KeepAnchor);
            txtBrwsr->setTextCursor(c);

            QStringList entry;
            entry << "Text" << item->text(1);
            items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
            entry.clear();
            entry << "Description" <<desc;
            items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
            entry.clear();
            entry << "Formula" << type;
            items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
            entry.clear();
            entry << "Source" << item->text(3);
            items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
            entry.clear();
            entry << "Position" << item->text(4);
            items.append(new QTreeWidgetItem((QTreeWidget*)0, entry));
            entry.clear();
            entry << "Length" << item->text(5);
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
            const MERFTag* merftag = (const MERFTag*)_atagger->simulationVector.at(index);
            merftag->match->buildMatchTree(G,tempNode,edge,&parentNode,tempItem,id);

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
                            if(edgeLabel == tt->name) {
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


            /** Create the entity-relation graph of graphviz
                We use this graph to create the ER graph
                and calculate the coordinates of each node for better visualization **/
            gvc = gvContext();
            edge = NULL;
            gvParseArgs(gvc, sizeof(args)/sizeof(char*), args);
            G = agopen(const_cast<char *>("matchGraph"), Agstrictdirected, 0);
            //Set graph attributes
            agsafeset(G, const_cast<char *>("overlap"), const_cast<char *>("prism"),const_cast<char *>(""));
            agsafeset(G, const_cast<char *>("splines"), const_cast<char *>("true"),const_cast<char *>("true"));
            agsafeset(G, const_cast<char *>("pad"), const_cast<char *>("0,2"),const_cast<char *>("0,2"));
            agsafeset(G, const_cast<char *>("dpi"), const_cast<char *>("96,0"),const_cast<char *>("96,0"));
            agsafeset(G, const_cast<char *>("nodesep"), const_cast<char *>("0,4"),const_cast<char *>("0,4"));
            agattr(G,AGNODE,const_cast<char *>("label"),const_cast<char *>(""));
            agattr(G,AGNODE,const_cast<char *>("fixedsize"), const_cast<char *>("true"));
            agattr(G,AGNODE,const_cast<char *>("regular"), const_cast<char *>("true"));
            id=0;

            QHash<QString, Agnode_t*> rmnHash;
            //QMap<Agnode_t*,Agnode_t*> edgesMap;
            QSet<Triplet<Agnode_t*,Agnode_t*,QString>*> nodeedgeSet;
            for(int k=0; k<merftag->relationMatchVector.count(); k++) {
                RelationM* relMatch = merftag->relationMatchVector[k];
                Agnode_t* node1;
                QString e1ID = relMatch->entity1->msf->name;
                e1ID.append(QString::number(relMatch->entity1->getPOS()));
                if(rmnHash.contains(e1ID)) {
                    node1 = rmnHash.value(e1ID);
                }
                else {
                    stringstream strs;
                    strs << id;
                    string temp_str = strs.str();
                    char* nodeID = strdup(temp_str.c_str());
                    node1 = agnode(G,nodeID, 1);
                    //agset(reG,const_cast<char *>("root"),nodeID);
                    id = id+1;
                    char * writable = strdup(relMatch->e1Label.toStdString().c_str());
                    agset(node1,const_cast<char *>("label"),writable);
                    rmnHash.insert(e1ID,node1);
                }

                Agnode_t* node2;
                QString e2ID = relMatch->entity2->msf->name;
                e2ID.append(QString::number(relMatch->entity2->getPOS()));
                if(rmnHash.contains(e2ID)) {
                    node2 = rmnHash.value(e2ID);
                }
                else {
                    stringstream strs;
                    strs << id;
                    string temp_str = strs.str();
                    char* nodeID = strdup(temp_str.c_str());
                    node2 = agnode(G,nodeID, 1);
                    //agset(reG,const_cast<char *>("root"),nodeID);
                    id = id+1;
                    char * writable = strdup(relMatch->e2Label.toStdString().c_str());
                    agset(node2,const_cast<char *>("label"),writable);
                    rmnHash.insert(e2ID,node2);
                }

                edge = agedge(G, node1, node2, 0, 1);
                Triplet<Agnode_t*,Agnode_t*,QString>* triplet = new Triplet<Agnode_t*,Agnode_t*,QString>(node1,node2,relMatch->edgeLabel);
                nodeedgeSet.insert(triplet);
                //edgesMap.insert(node1,node2);
            }

            /** Set layout of the graph and get coordinates **/
            /* Compute a layout using layout engine from command line args */
            gvLayoutJobs(gvc, G);
            //gvRenderJobs(gvc, G);

            /** Set scene rectangle **/
            left = GD_bb(G).LL.x;
            top = GD_bb(G).LL.y;
            width = GD_bb(G).UR.x;
            height = GD_bb(G).UR.y;
            QRectF rerect(left,top,width,height);
            relationScene->setSceneRect(rerect);

            /** Get coordinates of nodes and draw nodes in scene **/
            QMap<Agnode_t*,GraphNode *> renodes;
            QSetIterator<Triplet<Agnode_t *,Agnode_t *,QString>*> reiterator(nodeedgeSet);
            while (reiterator.hasNext()) {
                Triplet<Agnode_t*,Agnode_t*,QString>* triplet = reiterator.next();
                /// Use this string to know if an edge label should be added
                QString edgeLabel(triplet->third);

                Agnode_t* tmpNode1 = triplet->first;
                char* label1 = agget(tmpNode1,const_cast<char *>("label"));
                qreal nodeX1 = ND_coord(tmpNode1).x;
                qreal nodeY1 = (GD_bb(G).UR.y - ND_coord(tmpNode1).y);
                GraphNode *node1;
                if(!(renodes.contains(tmpNode1))) {
                    QString stringlabel1(label1);
                    node1 = new GraphNode(stringlabel1,"","");
                    relationScene->addItem(node1);
                    node1->setPos(nodeX1,nodeY1);
                    renodes.insert(tmpNode1,node1);
                }
                else {
                    node1 = renodes.value(tmpNode1);
                }

                Agnode_t* tmpNode2 = triplet->second;
                if(tmpNode2 == NULL) {
                    continue;
                }
                char* label2 = agget(tmpNode2,const_cast<char *>("label"));
                qreal nodeX2 = ND_coord(tmpNode2).x;
                qreal nodeY2 = (GD_bb(G).UR.y - ND_coord(tmpNode2).y);
                GraphNode *node2;
                if(!(renodes.contains(tmpNode2))) {
                    QString stringlabel2(label2);
                    node2 = new GraphNode(stringlabel2,"","");
                    relationScene->addItem(node2);
                    node2->setPos(nodeX2,nodeY2);
                    renodes.insert(tmpNode2,node2);
                    if(edgeLabel.isEmpty()) {
                        relationScene->addItem(new GraphEdge(node2,node1,"",false));
                    }
                    else {
                        relationScene->addItem(new GraphEdge(node2,node1,edgeLabel,false));
                        edgeLabel.clear();
                    }
                }
                else {
                    node2 = renodes.value(tmpNode2);
                    if(edgeLabel.isEmpty()) {
                        relationScene->addItem(new GraphEdge(node2,node1,"",false));
                    }
                    else {
                        relationScene->addItem(new GraphEdge(node2,node1,edgeLabel,false));
                        edgeLabel.clear();
                    }
                }
                delete triplet;
            }
            nodeedgeSet.clear();

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

void AMTMainWindow::sarfTagging(bool color) {

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
    _atagger->tagHash.clear();

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

    if(color) {
        applyTags();

        fillTreeWidget(sarf);
        finishTaggingText();
    }
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

    _atagger->compareToTagFile.clear();
    _atagger->compareToTagTypeFile.clear();
    _atagger->compareToTagTypeVector->clear();
    _atagger->compareToTagHash.clear();
    if(_atagger->tagHash.isEmpty()) {
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

    QStringList dirList = _atagger->compareToTagFile.split('/');
    dirList.removeLast();
    QString dir = dirList.join("/");
    dir.append('/');

    QString textFileName = result["file"].toString();

    QString textPath = dir;
    textPath.append(textFileName);

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

    /** Read the TagType file and store it **/

    _atagger->compareToTagTypeFile = tagtypeFile;

    QStringList ttdirList = _atagger->compareToTagFile.split('/');
    ttdirList.removeLast();
    QString ttdir = ttdirList.join("/");
    ttdir.append('/');

    QString ttPath = ttdir;
    ttPath.append(tagtypeFile);

    QFile ITTfile(ttPath);
    if (!ITTfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::about(this, tr("Input Tag File"),
                     tr("The <b>Tag Type File</b> can't be opened!"));
        _atagger->compareToTagFile.clear();
        _atagger->compareToTagTypeFile.clear();
        _atagger->compareToTagHash.clear();
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
        QString foreground_color;
        QString background_color;
        int font;
        bool underline = false;
        bool bold;
        bool italic;

        QVariantMap typeElements = type.toMap();

        tag = typeElements["Tag"].toString();
        desc = typeElements["Description"].toString();
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
            _atagger->insertSarfTagType(tag,tags,desc,foreground_color,background_color,font,underline,bold,italic,sarf,compareTo);
        }
        else {
            _atagger->isSarf = false;
            _atagger->insertTagType(tag,desc,foreground_color,background_color,font,underline,bold,italic,user,compareTo);
        }
    }

    /** Read Tags **/

    foreach(QVariant tag, result["TagArray"].toList()) {

        QVariantMap tagElements = tag.toMap();
        int start = tagElements["pos"].toInt();
        int length = tagElements["length"].toInt();
        int wordIndex = tagElements["wordIndex"].toInt();
        QString tagtype = tagElements["type"].toString();
        Source source = (Source)(tagElements["source"].toInt());
        bool check;
        const TagType* type = NULL;
        for(int i=0;i<_atagger->compareToTagTypeVector->count(); i++) {
            if(_atagger->compareToTagTypeVector->at(i)->name == tagtype) {
                type = _atagger->compareToTagTypeVector->at(i);
                break;
            }
        }
        if(type == NULL) {
            QMessageBox::warning(this,"warning","Something went wrong in tag file processing!");
            _atagger = new ATagger();
            clearLayout(this->layout());
            return;
        }
        check = _atagger->insertTag(type,start,length,wordIndex,source,compareTo);
    }

    if(_atagger->tagHash.begin().value()->source == user) {
        _atagger->compareToIsSarf = false;
    }
    else {
        _atagger->compareToIsSarf = true;
    }
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

         _atagger->tagHash.clear();
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
         processText(&text);
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

        _atagger->tagHash.clear();
        _atagger->tagTypeVector->clear();
        tagDescription->clear();
        descBrwsr->clear();
        txtBrwsr->clear();
        scene->clear();
        txtBrwsr->setText(_atagger->text);

        lineEditTTFName->setText(fileName);
        QByteArray tagtypes = file.readAll();
        process_TagTypes(tagtypes);

        applyTags();
        //createTagMenu();
    }
}

void AMTMainWindow::_new() {
    if(dirty == true) {
    }
    clearLayout(this->layout());

    _atagger = new ATagger();

    setWindowTitle("Arabic Morphological Tagger");
    static QFileDialog * fdlg = NULL;
    if (fdlg == NULL) {
        fdlg = new QFileDialog(this,
                                 tr("Create Tag File"), "",
                                 tr("tags (*.tags.json);;All Files (*)"));
        fdlg->setLabelText(QFileDialog::Accept,"Create");
        fdlg->setViewMode(QFileDialog::Detail);
    }
    if (!fdlg->exec()) {
        return;
    }
    if (fdlg->selectedFiles().empty()) {
        return;
    }
    QString tagFileName = fdlg->selectedFiles()[0];
#if 0
    QString tagFileName = QFileDialog::getSaveFileName(this,
                    tr("Create Tag File"), "",
                    tr("tags (*.tags.json);;All Files (*)"));
    if(tagFileName.isEmpty()) {
            return;
    }
#endif
    _atagger->tagFile = tagFileName + ".tags.json";
    setWindowTitle("MERF: " + _atagger->tagFile);


    createDockWindows(false);

    sarftagsAct->setEnabled(true);
    editMSFAct->setEnabled(true);
    edittagtypesAct->setEnabled(true);
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

bool compareTag(const Tag &tag1, const Tag &tag2) {
    if(tag1.pos != tag2.pos) {
        return tag1.pos < tag2.pos;
    }
    else {
        return tag1.tagtype->name < tag2.tagtype->name;
    }
}

void AMTMainWindow::runMERFSimulator() {
    if(_atagger->msfVector->isEmpty()) {
        return;
    }

    if(_atagger->tagTypeVector->isEmpty()) {
        QMessageBox::warning(this,"Warning","No tag types with MBF defined!");
        return;
    }

    _atagger->simulationVector.clear();
    descBrwsr->clear();
    /*
    timeval tim;
    gettimeofday(&tim,NULL);
    double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
    */
    if(_atagger->tagHash.isEmpty()) {
        sarfTagging(false);
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

    if(!(_atagger->runSimulator())) {
        return;
    }
    /*
    gettimeofday(&tim, NULL);
    double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
    double diff = t2-t1;
    */

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
             event->ignore();
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

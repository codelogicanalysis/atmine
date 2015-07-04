#include "diffview.h"
#include "global.h"
#include "sarftagtype.h"
#include <QGridLayout>
#include <QDockWidget>
#include <QScrollArea>
#include <QSplitter>
#include <QtAlgorithms>
#include <QMessageBox>

bool compareTags(const Tag *tag1, const Tag *tag2) {
    if(tag1->pos != tag2->pos) {
        return tag1->pos < tag2->pos;
    }
    else {
        return tag1->tagtype->name < tag2->tagtype->name;
    }
}

bool compareTagTypes(const TagType *tt1, const TagType *tt2) {
    return (tt1->name < tt2->name);
}

DiffView::DiffView(QWidget *parent) :
    QMainWindow(parent)
{
    QDockWidget *dock = new QDockWidget();
    dock->setAllowedAreas(Qt::LeftDockWidgetArea);
    lblCommon = new QLabel(tr("Common"));
    txtCommon = new QTextBrowser();
    txtCommon->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(txtCommon,SIGNAL(customContextMenuRequested(const QPoint&)), this,SLOT(showContextMenuCommon(const QPoint&)));
    QVBoxLayout *vb1 = new QVBoxLayout;
    vb1->setAlignment(Qt::AlignCenter);
    vb1->addWidget(lblCommon);
    vb1->addWidget(txtCommon);
    QScrollArea *sac = new QScrollArea;
    sac->setLayout(vb1);
    QString first = _atagger->tagtypeFile.split('/').last();
    first = first.remove(QRegExp("(.stt.json|.tt.json)"));
    QString second = _atagger->compareToTagTypeFile.split('/').last();
    second = second.remove(QRegExp("(.stt.json|.tt.json)"));
    QString AB = first;
    AB.append(" - ");
    AB.append(second);
    lblForwardDiff = new QLabel(AB);
    txtForwardDiff = new QTextBrowser();
    txtForwardDiff->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(txtForwardDiff,SIGNAL(customContextMenuRequested(const QPoint&)), this,SLOT(showContextMenuForward(const QPoint&)));
    QVBoxLayout *vb2 = new QVBoxLayout;
    vb2->setAlignment(Qt::AlignCenter);
    vb2->addWidget(lblForwardDiff);
    vb2->addWidget(txtForwardDiff);
    QScrollArea *saf = new QScrollArea;
    saf->setLayout(vb2);
    QString BA = second;
    BA.append(" - ");
    BA.append(first);
    lblReverseDiff = new QLabel(BA);
    txtReverseDiff = new QTextBrowser();
    txtReverseDiff->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(txtReverseDiff,SIGNAL(customContextMenuRequested(const QPoint&)), this,SLOT(showContextMenuReverse(const QPoint&)));
    QVBoxLayout *vb3 = new QVBoxLayout;
    vb3->setAlignment(Qt::AlignCenter);
    vb3->addWidget(lblReverseDiff);
    vb3->addWidget(txtReverseDiff);
    QScrollArea *sar = new QScrollArea;
    sar->setLayout(vb3);
    QSplitter *splitter = new QSplitter(dock);
    splitter->addWidget(sac);
    splitter->addWidget(saf);
    splitter->addWidget(sar);
    dock->setWidget(splitter);
    addDockWidget(Qt::TopDockWidgetArea, dock);

    dock = new QDockWidget(tr("Options"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea);
    QHBoxLayout *vboxmain = new QHBoxLayout;
    gb1 = new QGroupBox(tr("Difference"));
    rbTTDiff = new QRadioButton(tr("TagType"));
    connect(rbTTDiff,SIGNAL(clicked()),this,SLOT(rbTagTypes_clicked()));
    rbTDiff = new QRadioButton(tr("Tag"));
    connect(rbTDiff,SIGNAL(clicked()),this,SLOT(rbTags_clicked()));
    rbTTDiff->setChecked(true);
    QVBoxLayout *vbox1 = new QVBoxLayout;
    vbox1->addWidget(rbTTDiff);
    vbox1->addWidget(rbTDiff);
    gb1->setLayout(vbox1);
    vboxmain->addWidget(gb1);
    gb2 = new QGroupBox(tr("Criteria"));
    rbExact = new QRadioButton(tr("Exact"));
    connect(rbExact,SIGNAL(clicked()),this,SLOT(rbExact_clicked()));
    rbIntersect = new QRadioButton(tr("Intersect"));
    connect(rbIntersect,SIGNAL(clicked()),this,SLOT(rbIntersect_clicked()));
    rbAContainB = new QRadioButton(tr("A include B"));
    connect(rbAContainB,SIGNAL(clicked()),this,SLOT(rbAContainB_clicked()));
    rbBContainA = new QRadioButton(tr("B include A"));
    connect(rbBContainA,SIGNAL(clicked()),this,SLOT(rbBContainA_clicked()));
    rbIntersect->setChecked(true);
    gb2->setEnabled(false);
    QVBoxLayout *vbox2 = new QVBoxLayout;
    vbox2->addWidget(rbIntersect);
    vbox2->addWidget(rbExact);
    vbox2->addWidget(rbAContainB);
    vbox2->addWidget(rbBContainA);
    gb2->setLayout(vbox2);
    vboxmain->addWidget(gb2);
    QScrollArea *sa1 = new QScrollArea(dock);
    sa1->setLayout(vboxmain);
    sa1->setMaximumWidth(300);
    dock->setWidget(sa1);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    dock = new QDockWidget(tr("Statistics"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea);
    txtStats = new QTextBrowser(dock);
    dock->setWidget(txtStats);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    /** Initialize Actions **/
    createActions();
    /** End **/

    setWindowTitle(tr("Analysis"));
    resize(800,600);

    /** Sort the Tags in both Tag Vectors **/
    /*
    qSort(_atagger->tagVector.begin(), _atagger->tagVector.end(), compareTags);
    qSort(_atagger->compareToTagVector.begin(), _atagger->compareToTagVector.end(), compareTags);
    */

    /** Copy two tag vectors **/
    tVector = new QVector<Tag*>();
    cttVector = new QVector<Tag*>();

    QHashIterator<int, Tag*> iTag(*(_atagger->tagHash));
    while (iTag.hasNext()) {
        iTag.next();
        Tag* t = new Tag(iTag.value()->tagtype,
                         iTag.value()->pos,
                         iTag.value()->length,
                         iTag.value()->wordIndex,
                         iTag.value()->source,
                         iTag.value()->id);
        tVector->append(t);
    }

    QHashIterator<int, Tag*> iCompareToTag(_atagger->compareToTagHash);
    while (iCompareToTag.hasNext()) {
        iCompareToTag.next();
        Tag* t = new Tag(iCompareToTag.value()->tagtype,
                         iCompareToTag.value()->pos,
                         iCompareToTag.value()->length,
                         iCompareToTag.value()->wordIndex,
                         iCompareToTag.value()->source,
                         iCompareToTag.value()->id);
        cttVector->append(t);
    }

    qSort(tVector->begin(),tVector->end(),compareTags);
    qSort(cttVector->begin(),cttVector->end(),compareTags);

    dirty = false;

    /** Analyze tagtype difference **/

    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        const TagType * stt = (TagType*)(_atagger->tagTypeVector->at(i));
        bool found = false;
        for(int j=0; j<_atagger->compareToTagTypeVector->count();j++) {
            const TagType * ctt = (TagType*)(_atagger->compareToTagTypeVector->at(j));
            if(stt->name == ctt->name) {
                found = true;
                commonTT.append(stt->name);
            }
        }
        if(!found) {
            forwardTT.append(stt->name);
        }
    }

    for(int i=0; i<_atagger->compareToTagTypeVector->count();i++) {
        const TagType * ctt = (TagType*)(_atagger->tagTypeVector->at(i));
        if(!(commonTT.contains(ctt->name))){
            reverseTT.append(ctt->name);
        }
    }

    QString text;
    for(int i=0; i<commonTT.count(); i++) {
        text += commonTT[i] + '\n';
    }
    txtCommon->setText(text);

    text.clear();
    for(int i=0; i<forwardTT.count(); i++) {
        text += forwardTT[i] + '\n';
    }
    txtForwardDiff->setText(text);

    text.clear();
    for(int i=0; i<reverseTT.count(); i++) {
        text += reverseTT[i] + '\n';
    }
    txtReverseDiff->setText(text);
}

void DiffView::startTaggingText(QString & text) {
    QTextBrowser * taggedBox=txtCommon;
    taggedBox->clear();
    taggedBox->setLayoutDirection(Qt::RightToLeft);
    QTextCursor c=taggedBox->textCursor();
    c.clearSelection();
    c.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
    taggedBox->setTextCursor(c);
    taggedBox->setTextBackgroundColor(Qt::white);
    taggedBox->setTextColor(Qt::black);
    taggedBox->setText(text);

    taggedBox=txtForwardDiff;
    taggedBox->clear();
    taggedBox->setLayoutDirection(Qt::RightToLeft);
    c=taggedBox->textCursor();
    c.clearSelection();
    c.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
    taggedBox->setTextCursor(c);
    taggedBox->setTextBackgroundColor(Qt::white);
    taggedBox->setTextColor(Qt::black);
    taggedBox->setText(text);

    taggedBox=txtReverseDiff;
    taggedBox->clear();
    taggedBox->setLayoutDirection(Qt::RightToLeft);
    c=taggedBox->textCursor();
    c.clearSelection();
    c.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
    taggedBox->setTextCursor(c);
    taggedBox->setTextBackgroundColor(Qt::white);
    taggedBox->setTextColor(Qt::black);
    taggedBox->setText(text);
}

void DiffView::tagWord(int start, int length, QColor fcolor, QColor  bcolor, int font, bool underline, bool italic, bool bold, DestText dt){
    QTextBrowser * taggedBox;
    if(dt == common) {
        taggedBox = txtCommon;
    } else if (dt == ::forward) {
        taggedBox = txtForwardDiff;
    }
    else {
        taggedBox = txtReverseDiff;
    }
    QTextCursor c=taggedBox->textCursor();
    c.setPosition(start,QTextCursor::MoveAnchor);
    c.setPosition(start+length,QTextCursor::KeepAnchor);
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
}

void DiffView::finishTaggingText() {
    QTextBrowser * taggedBox= txtCommon;
    QTextCursor c=taggedBox->textCursor();
    c.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
    taggedBox->setTextCursor(c);

    taggedBox= txtForwardDiff;
    c=taggedBox->textCursor();
    c.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
    taggedBox->setTextCursor(c);

    taggedBox= txtReverseDiff;
    c=taggedBox->textCursor();
    c.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
    taggedBox->setTextCursor(c);
}

void DiffView::rbTagTypes_clicked() {
    gb2->setEnabled(false);
    rbIntersect->setChecked(true);

    txtCommon->clear();
    txtForwardDiff->clear();
    txtReverseDiff->clear();
    txtStats->clear();

    commonTT.clear();
    forwardTT.clear();
    reverseTT.clear();

    QString first = _atagger->tagtypeFile.split('/').last();
    first = first.remove(QRegExp("(.stt.json|.tt.json)"));
    QString second = _atagger->compareToTagTypeFile.split('/').last();
    second = second.remove(QRegExp("(.stt.json|.tt.json)"));
    QString AB = first;
    AB.append(" - ");
    AB.append(second);
    lblForwardDiff->setText(AB);

    QString BA = second;
    BA.append(" - ");
    BA.append(first);
    lblReverseDiff->setText(BA);

    /** Analyze tagtype difference **/

    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        const TagType * stt = (TagType*)(_atagger->tagTypeVector->at(i));
        bool found = false;
        for(int j=0; j<_atagger->compareToTagTypeVector->count();j++) {
            const TagType * ctt = (TagType*)(_atagger->compareToTagTypeVector->at(j));
            if(stt->name == ctt->name) {
                found = true;
                commonTT.append(stt->name);
            }
        }
        if(!found) {
            forwardTT.append(stt->name);
        }
    }

    for(int i=0; i<_atagger->compareToTagTypeVector->count();i++) {
        const TagType * ctt = (TagType*)(_atagger->tagTypeVector->at(i));
        if(!(commonTT.contains(ctt->name))){
            reverseTT.append(ctt->name);
        }
    }

    QString text;
    for(int i=0; i<commonTT.count(); i++) {
        text += commonTT[i] + '\n';
    }
    txtCommon->setText(text);

    text.clear();
    for(int i=0; i<forwardTT.count(); i++) {
        text += forwardTT[i] + '\n';
    }
    txtForwardDiff->setText(text);

    text.clear();
    for(int i=0; i<reverseTT.count(); i++) {
        text += reverseTT[i] + '\n';
    }
    txtReverseDiff->setText(text);
}

void DiffView::rbTags_clicked() {
    gb2->setEnabled(true);
    this->rbIntersect_clicked();

    QString first = _atagger->tagFile.split('/').last();
    first = first.remove(".tags.json");
    QString second = _atagger->compareToTagFile.split('/').last();
    second = second.remove(".tags.json");
    QString AB = first;
    AB.append(" - ");
    AB.append(second);
    lblForwardDiff->setText(AB);

    QString BA = second;
    BA.append(" - ");
    BA.append(first);
    lblReverseDiff->setText(BA);
}

void DiffView::rbExact_clicked() {
    txtCommon->clear();
    txtForwardDiff->clear();
    txtReverseDiff->clear();
    txtStats->clear();

    /** Analyze Tags based on Exact Match **/

    commonVector.clear();
    forwardVector.clear();
    reverseVector.clear();

    // apply an algorithm similar to mergesort
    int otlength = tVector->count();
    int ctlength = cttVector->count();
    //int count = otlength + ctlength;

    int c1=0;
    int c2=0;

    while((c1 < otlength) && (c2 < ctlength)) {
        Tag * ott = tVector->at(c1);
        Tag * ctt = cttVector->at(c2);
        if(((ott->pos) == (ctt->pos)) && ((ott->length) == (ctt->length))) {
            if((ott->tagtype->name) == (ctt->tagtype->name))  {
                commonVector.append(ott);
                c1++;
                c2++;
            }
            else if((ott->tagtype->name) > (ctt->tagtype->name)){
                reverseVector.append(ctt);
                c2++;
            }
            else {
                forwardVector.append(ott);
                c1++;
            }
        }
        else if((ott->pos) > (ctt->pos)) {
            reverseVector.append(ctt);
            c2++;
        }
        else {
            forwardVector.append(ott);
            c1++;
        }
    }
    if(c1 < (otlength-1)) {
        for(int i=c1; i< otlength; i++) {
            Tag * ott = tVector->at(i);
            forwardVector.append(ott);
        }
    }
    else if(c2 < (ctlength-1)) {
        for(int i=c2; i< ctlength; i++) {
            Tag * ctt = cttVector->at(i);
            reverseVector.append(ctt);
        }
    }

    /** Tag Words **/
    startTaggingText(_atagger->text);
    addTags(commonVector, forwardVector, reverseVector);
    finishTaggingText();

    double precision = (commonVector.count() * 1.0) / cttVector->count();
    double recall = (commonVector.count() * 1.0) / tVector->count();
    double fmeasure = 2 * (precision * recall) / (precision + recall);

    QString text;
    text.append("Precision, Recall, and F-Measure are done with ");
    QString first = _atagger->tagFile.split('/').last();
    first = first.remove(QRegExp(".tags.json"));
    text.append(first);
    text.append((" as reference:\n"));
    text.append("\nPrecision: ");
    text.append(QString::number(precision));
    text.append("\n\nRecall: ");
    text.append(QString::number(recall));
    text.append("\n\nF-Measure: ");
    text.append(QString::number(fmeasure));

    QHash<QString,int> curTagCount;
    for(int i=0; i< tVector->count(); i++) {
        QString tagName = tVector->at(i)->tagtype->name;
        if(curTagCount.contains(tagName)) {
            int count = curTagCount.value(tagName);
            curTagCount.insert(tagName,count+1);
        }
        else {
            curTagCount.insert(tagName,1);
        }
    }

    QHash<QString,int> refTagCount;
    for(int i=0; i< cttVector->count(); i++) {
        QString tagName = cttVector->at(i)->tagtype->name;
        if(refTagCount.contains(tagName)) {
            int count = refTagCount.value(tagName);
            refTagCount.insert(tagName,count+1);
        }
        else {
            refTagCount.insert(tagName,1);
        }
    }

    QHash<QString,int> tagCount;
    for(int i=0; i< commonVector.count(); i++) {
        QString tagName = commonVector.at(i)->tagtype->name;
        if(tagCount.contains(tagName)) {
            int count = tagCount.value(tagName);
            tagCount.insert(tagName,count+1);
        }
        else {
            tagCount.insert(tagName,1);
        }
    }

    text.append("\n\nPrecision, Recall, and F-Measure per Tag Type:\n");

    QHashIterator<QString, int> it(refTagCount);
    while (it.hasNext()) {
        it.next();

        double pttprecision = (tagCount.value(it.key()) * 1.0) / it.value();
        double pttrecall = (tagCount.value(it.key()) * 1.0) / curTagCount.value(it.key());
        double fmeasure = 2 * (precision * recall) / (precision + recall);

        text.append(it.key() + ":\n");
        text.append("\tPrecision: " + QString::number(pttprecision) + '\n');
        text.append("\tRecall: " + QString::number(pttrecall) + '\n');
        text.append("\tF-Measure: " + QString::number(fmeasure) + '\n');
    }

    txtStats->setText(text);
}

void DiffView::rbIntersect_clicked() {

    txtCommon->clear();
    txtForwardDiff->clear();
    txtReverseDiff->clear();
    txtStats->clear();

    /** Analyze Tags based on A intersect B Match **/

    commonVector.clear();
    forwardVector.clear();
    reverseVector.clear();

    int otlength = tVector->count();
    int ctlength = cttVector->count();

    int c1=0;
    int c2=0;

    while((c1 < otlength) && (c2 < ctlength)) {
        Tag * ott = tVector->at(c1);
        Tag * ctt = cttVector->at(c2);
        if(((ott->pos <= ctt->pos) && ((ott->pos + ott->length) > (ctt->pos))) ||
           ((ctt->pos <= ott->pos) && ((ctt->pos + ctt->length) > (ott->pos)))) {
            if((ott->tagtype->name) == (ctt->tagtype->name)) {
                commonVector.append(ott);
                c1++;
                c2++;
            }
            else if((ott->tagtype->name) > (ctt->tagtype->name)) {
                reverseVector.append(ctt);
                c2++;
            }
            else {
                forwardVector.append(ott);
                c1++;
            }
        }
        else if((ott->pos) > (ctt->pos)) {
            reverseVector.append(ctt);
            c2++;
        }
        else {
            forwardVector.append(ott);
            c1++;
        }
    }
    if(c1 < (otlength-1)) {
        for(int i=c1; i< otlength; i++) {
            Tag * ott = tVector->at(i);
            forwardVector.append(ott);
        }
    }
    else if(c2 < (ctlength-1)) {
        for(int i=c2; i< ctlength; i++) {
            Tag * ctt = cttVector->at(i);
            reverseVector.append(ctt);
        }
    }

    /** Tag Words **/
    startTaggingText(_atagger->text);
    addTags(commonVector, forwardVector, reverseVector);
    finishTaggingText();

    double precision = (commonVector.count() * 1.0) / cttVector->count();
    double recall = (commonVector.count() * 1.0) / tVector->count();
    double fmeasure = 2 * (precision * recall) / (precision + recall);

    QString text;
    text.append("Precision, Recall, and F-Measure are done with ");
    QString first = _atagger->tagFile.split('/').last();
    first = first.remove(QRegExp(".tags.json"));
    text.append(first);
    text.append((" as reference:\n"));
    text.append("\nPrecision: ");
    text.append(QString::number(precision));
    text.append("\n\nRecall: ");
    text.append(QString::number(recall));
    text.append("\n\nF-Measure: ");
    text.append(QString::number(fmeasure));

    QHash<QString,int> curTagCount;
    for(int i=0; i< tVector->count(); i++) {
        QString tagName = tVector->at(i)->tagtype->name;
        if(curTagCount.contains(tagName)) {
            int count = curTagCount.value(tagName);
            curTagCount.insert(tagName,count+1);
        }
        else {
            curTagCount.insert(tagName,1);
        }
    }

    QHash<QString,int> refTagCount;
    for(int i=0; i< cttVector->count(); i++) {
        QString tagName = cttVector->at(i)->tagtype->name;
        if(refTagCount.contains(tagName)) {
            int count = refTagCount.value(tagName);
            refTagCount.insert(tagName,count+1);
        }
        else {
            refTagCount.insert(tagName,1);
        }
    }

    QHash<QString,int> tagCount;
    for(int i=0; i< commonVector.count(); i++) {
        QString tagName = commonVector.at(i)->tagtype->name;
        if(tagCount.contains(tagName)) {
            int count = tagCount.value(tagName);
            tagCount.insert(tagName,count+1);
        }
        else {
            tagCount.insert(tagName,1);
        }
    }

    text.append("\n\nPrecision, Recall, and F-Measure per Tag Type:\n");

    QHashIterator<QString, int> it(refTagCount);
    while (it.hasNext()) {
        it.next();

        double pttprecision = (tagCount.value(it.key()) * 1.0) / it.value();
        double pttrecall = (tagCount.value(it.key()) * 1.0) / curTagCount.value(it.key());
        double fmeasure = 2 * (precision * recall) / (precision + recall);

        text.append(it.key() + ":\n");
        text.append("\tPrecision: " + QString::number(pttprecision) + '\n');
        text.append("\tRecall: " + QString::number(pttrecall) + '\n');
        text.append("\tF-Measure: " + QString::number(fmeasure) + '\n');
    }

    txtStats->setText(text);
}

void DiffView::rbAContainB_clicked() {
    txtCommon->clear();
    txtForwardDiff->clear();
    txtReverseDiff->clear();
    txtStats->clear();

    /** Analyze Tags based on A contain B Match **/

    commonVector.clear();
    forwardVector.clear();
    reverseVector.clear();

    // apply an algorithm similar to mergesort
    int otlength = tVector->count();
    int ctlength = cttVector->count();
    //int count = otlength + ctlength;

    int c1=0;
    int c2=0;

    while((c1 < otlength) && (c2 < ctlength)) {
        Tag * ott = tVector->at(c1);
        Tag * ctt = cttVector->at(c2);
        if(((ott->pos) <= (ctt->pos)) && ((ott->pos + ott->length) >= (ctt->pos + ctt->length))) {
            if((ott->tagtype->name) == (ctt->tagtype->name)) {
                commonVector.append(ott);
                c1++;
                c2++;
            }
            else if((ott->tagtype->name) > (ctt->tagtype->name)) {
                reverseVector.append(ctt);
                c2++;
            }
            else {
                forwardVector.append(ott);
                c1++;
            }
        }
        else if((ott->pos) > (ctt->pos)) {
            reverseVector.append(ctt);
            c2++;
        }
        else {
            forwardVector.append(ott);
            c1++;
        }
    }
    if(c1 < (otlength-1)) {
        for(int i=c1; i< otlength; i++) {
            Tag * ott = tVector->at(i);
            forwardVector.append(ott);
        }
    }
    else if(c2 < (ctlength-1)) {
        for(int i=c2; i< ctlength; i++) {
            Tag * ctt = cttVector->at(i);
            reverseVector.append(ctt);
        }
    }

    /** Tag Words **/
    startTaggingText(_atagger->text);
    addTags(commonVector, forwardVector, reverseVector);
    finishTaggingText();

    double precision = (commonVector.count() * 1.0) / cttVector->count();
    double recall = (commonVector.count() * 1.0) / tVector->count();
    double fmeasure = 2 * (precision * recall) / (precision + recall);

    QString text;
    text.append("Precision, Recall, and F-Measure are done with ");
    QString first = _atagger->tagFile.split('/').last();
    first = first.remove(QRegExp(".tags.json"));
    text.append(first);
    text.append((" as reference:\n"));
    text.append("\nPrecision: ");
    text.append(QString::number(precision));
    text.append("\n\nRecall: ");
    text.append(QString::number(recall));
    text.append("\n\nF-Measure: ");
    text.append(QString::number(fmeasure));

    QHash<QString,int> curTagCount;
    for(int i=0; i< tVector->count(); i++) {
        QString tagName = tVector->at(i)->tagtype->name;
        if(curTagCount.contains(tagName)) {
            int count = curTagCount.value(tagName);
            curTagCount.insert(tagName,count+1);
        }
        else {
            curTagCount.insert(tagName,1);
        }
    }

    QHash<QString,int> refTagCount;
    for(int i=0; i< cttVector->count(); i++) {
        QString tagName = cttVector->at(i)->tagtype->name;
        if(refTagCount.contains(tagName)) {
            int count = refTagCount.value(tagName);
            refTagCount.insert(tagName,count+1);
        }
        else {
            refTagCount.insert(tagName,1);
        }
    }

    QHash<QString,int> tagCount;
    for(int i=0; i< commonVector.count(); i++) {
        QString tagName = commonVector.at(i)->tagtype->name;
        if(tagCount.contains(tagName)) {
            int count = tagCount.value(tagName);
            tagCount.insert(tagName,count+1);
        }
        else {
            tagCount.insert(tagName,1);
        }
    }

    text.append("\n\nPrecision, Recall, and F-Measure per Tag Type:\n");

    QHashIterator<QString, int> it(refTagCount);
    while (it.hasNext()) {
        it.next();

        double pttprecision = (tagCount.value(it.key()) * 1.0) / it.value();
        double pttrecall = (tagCount.value(it.key()) * 1.0) / curTagCount.value(it.key());
        double fmeasure = 2 * (precision * recall) / (precision + recall);

        text.append(it.key() + ":\n");
        text.append("\tPrecision: " + QString::number(pttprecision) + '\n');
        text.append("\tRecall: " + QString::number(pttrecall) + '\n');
        text.append("\tF-Measure: " + QString::number(fmeasure) + '\n');
    }

    txtStats->setText(text);
}

void DiffView::rbBContainA_clicked() {

    txtCommon->clear();
    txtForwardDiff->clear();
    txtReverseDiff->clear();
    txtStats->clear();

    /** Analyze Tags based on B contain A Match **/

    commonVector.clear();
    forwardVector.clear();
    reverseVector.clear();

    // apply an algorithm similar to mergesort
    int otlength = tVector->count();
    int ctlength = cttVector->count();

    int c1=0;
    int c2=0;

    while((c1 < otlength) && (c2 < ctlength)) {
        Tag * ott = tVector->at(c1);
        Tag * ctt = cttVector->at(c2);
        if(((ctt->pos) <= (ott->pos)) && ((ctt->pos + ctt->length) >= (ott->pos + ott->length))) {
            if((ctt->tagtype->name) == (ott->tagtype->name)) {
                commonVector.append(ott);
                c1++;
                c2++;
            }
            else if((ott->tagtype->name) > (ctt->tagtype->name)) {
                reverseVector.append(ctt);
                c2++;
            }
            else {
                forwardVector.append(ott);
                c1++;
            }
        }
        else if((ctt->pos) < (ott->pos)) {
            reverseVector.append(ctt);
            c2++;
        }
        else {
            forwardVector.append(ott);
            c1++;
        }
    }
    if(c1 < (otlength-1)) {
        for(int i=c1; i< otlength; i++) {
            Tag * ctt = tVector->at(i);
            forwardVector.append(ctt);
        }
    }
    else if(c2 < (ctlength-1)) {
        for(int i=c2; i< ctlength; i++) {
            Tag * ctt = cttVector->at(i);
            reverseVector.append(ctt);
        }
    }

    /** Tag Words **/
    startTaggingText(_atagger->text);
    addTags(commonVector, forwardVector, reverseVector);
    finishTaggingText();

    double precision = (commonVector.count() * 1.0) / cttVector->count();
    double recall = (commonVector.count() * 1.0) / tVector->count();
    double fmeasure = 2 * (precision * recall) / (precision + recall);

    QString text;
    text.append("Precision, Recall, and F-Measure are done with ");
    QString first = _atagger->tagFile.split('/').last();
    first = first.remove(QRegExp(".tags.json"));
    text.append(first);
    text.append((" as reference:\n"));
    text.append("\nPrecision: ");
    text.append(QString::number(precision));
    text.append("\n\nRecall: ");
    text.append(QString::number(recall));
    text.append("\n\nF-Measure: ");
    text.append(QString::number(fmeasure));

    QHash<QString,int> curTagCount;
    for(int i=0; i< tVector->count(); i++) {
        QString tagName = tVector->at(i)->tagtype->name;
        if(curTagCount.contains(tagName)) {
            int count = curTagCount.value(tagName);
            curTagCount.insert(tagName,count+1);
        }
        else {
            curTagCount.insert(tagName,1);
        }
    }

    QHash<QString,int> refTagCount;
    for(int i=0; i< cttVector->count(); i++) {
        QString tagName = cttVector->at(i)->tagtype->name;
        if(refTagCount.contains(tagName)) {
            int count = refTagCount.value(tagName);
            refTagCount.insert(tagName,count+1);
        }
        else {
            refTagCount.insert(tagName,1);
        }
    }

    QHash<QString,int> tagCount;
    for(int i=0; i< commonVector.count(); i++) {
        QString tagName = commonVector.at(i)->tagtype->name;
        if(tagCount.contains(tagName)) {
            int count = tagCount.value(tagName);
            tagCount.insert(tagName,count+1);
        }
        else {
            tagCount.insert(tagName,1);
        }
    }

    text.append("\n\nPrecision, Recall, and F-Measure per Tag Type:\n");

    QHashIterator<QString, int> it(refTagCount);
    while (it.hasNext()) {
        it.next();

        double pttprecision = (tagCount.value(it.key()) * 1.0) / it.value();
        double pttrecall = (tagCount.value(it.key()) * 1.0) / curTagCount.value(it.key());
        double fmeasure = 2 * (precision * recall) / (precision + recall);

        text.append(it.key() + ":\n");
        text.append("\tPrecision: " + QString::number(pttprecision) + '\n');
        text.append("\tRecall: " + QString::number(pttrecall) + '\n');
        text.append("\tF-Measure: " + QString::number(fmeasure) + '\n');
    }

    txtStats->setText(text);
}

void DiffView::addTags(QVector<const Tag*> & commonVector, QVector<const Tag*> & forwardVector, QVector<const Tag*> & reverseVector) {

    /** Add common tags to common view **/
    for(int i =0; i< commonVector.count(); i++) {
        const Tag * pt = NULL;
        if(i>0) {
            pt = (Tag*)(commonVector.at(i-1));
        }
        const Tag * t = (Tag*)(commonVector.at(i));

        if(pt != NULL && pt->pos == t->pos) {
            continue;
        }
        const Tag * nt = NULL;
        if(i<(commonVector.count()-1)) {
            nt = (Tag*)(commonVector.at(i+1));
        }

        //for(int j=0; j< _atagger->tagTypeVector->count(); j++) {
            //const TagType * tt = (TagType*)(_atagger->tagTypeVector->at(j));

            //if(t->type == tt->name) {
        int start = t->pos;
        int length = t->length;
        QColor bgcolor(t->tagtype->bgcolor);
        QColor fgcolor(t->tagtype->fgcolor);
        int font = t->tagtype->font;
        //bool underline = (_atagger->tagTypeVector->at(j))->underline;
        bool underline = false;
        if(nt!=NULL && nt->pos == start) {
            underline = true;
        }
        bool bold = t->tagtype->bold;
        bool italic = t->tagtype->italic;
        tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold,common);
                //break;
            //}
        //}
    }

    /** Add forward tags to A-B view **/
    for(int i =0; i< forwardVector.count(); i++) {
        const Tag * pt = NULL;
        if(i>0) {
            pt = (Tag*)(forwardVector.at(i-1));
        }
        const Tag * t = (Tag*)(forwardVector.at(i));

        if(pt != NULL && pt->pos == t->pos) {
            continue;
        }
        const Tag * nt = NULL;
        if(i<(forwardVector.count()-1)) {
            nt = (Tag*)(forwardVector.at(i+1));
        }

        //for(int j=0; j< _atagger->tagTypeVector->count(); j++) {
            //const TagType * tt = (TagType*)(_atagger->tagTypeVector->at(j));

            //if(t->type == tt->tag) {
        int start = t->pos;
        int length = t->length;
        QColor bgcolor(t->tagtype->bgcolor);
        QColor fgcolor(t->tagtype->fgcolor);
        int font = t->tagtype->font;
        //bool underline = (_atagger->tagTypeVector->at(j))->underline;
        bool underline = false;
        if(nt!=NULL && nt->pos == start) {
            underline = true;
        }
        bool bold = t->tagtype->bold;
        bool italic = t->tagtype->italic;
        tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold,::forward);
                //break;
            //}
        //}
    }

    /** Add reverse tags to B-A view **/
    for(int i =0; i< reverseVector.count(); i++) {
        const Tag * pt = NULL;
        if(i>0) {
            pt = (Tag*)(reverseVector.at(i-1));
        }
        const Tag * t = (Tag*)(reverseVector.at(i));

        if(pt != NULL && pt->pos == t->pos) {
            continue;
        }
        const Tag * nt = NULL;
        if(i<(reverseVector.count()-1)) {
            nt = (Tag*)(reverseVector.at(i+1));
        }

        //for(int j=0; j< _atagger->tagTypeVector->count(); j++) {
            //const TagType * tt = (TagType*)(_atagger->tagTypeVector->at(j));

            //if(t->type == tt->tag) {
        int start = t->pos;
        int length = t->length;
        QColor bgcolor(t->tagtype->bgcolor);
        QColor fgcolor(t->tagtype->fgcolor);
        int font = t->tagtype->font;
        //bool underline = (_atagger->tagTypeVector->at(j))->underline;
        bool underline = false;
        if(nt!=NULL && nt->pos == start) {
            underline = true;
        }
        bool bold = t->tagtype->bold;
        bool italic = t->tagtype->italic;
        tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold,_reverse);
                //break;
            //}
        //}
    }
}

void DiffView::createActions() {
    /*
    untagCommonAct = new QAction(tr("Untag from Both"), this);
    untagCommonAct->setStatusTip(tr("Untag selected word"));
    connect(untagCommonAct, SIGNAL(triggered()), this, SLOT(untagCommon()));

    untagForwardAct = new QAction(tr("Untag from A"), this);
    untagForwardAct->setStatusTip(tr("Untag selected word"));
    connect(untagForwardAct, SIGNAL(triggered()), this, SLOT(untagForward()));

    untagReverseAct = new QAction(tr("Untag from B"), this);
    untagReverseAct->setStatusTip(tr("Untag selected word"));
    connect(untagReverseAct, SIGNAL(triggered()), this, SLOT(untagReverse()));
    */
}

void DiffView::showContextMenuCommon(const QPoint &pt) {
    int pos;
    //int length;
    signalMapper = new QSignalMapper(this);
    QMenu * menu = new QMenu();
    QMenu * mTags;
    mTags = menu->addMenu(tr("&Tag in Both"));

    signalMapperU = new QSignalMapper(this);
    QMenu * muTags;
    muTags = menu->addMenu(tr("&Untag in Both"));

    if(txtCommon->textCursor().selectedText().isEmpty()) {
        myTC = txtCommon->cursorForPosition(pt);
        myTC.select(QTextCursor::WordUnderCursor);
        QString word = myTC.selectedText();

        if(word.isEmpty()) {
            mTags->setEnabled(false);
            muTags->setEnabled(false);
            //untagCommonAct->setEnabled(false);
        }
        else {
            muTags->setEnabled(true);
        }
    }
    else {
        myTC = txtCommon->textCursor();
        muTags->setEnabled(true);
    }
    pos = myTC.selectionStart();
    //length = myTC.selectionEnd() - myTC.selectionStart();

    QStringList tagtypes;
    for(int i=0; i < commonVector.count(); i++) {
        const Tag * t = static_cast<const Tag *>(commonVector.at(i));
        if(t->pos == pos) {
            tagtypes << t->tagtype->name;
        }
    }

    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        if(!(tagtypes.contains(_atagger->tagTypeVector->at(i)->name))) {
            QAction * taginstance;
            taginstance = new QAction((_atagger->tagTypeVector->at(i))->name,this);
            signalMapper->setMapping(taginstance, (_atagger->tagTypeVector->at(i))->name);
            connect(taginstance, SIGNAL(triggered()), signalMapper, SLOT(map()));
            mTags->addAction(taginstance);
        }
    }
    connect(signalMapper, SIGNAL(mapped(const QString &)), this, SLOT(tagCommon(QString)));
    //menu->addAction(untagCommonAct);
    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        if(tagtypes.contains(_atagger->tagTypeVector->at(i)->name)) {
            QAction * taginstance;
            taginstance = new QAction((_atagger->tagTypeVector->at(i))->name,this);
            signalMapperU->setMapping(taginstance, (_atagger->tagTypeVector->at(i))->name);
            connect(taginstance, SIGNAL(triggered()), signalMapperU, SLOT(map()));
            muTags->addAction(taginstance);
        }
    }
    connect(signalMapperU, SIGNAL(mapped(const QString &)), this, SLOT(untagCommon(QString)));
    menu->exec(txtCommon->mapToGlobal(pt));
    delete menu;
}

int DiffView::insertTag(QString type, int pos, int length, int wordIndex, Source source, Dest dest) {

    const TagType* tagtype = NULL;
    for(int i=0;i<_atagger->tagTypeVector->count(); i++) {
        if(_atagger->tagTypeVector->at(i)->name == type) {
            tagtype = _atagger->tagTypeVector->at(i);
            break;
        }
    }
    Tag* tag = new Tag(tagtype,pos,length,wordIndex,source,_atagger->uniqueID);
    if(dest == original) {
        bool insert = true;
        for(int i=0; i<tVector->count(); i++) {
            if((tVector->at(i)->pos == pos) && (tVector->at(i)->tagtype->name == type)) {
                insert = false;
                break;
            }
        }
        if(insert) {
            tVector->append(tag);
            qSort(tVector->begin(), tVector->end(), compareTags);
            (_atagger->uniqueID)++;
            return 1;
        }
        else {
            return -1;
        }
    }
    else {
        bool insert = true;
        for(int i=0; i<cttVector->count(); i++) {
            if((cttVector->at(i)->pos == pos) && (cttVector->at(i)->tagtype->name == type)) {
                insert = false;
                break;
            }
        }
        if(insert) {
            cttVector->append(tag);
            qSort(cttVector->begin(), cttVector->end(), compareTags);
            (_atagger->uniqueID)++;
            return 1;
        }
        else {
            return -1;
        }
    }
    return -1;
}

void DiffView::tagCommon(QString tagValue) {
    if(!(myTC.selectedText().isEmpty())) {
        dirty = true;
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
        int insertA = insertTag(tagValue,start,length,wordIndex,user,original);
        int insertB = insertTag(tagValue,start,length,wordIndex,user,compareTo);

        if(insertA == -1 && insertB == -1) {
            return;
        }

        if(rbExact->isChecked()) {
            rbExact_clicked();
        }
        else if(rbIntersect->isChecked()) {
            rbIntersect_clicked();
        }
        else if(rbAContainB->isChecked()) {
            rbAContainB_clicked();
        }
        else {
            rbBContainA_clicked();
        }
        /*
        if(insertA == -1) {
            Tag tag(tagValue, start, length, user);
            for(int i=0; i < tVector->count(); i++) {
                if((tVector->at(i)) == tag) {
                    tagWord(start,length,QColor("black"),QColor("white"),12,false,false,false,forward);
                }
            }
        }
        int insertB = insertTag(tagValue, start, length, user, compareTo);
        if(insertB == -1) {
            Tag tag(tagValue, start, length, user);
            for(int i=0; i < cttVector->count(); i++) {
                if((cttVector->at(i)) == tag) {
                    tagWord(start,length,QColor("black"),QColor("white"),12,false,false,false,reverse);
                }
            }
        }

        for(int i=0; i< _atagger->tagTypeVector->count(); i++) {
            if((_atagger->tagTypeVector->at(i))->tag == tagValue) {
                QColor bgcolor((_atagger->tagTypeVector->at(i))->bgcolor);
                QColor fgcolor((_atagger->tagTypeVector->at(i))->fgcolor);
                int font = (_atagger->tagTypeVector->at(i))->font;
                //bool underline = (_atagger->tagTypeVector->at(i))->underline;
                bool underline = false;
                bool bold = (_atagger->tagTypeVector->at(i))->bold;
                bool italic = (_atagger->tagTypeVector->at(i))->italic;

                tagWord(cursor.selectionStart(),cursor.selectionEnd()-cursor.selectionStart(),fgcolor,bgcolor,font,underline,italic,bold,common);
            }
        }

        cursor.clearSelection();
        */
    }
    else {
        switch( QMessageBox::information( this, "Add Tag","No word is selected for tagging!","&Ok",0,0) ) {
            return;
        }
    }
}

void DiffView::untagCommon(QString tagValue) {
    QTextCursor cursor = myTC;
    int start = cursor.selectionStart();
    int length = cursor.selectionEnd() - cursor.selectionStart();
    if(length <= 0) {
        return;
    }
    dirty = true;
    for(int i=0; i < commonVector.count(); i++) {
        const Tag * t = (Tag*)(commonVector.at(i));
        QString t_type = t->tagtype->name;
        int t_pos = t->pos;
        //int t_length = t->length;
        Source t_source = t->source;

        if(t->pos == start && t->tagtype->name == tagValue) {

            for(int j=0; j< tVector->count(); j++) {
                if(t_pos == tVector->at(j)->pos && t_type == tVector->at(j)->tagtype->name && t_source == tVector->at(j)->source) {
                    delete (tVector->at(j));
                    tVector->remove(j);
                    break;
                }
            }

            for(int j=0; j< cttVector->count(); j++) {
                if(t_pos == cttVector->at(j)->pos && t_type == cttVector->at(j)->tagtype->name && t_source == cttVector->at(j)->source) {
                    delete (cttVector->at(j));
                    cttVector->remove(j);
                    break;
                }
            }
            cursor.clearSelection();
            break;
        }
    }

    if(rbExact->isChecked()) {
        rbExact_clicked();
    }
    else if(rbIntersect->isChecked()) {
        rbIntersect_clicked();
    }
    else if(rbAContainB->isChecked()) {
        rbAContainB_clicked();
    }
    else {
        rbBContainA_clicked();
    }
    /*
    for(int i=0; i < tVector->count(); i++) {
        if((tVector->at(i)).pos == start) {
            tagWord(start,length,QColor("black"),QColor("white"),12,false,false,false,common);
            tVector->remove(i);
            cursor.clearSelection();
        }
    }
    for(int i=0; i < cttVector->count(); i++) {
        if((cttVector->at(i)).pos == start) {
            cttVector->remove(i);
        }
    }
    */
}

void DiffView::showContextMenuForward(const QPoint &pt) {

    int pos;
    //int length;
    signalMapper = new QSignalMapper(this);
    QMenu * menu = new QMenu();
    QMenu * mTags;
    mTags = menu->addMenu(tr("&Tag in A"));

    signalMapperU = new QSignalMapper(this);
    QMenu * muTags;
    muTags = menu->addMenu(tr("&Untag in A"));

    if(txtForwardDiff->textCursor().selectedText().isEmpty()) {
        myTC = txtForwardDiff->cursorForPosition(pt);
        myTC.select(QTextCursor::WordUnderCursor);
        QString word = myTC.selectedText();

        if(word.isEmpty()) {
            mTags->setEnabled(false);
            muTags->setEnabled(false);
        }
        else {
            muTags->setEnabled(true);
        }
    }
    else {
        myTC = txtForwardDiff->textCursor();
        muTags->setEnabled(true);
    }
    pos = myTC.selectionStart();
    //length = myTC.selectionEnd() - myTC.selectionStart();

    QStringList tagtypes;
    for(int i=0; i < forwardVector.count(); i++) {
        const Tag * t = static_cast<const Tag *>(forwardVector.at(i));
        if(t->pos == pos) {
            tagtypes << t->tagtype->name;
        }
    }

    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        if(!(tagtypes.contains(_atagger->tagTypeVector->at(i)->name))) {
            QAction * taginstance;
            taginstance = new QAction((_atagger->tagTypeVector->at(i))->name,this);
            signalMapper->setMapping(taginstance, (_atagger->tagTypeVector->at(i))->name);
            connect(taginstance, SIGNAL(triggered()), signalMapper, SLOT(map()));
            mTags->addAction(taginstance);
        }
    }
    connect(signalMapper, SIGNAL(mapped(const QString &)), this, SLOT(tagForward(QString)));
    //menu->addAction(untagCommonAct);
    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        if(tagtypes.contains(_atagger->tagTypeVector->at(i)->name)) {
            QAction * taginstance;
            taginstance = new QAction((_atagger->tagTypeVector->at(i))->name,this);
            signalMapperU->setMapping(taginstance, (_atagger->tagTypeVector->at(i))->name);
            connect(taginstance, SIGNAL(triggered()), signalMapperU, SLOT(map()));
            muTags->addAction(taginstance);
        }
    }
    connect(signalMapperU, SIGNAL(mapped(const QString &)), this, SLOT(untagForward(QString)));
    menu->exec(txtForwardDiff->mapToGlobal(pt));
    delete menu;
}

void DiffView::tagForward(QString tagValue) {
    if(!(myTC.selectedText().isEmpty())) {

        dirty = true;
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
        insertTag(tagValue,start,length,wordIndex,user,original);

        if(rbExact->isChecked()) {
            rbExact_clicked();
        }
        else if(rbIntersect->isChecked()) {
            rbIntersect_clicked();
        }
        else if(rbAContainB->isChecked()) {
            rbAContainB_clicked();
        }
        else {
            rbBContainA_clicked();
        }
        /*
        for(int i=0; i< _atagger->tagTypeVector->count(); i++) {
            if((_atagger->tagTypeVector->at(i))->tag == tagValue) {
                QColor bgcolor((_atagger->tagTypeVector->at(i))->bgcolor);
                QColor fgcolor((_atagger->tagTypeVector->at(i))->fgcolor);
                int font = (_atagger->tagTypeVector->at(i))->font;
                //bool underline = (_atagger->tagTypeVector->at(i))->underline;
                bool underline = false;
                bool bold = (_atagger->tagTypeVector->at(i))->bold;
                bool italic = (_atagger->tagTypeVector->at(i))->italic;

                tagWord(cursor.selectionStart(),cursor.selectionEnd()-cursor.selectionStart(),fgcolor,bgcolor,font,underline,italic,bold,forward);
            }
        }
        cursor.clearSelection();
        */
    }
    else {
        switch( QMessageBox::information( this, "Add Tag","No word is selected for tagging!","&Ok",0,0) ) {
            return;
        }
    }
}

void DiffView::untagForward(QString tagValue) {
    QTextCursor cursor = myTC;
    int start = cursor.selectionStart();
    int length = cursor.selectionEnd() - cursor.selectionStart();
    if(length <= 0) {
        return;
    }
    dirty = true;

    for(int i=0; i < forwardVector.count(); i++) {
        const Tag * t = (Tag*)(forwardVector.at(i));

        if(t->pos == start && t->tagtype->name == tagValue) {
            for(int j=0; j< tVector->count(); j++) {
                if(t->pos == tVector->at(j)->pos && t->tagtype->name == tVector->at(j)->tagtype->name && t->source == tVector->at(j)->source) {
                    delete (tVector->at(j));
                    tVector->remove(j);
                    cursor.clearSelection();
                    break;
                }
            }
            break;
        }
    }

    if(rbExact->isChecked()) {
        rbExact_clicked();
    }
    else if(rbIntersect->isChecked()) {
        rbIntersect_clicked();
    }
    else if(rbAContainB->isChecked()) {
        rbAContainB_clicked();
    }
    else {
        rbBContainA_clicked();
    }
}

void DiffView::showContextMenuReverse(const QPoint &pt) {

    int pos;
    //int length;
    signalMapper = new QSignalMapper(this);
    QMenu * menu = new QMenu();
    QMenu * mTags;
    mTags = menu->addMenu(tr("&Tag in A"));

    signalMapperU = new QSignalMapper(this);
    QMenu * muTags;
    muTags = menu->addMenu(tr("&Untag in A"));

    if(txtReverseDiff->textCursor().selectedText().isEmpty()) {
        myTC = txtReverseDiff->cursorForPosition(pt);
        myTC.select(QTextCursor::WordUnderCursor);
        QString word = myTC.selectedText();

        if(word.isEmpty()) {
            mTags->setEnabled(false);
            muTags->setEnabled(false);
        }
        else {
            muTags->setEnabled(true);
        }
    }
    else {
        myTC = txtReverseDiff->textCursor();
        muTags->setEnabled(true);
    }
    pos = myTC.selectionStart();
    //length = myTC.selectionEnd() - myTC.selectionStart();

    QStringList tagtypes;
    for(int i=0; i < reverseVector.count(); i++) {
        const Tag * t = static_cast<const Tag *>(reverseVector.at(i));
        if(t->pos == pos) {
            tagtypes << t->tagtype->name;
        }
    }

    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        if(!(tagtypes.contains(_atagger->tagTypeVector->at(i)->name))) {
            QAction * taginstance;
            taginstance = new QAction((_atagger->tagTypeVector->at(i))->name,this);
            signalMapper->setMapping(taginstance, (_atagger->tagTypeVector->at(i))->name);
            connect(taginstance, SIGNAL(triggered()), signalMapper, SLOT(map()));
            mTags->addAction(taginstance);
        }
    }
    connect(signalMapper, SIGNAL(mapped(const QString &)), this, SLOT(tagReverse(QString)));
    //menu->addAction(untagCommonAct);
    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        if(tagtypes.contains(_atagger->tagTypeVector->at(i)->name)) {
            QAction * taginstance;
            taginstance = new QAction((_atagger->tagTypeVector->at(i))->name,this);
            signalMapperU->setMapping(taginstance, (_atagger->tagTypeVector->at(i))->name);
            connect(taginstance, SIGNAL(triggered()), signalMapperU, SLOT(map()));
            muTags->addAction(taginstance);
        }
    }
    connect(signalMapperU, SIGNAL(mapped(const QString &)), this, SLOT(untagReverse(QString)));
    menu->exec(txtReverseDiff->mapToGlobal(pt));
    delete menu;
}

void DiffView::tagReverse(QString tagValue) {
    if(!(myTC.selectedText().isEmpty())) {

        dirty = true;
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
        insertTag(tagValue,start,length,wordIndex,user,compareTo);

        if(rbExact->isChecked()) {
            rbExact_clicked();
        }
        else if(rbIntersect->isChecked()) {
            rbIntersect_clicked();
        }
        else if(rbAContainB->isChecked()) {
            rbAContainB_clicked();
        }
        else {
            rbBContainA_clicked();
        }
        /*
        for(int i=0; i< _atagger->compareToTagTypeVector->count(); i++) {
            if((_atagger->compareToTagTypeVector->at(i))->tag == tagValue) {
                QColor bgcolor((_atagger->compareToTagTypeVector->at(i))->bgcolor);
                QColor fgcolor((_atagger->compareToTagTypeVector->at(i))->fgcolor);
                int font = (_atagger->compareToTagTypeVector->at(i))->font;
                //bool underline = (_atagger->compareToTagTypeVector->at(i))->underline;
                bool underline = false;
                bool bold = (_atagger->compareToTagTypeVector->at(i))->bold;
                bool italic = (_atagger->compareToTagTypeVector->at(i))->italic;

                tagWord(cursor.selectionStart(),cursor.selectionEnd()-cursor.selectionStart(),fgcolor,bgcolor,font,underline,italic,bold,reverse);
            }
        }
        cursor.clearSelection();
        */
    }
    else {
        switch( QMessageBox::information( this, "Add Tag","No word is selected for tagging!","&Ok",0,0) ) {
            return;
        }
    }
}

void DiffView::untagReverse(QString tagValue) {
    QTextCursor cursor = myTC;
    int start = cursor.selectionStart();
    int length = cursor.selectionEnd() - cursor.selectionStart();
    if(length <= 0) {
        return;
    }
    dirty = true;

    for(int i=0; i < reverseVector.count(); i++) {
        const Tag * t = (Tag*)(reverseVector.at(i));

        if(t->pos == start && t->tagtype->name == tagValue) {
            for(int j=0; j< cttVector->count(); j++) {
                if(t->pos == cttVector->at(j)->pos && t->tagtype->name == cttVector->at(j)->tagtype->name && t->source == cttVector->at(j)->source) {
                    delete (cttVector->at(j));
                    cttVector->remove(j);
                    cursor.clearSelection();
                    break;
                }
            }
            break;
        }
    }

    if(rbExact->isChecked()) {
        rbExact_clicked();
    }
    else if(rbIntersect->isChecked()) {
        rbIntersect_clicked();
    }
    else if(rbAContainB->isChecked()) {
        rbAContainB_clicked();
    }
    else {
        rbBContainA_clicked();
    }
}

void DiffView::closeEvent(QCloseEvent *event) {

    if(dirty) {
        QMessageBox msgBox;
         msgBox.setText("The document has been modified.");
         msgBox.setInformativeText("Do you want to save your changes?");
         msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
         msgBox.setDefaultButton(QMessageBox::Save);
         int ret = msgBox.exec();

         switch (ret) {
         case QMessageBox::Save:
             _atagger->tagHash->clear();
             for(int i=0; i<tVector->count(); i++) {
                 _atagger->tagHash->insert(tVector->at(i)->wordIndex,tVector->at(i));
             }
             _atagger->compareToTagHash.clear();
             for(int i=0; i<cttVector->count(); i++) {
                 _atagger->compareToTagHash.insert(cttVector->at(i)->wordIndex,cttVector->at(i));
             }
             break;
         case QMessageBox::Discard:
             break;
         default:
             break;
         }
     }
}

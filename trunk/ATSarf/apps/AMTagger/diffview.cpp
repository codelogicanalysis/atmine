#include "diffview.h"
#include "global.h"
#include "sarftagtype.h"
#include <QGridLayout>
#include <QDockWidget>
#include <QScrollArea>
#include <QSplitter>
#include <QtAlgorithms>
#include <QStringBuilder>

bool compareTags(const Tag &tag1, const Tag &tag2) {
    if(tag1.pos != tag2.pos) {
        return tag1.pos < tag2.pos;
    }
    else {
        return tag1.type < tag2.type;
    }
}

bool compareTagTypes(const TagType *tt1, const TagType *tt2) {
    return (tt1->tag < tt2->tag);
}

DiffView::DiffView(QWidget *parent) :
    QMainWindow(parent)
{
    QDockWidget *dock = new QDockWidget(tr("Difference"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea);
    lblCommon = new QLabel(tr("Common"));
    txtCommon = new QTextBrowser();
    QVBoxLayout *vb1 = new QVBoxLayout;
    vb1->setAlignment(Qt::AlignCenter);
    vb1->addWidget(lblCommon);
    vb1->addWidget(txtCommon);
    QScrollArea *sac = new QScrollArea;
    sac->setLayout(vb1);
    lblForwardDiff = new QLabel(tr("A-B"));
    txtForwardDiff = new QTextBrowser();
    QVBoxLayout *vb2 = new QVBoxLayout;
    vb2->setAlignment(Qt::AlignCenter);
    vb2->addWidget(lblForwardDiff);
    vb2->addWidget(txtForwardDiff);
    QScrollArea *saf = new QScrollArea;
    saf->setLayout(vb2);
    lblReverseDiff = new QLabel(tr("B-A"));
    txtReverseDiff = new QTextBrowser();
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

    setWindowTitle(tr("Difference Analysis Statistics"));
    resize(800,600);

    /** Sort the Tags in both Tag Vectors **/
    qSort(_atagger->tagVector.begin(), _atagger->tagVector.end(), compareTags);
    qSort(_atagger->compareToTagVector.begin(), _atagger->compareToTagVector.end(), compareTags);

    /** Analyze tagtype difference **/

    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        const TagType * stt = (TagType*)(_atagger->tagTypeVector->at(i));
        bool found = false;
        for(int j=0; j<_atagger->compareToTagTypeVector->count();j++) {
            const TagType * ctt = (TagType*)(_atagger->compareToTagTypeVector->at(j));
            if(stt->tag == ctt->tag) {
                found = true;
                commonTT.append(stt->tag);
            }
        }
        if(!found) {
            forwardTT.append(stt->tag);
        }
    }

    for(int i=0; i<_atagger->compareToTagTypeVector->count();i++) {
        const TagType * ctt = (TagType*)(_atagger->tagTypeVector->at(i));
        if(!(commonTT.contains(ctt->tag))){
            reverseTT.append(ctt->tag);
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
    if (this==NULL)
        return;
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

void DiffView::tagWord(int start, int length, QColor fcolor, QColor  bcolor,int font, bool underline, bool italic, bool bold, DestText dt){
    if (this==NULL)
        return;
    QTextBrowser * taggedBox;
    if(dt == common) {
        taggedBox = txtCommon;
    } else if(dt == forward) {
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
    if (this==NULL)
        return;
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

    /** Analyze tagtype difference **/

    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        const TagType * stt = (TagType*)(_atagger->tagTypeVector->at(i));
        bool found = false;
        for(int j=0; j<_atagger->compareToTagTypeVector->count();j++) {
            const TagType * ctt = (TagType*)(_atagger->compareToTagTypeVector->at(j));
            if(stt->tag == ctt->tag) {
                found = true;
                commonTT.append(stt->tag);
            }
        }
        if(!found) {
            forwardTT.append(stt->tag);
        }
    }

    for(int i=0; i<_atagger->compareToTagTypeVector->count();i++) {
        const TagType * ctt = (TagType*)(_atagger->tagTypeVector->at(i));
        if(!(commonTT.contains(ctt->tag))){
            reverseTT.append(ctt->tag);
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

}

void DiffView::rbExact_clicked() {
    txtCommon->clear();
    txtForwardDiff->clear();
    txtReverseDiff->clear();
    txtStats->clear();

    /** Analyze Tags based on Exact Match **/

    QVector<const Tag*> commonVector;
    QVector<const Tag*> forwardVector;
    QVector<const Tag*> reverseVector;

    // apply an algorithm similar to mergesort
    int otlength = _atagger->tagVector.count();
    int ctlength = _atagger->compareToTagVector.count();
    //int count = otlength + ctlength;

    int c1=0;
    int c2=0;

    while((c1 < otlength) && (c2 < ctlength)) {
        const Tag * ott = (Tag*)(&(_atagger->tagVector.at(c1)));
        const Tag * ctt = (Tag*)(&(_atagger->compareToTagVector.at(c2)));
        if(((ott->pos) == (ctt->pos)) && ((ott->length) == (ctt->length))) {
            if((ott->type) == (ctt->type))  {
                commonVector.append(ott);
                c1++;
                c2++;
            }
            else if((ott->type) > (ctt->type)){
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
            const Tag * ott = (Tag*)(&(_atagger->tagVector.at(i)));
            forwardVector.append(ott);
        }
    }
    else if(c2 < (ctlength-1)) {
        for(int i=c2; i< ctlength; i++) {
            const Tag * ctt = (Tag*)(&(_atagger->compareToTagVector.at(i)));
            reverseVector.append(ctt);
        }
    }

    /** Tag Words **/
    startTaggingText(_atagger->text);

    for(int i=0; i<commonVector.count(); i++) {
        for(int j=0; j<_atagger->tagTypeVector->count(); j++) {
            const TagType * stt = (TagType*)(_atagger->tagTypeVector->at(j));
            if(commonVector.at(i)->type == stt->tag) {
                int start = commonVector.at(i)->pos;
                int length = commonVector.at(i)->length;
                QColor bgcolor(stt->bgcolor);
                QColor fgcolor(stt->fgcolor);
                int font = stt->font;
                bool underline = stt->underline;
                bool bold = stt->bold;
                bool italic = stt->italic;
                tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold,common);
                break;
            }
        }
    }

    for(int i=0; i<forwardVector.count(); i++) {
        for(int j=0; j<_atagger->tagTypeVector->count(); j++) {
            const TagType * stt = (TagType*)(_atagger->tagTypeVector->at(j));
            if(forwardVector.at(i)->type == stt->tag) {
                int start = forwardVector.at(i)->pos;
                int length = forwardVector.at(i)->length;
                QColor bgcolor(stt->bgcolor);
                QColor fgcolor(stt->fgcolor);
                int font = stt->font;
                bool underline = stt->underline;
                bool bold = stt->bold;
                bool italic = stt->italic;
                tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold,forward);
                break;
            }
        }
    }

    for(int i=0; i<reverseVector.count(); i++) {
        for(int j=0; j<_atagger->tagTypeVector->count(); j++) {
            const TagType * stt = (TagType*)(_atagger->compareToTagTypeVector->at(j));
            if(reverseVector.at(i)->type == stt->tag) {
                int start = reverseVector.at(i)->pos;
                int length = reverseVector.at(i)->length;
                QColor bgcolor(stt->bgcolor);
                QColor fgcolor(stt->fgcolor);
                int font = stt->font;
                bool underline = stt->underline;
                bool bold = stt->bold;
                bool italic = stt->italic;
                tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold,reverse);
                break;
            }
        }
    }

    finishTaggingText();

    double precision = (commonVector.count() * 1.0) / _atagger->compareToTagVector.count();
    double recall = (commonVector.count() * 1.0) / _atagger->tagVector.count();
    double fmeasure = 2 * (precision * recall) / (precision + recall);

    QString text;
    text.append("Precision as A is reference: ");
    text.append(QString::number(precision));
    text.append("\n\nRecall as A is reference: ");
    text.append(QString::number(recall));
    text.append("\n\nF-Measure as A is reference: ");
    text.append(QString::number(fmeasure));

    txtStats->setText(text);
}

void DiffView::rbIntersect_clicked() {

    txtCommon->clear();
    txtForwardDiff->clear();
    txtReverseDiff->clear();
    txtStats->clear();

    /** Analyze Tags based on A intersect B Match **/

    QVector<const Tag*> commonVector;
    QVector<const Tag*> forwardVector;
    QVector<const Tag*> reverseVector;

    int otlength = _atagger->tagVector.count();
    int ctlength = _atagger->compareToTagVector.count();

    int c1=0;
    int c2=0;

    while((c1 < otlength) && (c2 < ctlength)) {
        const Tag * ott = (Tag*)(&(_atagger->tagVector.at(c1)));
        const Tag * ctt = (Tag*)(&(_atagger->compareToTagVector.at(c2)));
        if(((ott->pos <= ctt->pos) && ((ott->pos + ott->length) > (ctt->pos))) ||
           ((ctt->pos <= ott->pos) && ((ctt->pos + ctt->length) > (ott->pos)))) {
            if((ott->type) == (ctt->type)) {
                commonVector.append(ott);
                c1++;
                c2++;
            }
            else if((ott->type) > (ctt->type)) {
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
            const Tag * ott = (Tag*)(&(_atagger->tagVector.at(i)));
            forwardVector.append(ott);
        }
    }
    else if(c2 < (ctlength-1)) {
        for(int i=c2; i< ctlength; i++) {
            const Tag * ctt = (Tag*)(&(_atagger->compareToTagVector.at(i)));
            reverseVector.append(ctt);
        }
    }

    /** Tag Words **/
    startTaggingText(_atagger->text);

    for(int i=0; i<commonVector.count(); i++) {
        for(int j=0; j<_atagger->tagTypeVector->count(); j++) {
            const TagType * stt = (TagType*)(_atagger->tagTypeVector->at(j));
            if(commonVector.at(i)->type == stt->tag) {
                int start = commonVector.at(i)->pos;
                int length = commonVector.at(i)->length;
                QColor bgcolor(stt->bgcolor);
                QColor fgcolor(stt->fgcolor);
                int font = stt->font;
                bool underline = stt->underline;
                bool bold = stt->bold;
                bool italic = stt->italic;
                tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold,common);
                break;
            }
        }
    }

    for(int i=0; i<forwardVector.count(); i++) {
        for(int j=0; j<_atagger->tagTypeVector->count(); j++) {
            const TagType * stt = (TagType*)(_atagger->tagTypeVector->at(j));
            if(forwardVector.at(i)->type == stt->tag) {
                int start = forwardVector.at(i)->pos;
                int length = forwardVector.at(i)->length;
                QColor bgcolor(stt->bgcolor);
                QColor fgcolor(stt->fgcolor);
                int font = stt->font;
                bool underline = stt->underline;
                bool bold = stt->bold;
                bool italic = stt->italic;
                tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold,forward);
                break;
            }
        }
    }

    for(int i=0; i<reverseVector.count(); i++) {
        for(int j=0; j<_atagger->tagTypeVector->count(); j++) {
            const TagType * stt = (TagType*)(_atagger->compareToTagTypeVector->at(j));
            if(reverseVector.at(i)->type == stt->tag) {
                int start = reverseVector.at(i)->pos;
                int length = reverseVector.at(i)->length;
                QColor bgcolor(stt->bgcolor);
                QColor fgcolor(stt->fgcolor);
                int font = stt->font;
                bool underline = stt->underline;
                bool bold = stt->bold;
                bool italic = stt->italic;
                tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold,reverse);
                break;
            }
        }
    }

    finishTaggingText();

    double precision = (commonVector.count() * 1.0) / _atagger->compareToTagVector.count();
    double recall = (commonVector.count() * 1.0) / _atagger->tagVector.count();
    double fmeasure = 2 * (precision * recall) / (precision + recall);

    QString text;
    text.append("Precision as A is reference: ");
    text.append(QString::number(precision));
    text.append("\n\nRecall as A is reference: ");
    text.append(QString::number(recall));
    text.append("\n\nF-Measure as A is reference: ");
    text.append(QString::number(fmeasure));

    txtStats->setText(text);
}

void DiffView::rbAContainB_clicked() {
    txtCommon->clear();
    txtForwardDiff->clear();
    txtReverseDiff->clear();
    txtStats->clear();

    /** Analyze Tags based on A contain B Match **/

    QVector<const Tag*> commonVector;
    QVector<const Tag*> forwardVector;
    QVector<const Tag*> reverseVector;

    // apply an algorithm similar to mergesort
    int otlength = _atagger->tagVector.count();
    int ctlength = _atagger->compareToTagVector.count();
    //int count = otlength + ctlength;

    int c1=0;
    int c2=0;

    while((c1 < otlength) && (c2 < ctlength)) {
        const Tag * ott = (Tag*)(&(_atagger->tagVector.at(c1)));
        const Tag * ctt = (Tag*)(&(_atagger->compareToTagVector.at(c2)));
        if(((ott->pos) <= (ctt->pos)) && ((ott->pos + ott->length) >= (ctt->pos + ctt->length))) {
            if((ott->type) == (ctt->type)) {
                commonVector.append(ott);
                c1++;
                c2++;
            }
            else if((ott->type) > (ctt->type)) {
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
            const Tag * ott = (Tag*)(&(_atagger->tagVector.at(i)));
            forwardVector.append(ott);
        }
    }
    else if(c2 < (ctlength-1)) {
        for(int i=c2; i< ctlength; i++) {
            const Tag * ctt = (Tag*)(&(_atagger->compareToTagVector.at(i)));
            reverseVector.append(ctt);
        }
    }

    /** Tag Words **/
    startTaggingText(_atagger->text);

    for(int i=0; i<commonVector.count(); i++) {
        for(int j=0; j<_atagger->tagTypeVector->count(); j++) {
            const TagType * stt = (TagType*)(_atagger->tagTypeVector->at(j));
            if(commonVector.at(i)->type == stt->tag) {
                int start = commonVector.at(i)->pos;
                int length = commonVector.at(i)->length;
                QColor bgcolor(stt->bgcolor);
                QColor fgcolor(stt->fgcolor);
                int font = stt->font;
                bool underline = stt->underline;
                bool bold = stt->bold;
                bool italic = stt->italic;
                tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold,common);
                break;
            }
        }
    }

    for(int i=0; i<forwardVector.count(); i++) {
        for(int j=0; j<_atagger->tagTypeVector->count(); j++) {
            const TagType * stt = (TagType*)(_atagger->tagTypeVector->at(j));
            if(forwardVector.at(i)->type == stt->tag) {
                int start = forwardVector.at(i)->pos;
                int length = forwardVector.at(i)->length;
                QColor bgcolor(stt->bgcolor);
                QColor fgcolor(stt->fgcolor);
                int font = stt->font;
                bool underline = stt->underline;
                bool bold = stt->bold;
                bool italic = stt->italic;
                tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold,forward);
                break;
            }
        }
    }

    for(int i=0; i<reverseVector.count(); i++) {
        for(int j=0; j<_atagger->tagTypeVector->count(); j++) {
            const TagType * stt = (TagType*)(_atagger->compareToTagTypeVector->at(j));
            if(reverseVector.at(i)->type == stt->tag) {
                int start = reverseVector.at(i)->pos;
                int length = reverseVector.at(i)->length;
                QColor bgcolor(stt->bgcolor);
                QColor fgcolor(stt->fgcolor);
                int font = stt->font;
                bool underline = stt->underline;
                bool bold = stt->bold;
                bool italic = stt->italic;
                tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold,reverse);
                break;
            }
        }
    }

    finishTaggingText();

    double precision = (commonVector.count() * 1.0) / _atagger->compareToTagVector.count();
    double recall = (commonVector.count() * 1.0) / _atagger->tagVector.count();
    double fmeasure = 2 * (precision * recall) / (precision + recall);

    QString text;
    text.append("Precision as A is reference: ");
    text.append(QString::number(precision));
    text.append("\n\nRecall as A is reference: ");
    text.append(QString::number(recall));
    text.append("\n\nF-Measure as A is reference: ");
    text.append(QString::number(fmeasure));

    txtStats->setText(text);
}

void DiffView::rbBContainA_clicked() {

    txtCommon->clear();
    txtForwardDiff->clear();
    txtReverseDiff->clear();
    txtStats->clear();

    /** Analyze Tags based on B contain A Match **/

    QVector<const Tag*> commonVector;
    QVector<const Tag*> forwardVector;
    QVector<const Tag*> reverseVector;

    // apply an algorithm similar to mergesort
    int otlength = _atagger->tagVector.count();
    int ctlength = _atagger->compareToTagVector.count();

    int c1=0;
    int c2=0;

    while((c1 < otlength) && (c2 < ctlength)) {
        const Tag * ott = (Tag*)(&(_atagger->tagVector.at(c1)));
        const Tag * ctt = (Tag*)(&(_atagger->compareToTagVector.at(c2)));
        if(((ctt->pos) <= (ott->pos)) && ((ctt->pos + ctt->length) >= (ott->pos + ott->length))) {
            if((ctt->type) == (ott->type)) {
                commonVector.append(ott);
                c1++;
                c2++;
            }
            else if((ott->type) > (ctt->type)) {
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
            const Tag * ctt = (Tag*)(&(_atagger->tagVector.at(i)));
            forwardVector.append(ctt);
        }
    }
    else if(c2 < (ctlength-1)) {
        for(int i=c2; i< ctlength; i++) {
            const Tag * ctt = (Tag*)(&(_atagger->compareToTagVector.at(i)));
            reverseVector.append(ctt);
        }
    }

    /** Tag Words **/
    startTaggingText(_atagger->text);

    for(int i=0; i<commonVector.count(); i++) {
        for(int j=0; j<_atagger->tagTypeVector->count(); j++) {
            const TagType * stt = (TagType*)(_atagger->tagTypeVector->at(j));
            if(commonVector.at(i)->type == stt->tag) {
                int start = commonVector.at(i)->pos;
                int length = commonVector.at(i)->length;
                QColor bgcolor(stt->bgcolor);
                QColor fgcolor(stt->fgcolor);
                int font = stt->font;
                bool underline = stt->underline;
                bool bold = stt->bold;
                bool italic = stt->italic;
                tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold,common);
                break;
            }
        }
    }

    for(int i=0; i<forwardVector.count(); i++) {
        for(int j=0; j<_atagger->tagTypeVector->count(); j++) {
            const TagType * stt = (TagType*)(_atagger->tagTypeVector->at(j));
            if(forwardVector.at(i)->type == stt->tag) {
                int start = forwardVector.at(i)->pos;
                int length = forwardVector.at(i)->length;
                QColor bgcolor(stt->bgcolor);
                QColor fgcolor(stt->fgcolor);
                int font = stt->font;
                bool underline = stt->underline;
                bool bold = stt->bold;
                bool italic = stt->italic;
                tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold,forward);
                break;
            }
        }
    }

    for(int i=0; i<reverseVector.count(); i++) {
        for(int j=0; j<_atagger->tagTypeVector->count(); j++) {
            const TagType * stt = (TagType*)(_atagger->compareToTagTypeVector->at(j));
            if(reverseVector.at(i)->type == stt->tag) {
                int start = reverseVector.at(i)->pos;
                int length = reverseVector.at(i)->length;
                QColor bgcolor(stt->bgcolor);
                QColor fgcolor(stt->fgcolor);
                int font = stt->font;
                bool underline = stt->underline;
                bool bold = stt->bold;
                bool italic = stt->italic;
                tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold,reverse);
                break;
            }
        }
    }

    finishTaggingText();

    double precision = (commonVector.count() * 1.0) / _atagger->compareToTagVector.count();
    double recall = (commonVector.count() * 1.0) / _atagger->tagVector.count();
    double fmeasure = 2 * (precision * recall) / (precision + recall);

    QString text;
    text.append("Precision as A is reference: ");
    text.append(QString::number(precision));
    text.append("\n\nRecall as A is reference: ");
    text.append(QString::number(recall));
    text.append("\n\nF-Measure as A is reference: ");
    text.append(QString::number(fmeasure));

    txtStats->setText(text);
}

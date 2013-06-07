#include "customizemsfview.h"

CustomizeMSFView::CustomizeMSFView(QWidget *parent) :
    QMainWindow(parent)
{
    QGridLayout *grid = new QGridLayout();
    btnSelect = new QPushButton(tr(">>"), this);
    btnUnselect = new QPushButton(tr("<<"), this);
    btnAdd = new QPushButton(tr("Add\nMSF"), this);
    btnRemove = new QPushButton(tr("Remove\nMSF"), this);

    btnSelect->setEnabled(false);
    btnUnselect->setEnabled(false);
    btnRemove->setEnabled(false);

    connect(btnSelect, SIGNAL(clicked()), this, SLOT(btnSelect_clicked()));
    connect(btnUnselect, SIGNAL(clicked()), this, SLOT(btnUnselect_clicked()));
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(btnAdd_clicked()));
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(btnRemove_clicked()));

    grid->addWidget(btnSelect,6,2);
    grid->addWidget(btnUnselect,7,2);
    grid->addWidget(btnAdd,3,2);
    grid->addWidget(btnRemove,4,2);

    lblMBF = new QLabel(tr("Possible MBFs"), this);
    lblMSF = new QLabel(tr("MSF:"), this);
    lblActions = new QLabel(tr("Actions"), this);

    grid->addWidget(lblMBF,0,0);
    grid->addWidget(lblMSF,0,3);
    grid->addWidget(lblActions,0,5,1,2,Qt::AlignCenter);

    editActions = new QTextEdit(this);
    editLimit = new QLineEdit(this);
    editLimit->setMaximumWidth(90);
    editLimit->setEnabled(false);

    grid->addWidget(editActions,1,5,12,2);
    grid->addWidget(editLimit,9,2);

    cbMSF = new QComboBox(this);
    cbOperations = new QComboBox(this);

    cbOperations->addItem("?");
    cbOperations->addItem("*");
    cbOperations->addItem("+");
    cbOperations->addItem("^");

    connect(cbOperations, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbOperation_changed(QString)));
    connect(cbMSF, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbMSF_changed(QString)));

    grid->addWidget(cbMSF,0,4);
    grid->addWidget(cbOperations,8,2);

    listMBF = new QListWidget(this);
    listMBF->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(listMBF, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listMBF_itemclicked(QListWidgetItem*)));

    grid->addWidget(listMBF,1,0,6,2);

    treeMSF = new QTreeWidget(this);
    treeMSF->setColumnCount(3);
    QStringList columnsD;
    columnsD << "Name" << "MBF" << "Operation";
    QTreeWidgetItem* itemD=new QTreeWidgetItem(columnsD);
    treeMSF->setHeaderItem(itemD);
    treeMSF->setSelectionMode(QAbstractItemView::SingleSelection);

    grid->addWidget(treeMSF,1,3,12,2);

    treeMBFdesc = new QTreeWidget(this);
    treeMBFdesc->setColumnCount(4);
    QStringList columnsdesc;
    columnsdesc << QString() << "Feature" << "Relation" << "Value";
    QTreeWidgetItem* itemdesc=new QTreeWidgetItem(columnsdesc);
    treeMBFdesc->setHeaderItem(itemdesc);
    treeMBFdesc->setSelectionMode(QAbstractItemView::SingleSelection);

    grid->addWidget(treeMBFdesc,7,0,6,2);

    QWidget *widget = new QWidget(this);
    widget->setLayout(grid);
    setCentralWidget(widget);
    setWindowTitle(tr("Custom Morphology-based Sequential Formula"));

    /** Initialize Elements **/

    theSarf->query.exec("SELECT id,name FROM category where abstract=1");
    while(theSarf->query.next()) {
        if(!(theSarf->query.value(0).toString().isEmpty())) {
            listCategoryId << theSarf->query.value(0).toString();
            listCategory << theSarf->query.value(1).toString();
        }
    }

    QStringList tagtypes;
    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        tagtypes.append(_atagger->tagTypeVector->at(i)->tag);
    }
    tagtypes.append("NONE");
    listMBF->addItems(tagtypes);
    //listMBF->setAlternatingRowColors(true);
}

void CustomizeMSFView::btnAdd_clicked() {

    bool ok;
    QString msfName = QInputDialog::getText(this,"Tag Name","Please insert a formula Name", QLineEdit::Normal,
                                            QDir::home().dirName(), &ok);
    if(!ok || msfName.isEmpty()) {
        return;
    }

    for(int i=0; i<_atagger->msfVector->count(); i++) {
        const MSFormula* msf = _atagger->msfVector->at(i);
        if(msf->name == msfName) {
            QMessageBox::warning(this, "Warning", "This formula Name already exists!");
            return;
        }
    }

    MSFormula* formula = new MSFormula(msfName);
    _atagger->msfVector->append(formula);

    disconnect_Signals();

    btnRemove->setEnabled(true);

    if(!btnSelect->isEnabled()) {
        btnSelect->setEnabled(true);
        btnUnselect->setEnabled(true);
        btnRemove->setEnabled(true);
    }
    treeMSF->clear();
    editActions->clear();

    cbMSF->addItem(msfName);
    cbMSF->setCurrentIndex(cbMSF->findText(msfName));

    connect_Signals();
}

void CustomizeMSFView::btnRemove_clicked() {
    if(_atagger->msfVector->count() == 0) {
        return;
    }
    QString msfRemoved = cbMSF->currentText();
    for(int i=0; i<_atagger->msfVector->count(); i++) {
        const MSFormula * msf = _atagger->msfVector->at(i);
        if(msf->name == msfRemoved) {
            _atagger->msfVector->remove(i);
            break;
        }
    }
    cbMSF->removeItem(cbMSF->currentIndex());
    if(_atagger->msfVector->count() == 0) {
        btnSelect->setEnabled(false);
        btnUnselect->setEnabled(false);
        btnRemove->setEnabled(false);
    }
}

void CustomizeMSFView::btnSelect_clicked() {

}

void CustomizeMSFView::btnUnselect_clicked() {

}

void CustomizeMSFView::cbMSF_changed(QString name) {
    treeMSF->clear();
    if(name.isEmpty() || name.isNull()) {
        return;
    }

    disconnect_Signals();
    for(int i=0; i<_atagger->msfVector->count(); i++) {
        MSFormula * msf = _atagger->msfVector->at(i);
        if(msf->name == name) {
            btnSelect->setEnabled(true);
            btnUnselect->setEnabled(true);
        }
    }
    connect_Signals();
}

void CustomizeMSFView::cbOperation_changed(QString op) {
    editLimit->clear();
    if(op == "^") {
        editLimit->setEnabled(true);
    }
    else {
        editLimit->setEnabled(false);
    }
}

void CustomizeMSFView::listMBF_itemclicked(QListWidgetItem *item) {
    treeMBFdesc->clear();
    QString tagtype = item->text();
    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        if(_atagger->tagTypeVector->at(i)->tag == tagtype && _atagger->tagTypeVector->at(i)->source == sarf) {
            SarfTagType* stt = (SarfTagType*)(_atagger->tagTypeVector->at(i));
            for(int j=0; j<stt->tags.count(); j++) {
                QStringList list;
                if(stt->tags.at(j).first != "Category") {
                    QString four = stt->tags[j].fourth;
                    if(four.contains("Syn")) {
                        four.append(" of");
                    }
                    list << stt->tags.at(j).third <<stt->tags.at(j).first << four <<stt->tags.at(j).second;
                }
                else {
                    for(int k=0; k< listCategoryId.count();k++) {
                        if(listCategoryId[k] == stt->tags.at(j).second) {
                            list << stt->tags.at(j).third << stt->tags.at(j).first << stt->tags.at(j).fourth << listCategory[k];
                        }
                    }
                }
                treeMBFdesc->addTopLevelItem(new QTreeWidgetItem(list));
            }
            break;
        }
    }
}

void CustomizeMSFView::disconnect_Signals() {
    disconnect(btnSelect, SIGNAL(clicked()), this, SLOT(btnSelect_clicked()));
    disconnect(btnUnselect, SIGNAL(clicked()), this, SLOT(btnUnselect_clicked()));
    disconnect(btnAdd, SIGNAL(clicked()), this, SLOT(btnAdd_clicked()));
    disconnect(btnRemove, SIGNAL(clicked()), this, SLOT(btnRemove_clicked()));
    disconnect(cbMSF, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbMSF_changed(QString)));
    disconnect(cbOperations,SIGNAL(currentIndexChanged(QString)), this, SLOT(cbOperation_changed(QString)));
    disconnect(listMBF, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listMBF_itemclicked(QListWidgetItem*)));
}

void CustomizeMSFView::connect_Signals() {
    connect(btnSelect, SIGNAL(clicked()), this, SLOT(btnSelect_clicked()));
    connect(btnUnselect, SIGNAL(clicked()), this, SLOT(btnUnselect_clicked()));
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(btnAdd_clicked()));
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(btnRemove_clicked()));
    connect(cbMSF, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbMSF_changed(QString)));
    connect(cbOperations,SIGNAL(currentIndexChanged(QString)), this, SLOT(cbOperation_changed(QString)));
    connect(listMBF, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listMBF_itemclicked(QListWidgetItem*)));
}

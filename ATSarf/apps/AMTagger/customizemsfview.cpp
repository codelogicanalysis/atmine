#include "customizemsfview.h"

CustomizeMSFView::CustomizeMSFView(QWidget *parent) :
    QMainWindow(parent)
{

    isDirty = false;
    currentF = NULL;
    QGridLayout *grid = new QGridLayout();
    btnSelect = new QPushButton(tr(">>"), this);
    btnUnselect = new QPushButton(tr("<<"), this);
    btnAdd = new QPushButton(tr("Add\nMSF"), this);
    btnRemove = new QPushButton(tr("Remove\nMSF"), this);

    btnStar = new QPushButton(tr("*"), this);
    btnPlus = new QPushButton(tr("+"), this);
    btnQuestion = new QPushButton(tr("?"), this);
    btnLimit = new QPushButton(tr("^"), this);
    btnOr = new QPushButton(tr("|"), this);
    btnAnd = new QPushButton(tr("&&"), this);
    btnSequence = new QPushButton(tr("()"), this);

    btnSelect->setEnabled(false);
    btnUnselect->setEnabled(false);
    btnRemove->setEnabled(false);

    btnStar->setEnabled(false);
    btnPlus->setEnabled(false);
    btnQuestion->setEnabled(false);
    btnLimit->setEnabled(false);
    btnOr->setEnabled(false);
    btnAnd->setEnabled(false);
    btnSequence->setEnabled(false);

    grid->addWidget(btnSelect,4,2);
    grid->addWidget(btnUnselect,5,2);
    grid->addWidget(btnAdd,1,2);
    grid->addWidget(btnRemove,2,2);

    grid->addWidget(btnStar,11,3);
    grid->addWidget(btnPlus,12,3);
    grid->addWidget(btnQuestion,11,4);
    grid->addWidget(btnOr,11,5);
    grid->addWidget(btnAnd,12,5);
    grid->addWidget(btnLimit,11,6);
    grid->addWidget(btnSequence,12,4);

    lblMBF = new QLabel(tr("MBFs"), this);
    lblMSF = new QLabel(tr("MSF:"), this);
    lblFormula = new QLabel(tr("Formula:"), this);
    lblDescription = new QLabel(tr("Description:"), this);
    lblFGColor = new QLabel(tr("Foregroud Color:"), this);
    lblBGColor = new QLabel(tr("Background Color:"), this);

    grid->addWidget(lblMBF,0,0);
    grid->addWidget(lblMSF,0,3);
    grid->addWidget(lblFormula,10,2,Qt::AlignCenter);
    grid->addWidget(lblDescription,0,5);
    grid->addWidget(lblFGColor,2,5,1,2);
    grid->addWidget(lblBGColor,4,5,1,2);

    editDescription = new QTextEdit(this);

    editFormula = new QLineEdit(this);
    editFormula->setReadOnly(true);
    editLimit = new QLineEdit(this);
    editLimit->setMaximumWidth(90);

    grid->addWidget(editLimit,12,6);
    grid->addWidget(editFormula,10,3,1,4);
    grid->addWidget(editDescription,1,5,1,2);

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

    grid->addWidget(colorfgcolor,3,5,1,2);
    grid->addWidget(colorbgcolor,5,5,1,2);

    cbMSF = new QComboBox(this);

    grid->addWidget(cbMSF,0,4);

    listMBF = new QListWidget(this);
    listMBF->setSelectionMode(QAbstractItemView::SingleSelection);

    grid->addWidget(listMBF,1,0,6,2);

    treeMSF = new QTreeWidget(this);
    treeMSF->setColumnCount(3);
    QStringList columnsD;
    columnsD << "Name" << "MBF" << "Operation";
    treeMSF->setHeaderLabels(columnsD);
    treeMSF->setSelectionMode(QAbstractItemView::MultiSelection);

    grid->addWidget(treeMSF,1,3,9,2);

    treeMBFdesc = new QTreeWidget(this);
    treeMBFdesc->setColumnCount(4);
    QStringList columnsdesc;
    columnsdesc << QString() << "Feature" << "Relation" << "Value";
    treeMBFdesc->setHeaderLabels(columnsdesc);
    treeMBFdesc->setSelectionMode(QAbstractItemView::SingleSelection);

    grid->addWidget(treeMBFdesc,7,0,6,2);


    /** Add Action widget block **/
#if 0
    QGridLayout *actiongrid = new QGridLayout();
#endif

    lblActions = new QLabel(tr("Actions"), this);
    lblIncludes = new QLabel(tr("Includes:"), this);
    lblMembers = new QLabel(tr("Members:"), this);
    lblInit = new QLabel(tr("Init:"), this);
    lblAfter = new QLabel(tr("After:"), this);
    lblActionMSF = new QLabel(tr("MSF:"), this);
    lblReturns = new QLabel(tr("Returns"), this);
    lblMSFAction = new QLabel(tr("MSF Actions:"), this);

#if 0
    actiongrid->addWidget(lblActions,0,0,1,4,Qt::AlignCenter);
    actiongrid->addWidget(lblIncludes,1,0);
    actiongrid->addWidget(lblMembers,1,2);
    actiongrid->addWidget(lblInit,3,0);
    actiongrid->addWidget(lblAfter,3,2);
    actiongrid->addWidget(lblMSFAction,5,0);
    actiongrid->addWidget(lblActionMSF,8,0);
    actiongrid->addWidget(lblReturns,10,0);
#else
    grid->addWidget(lblActions,0,7,1,4,Qt::AlignCenter);
    grid->addWidget(lblIncludes,1,7);
    grid->addWidget(lblMembers,1,9);
    grid->addWidget(lblInit,3,7);
    grid->addWidget(lblAfter,3,9);
    grid->addWidget(lblMSFAction,5,7);
    grid->addWidget(lblActionMSF,8,7);
    grid->addWidget(lblReturns,10,7);
#endif
    editIncludes = new QTextEdit(this);
    editMembers = new QTextEdit(this);
    editInit = new QTextEdit(this);
    editAfter = new QTextEdit(this);
    editActions = new QTextEdit(this);

#if 0
    actiongrid->addWidget(editIncludes,1,1);
    actiongrid->addWidget(editMembers,1,3);
    actiongrid->addWidget(editInit,3,1);
    actiongrid->addWidget(editAfter,3,3);
    actiongrid->addWidget(editActions,5,1,3,3);
#else
    grid->addWidget(editIncludes,1,8);
    grid->addWidget(editMembers,1,10);
    grid->addWidget(editInit,3,8);
    grid->addWidget(editAfter,3,10);
    grid->addWidget(editActions,5,8,3,3);
#endif
    cbActionMSF = new QComboBox(this);
#if 0
    actiongrid->addWidget(cbActionMSF,8,1);
#else
    grid->addWidget(cbActionMSF,8,8);
#endif
    btnText = new QPushButton(tr("Text"), this);
    btnText->setMaximumWidth(100);
    btnPOS = new QPushButton(tr("pos"), this);
    btnPOS->setMaximumWidth(100);
    btnLength = new QPushButton(tr("Length"), this);
    btnLength->setMaximumWidth(100);
    btnNumber = new QPushButton(tr("Number"), this);
    btnNumber->setMaximumWidth(100);
#if 0
    actiongrid->addWidget(btnText,8,2);
    actiongrid->addWidget(btnPOS,8,3);
    actiongrid->addWidget(btnLength,9,2);
    actiongrid->addWidget(btnNumber,9,3);
#else
    grid->addWidget(btnText,8,9);
    grid->addWidget(btnPOS,8,10);
    grid->addWidget(btnLength,9,9);
    grid->addWidget(btnNumber,9,10);
#endif
    editReturns = new QLineEdit(this);
#if 0
    actiongrid->addWidget(editReturns,10,1,1,3);
#else
    grid->addWidget(editReturns,10,8,1,3);
#endif

#if 0
    QScrollArea *sa = new QScrollArea(this);
    sa->setLayout(actiongrid);
    grid->addWidget(sa,0,7,12,4);
#endif
    /** Done **/

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

    if(_atagger->tagTypeVector->count() == 0) {
        return;
    }

    /** Initialize a copy of the MSFs **/
    _atagger->tempMSFVector = new QVector<MSFormula*>();

    QString ttFName;
    ttFName = _atagger->tagtypeFile;

    QFile ITfile(ttFName);
    if (!ITfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        close();
    }

    QByteArray tagtypedata = ITfile.readAll();
    ITfile.close();

    QJson::Parser parser;
    bool ok;

    QVariantMap result = parser.parse (tagtypedata,&ok).toMap();

    if (!ok) {
        close();
    }

    if(!(result.value("MSFs").isNull())) {
        /** The tagtype file contains MSFs **/

        foreach(QVariant msfsData, result.value("MSFs").toList()) {

            /** List of variables for each MSFormula **/
            QString bgcolor;
            QString fgcolor;
            QString name;
            QString description;
            int i;
            int usedCount;

            /** This is an MSFormula **/
            QVariantMap msformulaData = msfsData.toMap();

            name = msformulaData.value("name").toString();
            description = msformulaData.value("description").toString();
            fgcolor = msformulaData.value("fgcolor").toString();
            bgcolor = msformulaData.value("bgcolor").toString();
            i = msformulaData.value("i").toInt();
            usedCount = msformulaData.value("usedCount").toInt();

            MSFormula* msf = new MSFormula(name, NULL);
            msf->fgcolor = fgcolor;
            msf->bgcolor = bgcolor;
            msf->description = description;
            msf->i = i;
            msf->usedCount = usedCount;
            _atagger->tempMSFVector->append(msf);

            /** Get MSFormula MSFs **/
            foreach(QVariant msfData, msformulaData.value("MSFs").toList()) {
                readMSF(msf, msfData, msf);
            }
        }
    }
    /** Done **/

    QStringList tagtypes;
    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        tagtypes.append(_atagger->tagTypeVector->at(i)->tag);
    }
    tagtypes.append("NONE");

    if(_atagger->tempMSFVector->count() != 0) {
        currentF = (MSFormula*)(_atagger->tempMSFVector->at(0));
        for(int i=0; i<_atagger->tempMSFVector->count(); i++) {
            tagtypes.append(_atagger->tempMSFVector->at(i)->name);
            cbMSF->addItem(_atagger->tempMSFVector->at(i)->name);
        }

        cbMSF->setCurrentIndex(0);
        currentF->buildTree(treeMSF);
        editFormula->setText(currentF->print());
        editActions->setText(currentF->actions);
        editDescription->setText(currentF->description);
        colorfgcolor->setColor(QColor(currentF->fgcolor));
        colorbgcolor->setColor(QColor(currentF->bgcolor));

        btnSelect->setEnabled(true);
        btnUnselect->setEnabled(true);
        btnRemove->setEnabled(true);

        btnStar->setEnabled(true);
        btnPlus->setEnabled(true);
        btnQuestion->setEnabled(true);
        btnLimit->setEnabled(true);
        btnOr->setEnabled(true);
        btnAnd->setEnabled(true);
        btnSequence->setEnabled(true);
    }
    listMBF->addItems(tagtypes);

    /** Connect Signals **/
    /*
    connect(colorfgcolor, SIGNAL(currentIndexChanged(QString)), this, SLOT(fgcolor_changed(QString)));
    connect(colorbgcolor, SIGNAL(currentIndexChanged(QString)), this, SLOT(bgcolor_changed(QString)));
    connect(editDescription, SIGNAL(textChanged()), this, SLOT(description_edited()));
    connect(btnSelect, SIGNAL(clicked()), this, SLOT(btnSelect_clicked()));
    connect(btnUnselect, SIGNAL(clicked()), this, SLOT(btnUnselect_clicked()));
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(btnAdd_clicked()));
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(btnRemove_clicked()));
    connect(btnStar, SIGNAL(clicked()), this, SLOT(btnStar_clicked()));
    connect(btnPlus, SIGNAL(clicked()), this, SLOT(btnPlus_clicked()));
    connect(btnQuestion, SIGNAL(clicked()), this, SLOT(btnQuestion_clicked()));
    connect(btnLimit, SIGNAL(clicked()), this, SLOT(btnLimit_clicked()));
    connect(btnOr, SIGNAL(clicked()), this, SLOT(btnOr_clicked()));
    connect(btnAnd, SIGNAL(clicked()), this, SLOT(btnAnd_clicked()));
    connect(btnSequence, SIGNAL(clicked()), this, SLOT(btnSequence_clicked()));
    connect(cbMSF, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbMSF_changed(QString)));
    connect(listMBF, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listMBF_itemclicked(QListWidgetItem*)));
    */
    connect_Signals();
    /** Connections Done **/
}

bool CustomizeMSFView::readMSF(MSFormula* formula, QVariant data, MSF *parent) {
    /** Common variables in MSFs **/
    QString name;
    QString parentName;
    QString type;

    QVariantMap msfData = data.toMap();
    name = msfData.value("name").toString();
    parentName = msfData.value("parent").toString();
    type = msfData.value("type").toString();

    if(type == "mbf") {
        /** This is MBF **/
        QString bf = msfData.value("MBF").toString();
        bool isF = msfData.value("isFormula").toBool();

        /** initialize MBF **/
        MBF* mbf = new MBF(name,parent,bf,isF);
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

void CustomizeMSFView::description_edited() {
    if(currentF == NULL) {
        return;
    }

    isDirty = true;
    currentF->description = editDescription->toPlainText();
}

void CustomizeMSFView::fgcolor_changed(QString color) {
    if(color.isNull() || color.isEmpty()) {
        return;
    }

    if(currentF == NULL) {
        return;
    }

    isDirty = true;
    currentF->fgcolor = color;
}

void CustomizeMSFView::bgcolor_changed(QString color) {
    if(color.isNull() || color.isEmpty()) {
        return;
    }

    if(currentF == NULL) {
        return;
    }

    isDirty = true;
    currentF->bgcolor = color;
}

void CustomizeMSFView::btnAdd_clicked() {

    bool ok;
    QString msfName = QInputDialog::getText(this,"Tag Name","Please insert a formula Name", QLineEdit::Normal,
                                            QString(), &ok);
    if(!ok || msfName.isEmpty()) {
        return;
    }

    for(int i=0; i<_atagger->tempMSFVector->count(); i++) {
        const MSFormula* msf = _atagger->tempMSFVector->at(i);
        if(msf->name == msfName) {
            QMessageBox::warning(this, "Warning", "This formula Name already exists!");
            return;
        }
    }
    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        const TagType* tt = _atagger->tagTypeVector->at(i);
        if(tt->tag == msfName) {
            QMessageBox::warning(this, "Warning", "Name conflict with an MBF!");
            return;
        }
    }

    MSFormula* formula = new MSFormula(msfName, NULL);
    currentF = formula;
    _atagger->tempMSFVector->append(formula);
    listMBF->addItem(formula->name);
    isDirty = true;

    disconnect_Signals();

    btnRemove->setEnabled(true);

    if(!btnSelect->isEnabled()) {
        btnSelect->setEnabled(true);
        btnUnselect->setEnabled(true);
        btnRemove->setEnabled(true);
        btnStar->setEnabled(true);
        btnPlus->setEnabled(true);
        btnQuestion->setEnabled(true);
        btnLimit->setEnabled(true);
        btnOr->setEnabled(true);
        btnAnd->setEnabled(true);
        btnSequence->setEnabled(true);
    }
    treeMSF->clear();
    editActions->clear();
    editDescription->clear();
    listMBF->clearSelection();
    treeMBFdesc->clear();

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

    formula->fgcolor = colorfgcolor->color().name();
    formula->bgcolor = colorbgcolor->color().name();

    cbMSF->addItem(msfName);
    cbMSF->setCurrentIndex(cbMSF->findText(msfName));
    editFormula->clear();

    connect_Signals();
}

void CustomizeMSFView::btnRemove_clicked() {
    if(_atagger->tempMSFVector->count() == 0) {
        return;
    }
    //disconnect_Signals();

    QString msfRemoved = cbMSF->currentText();
    for(int i=0; i<_atagger->tempMSFVector->count(); i++) {
        const MSFormula * msf = _atagger->tempMSFVector->at(i);
        if(msf->name == msfRemoved) {
            if(msf->usedCount != 0) {
                QMessageBox::warning(this, "Warning", "Formula used in building other formulae\nRemove it from them first");
                return;
            }
            else {
                if(!(_atagger->tempMSFVector->at(i)->removeSelfFromMap(_atagger->tempMSFVector->at(i)->map))) {
                    QMessageBox::warning(this, "Warning", "Couldn't remove Formula");
                    return;
                }
                delete (_atagger->tempMSFVector->at(i));
                _atagger->tempMSFVector->remove(i);
                break;
            }
        }
    }

    listMBF->clear();
    if(_atagger->tagTypeVector->count() != 0) {
        for(int i=0; i< _atagger->tagTypeVector->count(); i++) {
            listMBF->addItem(_atagger->tagTypeVector->at(i)->tag);
        }
    }
    listMBF->addItem("NONE");

    if(_atagger->tempMSFVector->count() != 0) {
        for(int i=0; i< _atagger->tempMSFVector->count(); i++) {
            listMBF->addItem(_atagger->tempMSFVector->at(i)->name);
        }
    }


    cbMSF->removeItem(cbMSF->currentIndex());

    if(_atagger->tempMSFVector->count() == 0) {
        currentF = NULL;
        btnSelect->setEnabled(false);
        btnUnselect->setEnabled(false);
        btnRemove->setEnabled(false);
        btnStar->setEnabled(false);
        btnPlus->setEnabled(false);
        btnQuestion->setEnabled(false);
        btnLimit->setEnabled(false);
        btnOr->setEnabled(false);
        btnAnd->setEnabled(false);
        btnSequence->setEnabled(false);
    }
    isDirty = true;

    //connect_Signals();
}

void CustomizeMSFView::btnSelect_clicked() {

    QString name = "s";

    if(!(listMBF->selectedItems().isEmpty())) {

        QListWidgetItem* item = listMBF->selectedItems().at(0);
        QString bf = item->text();
        name.append(QString::number(currentF->i));

        for(int i=0; i< _atagger->tempMSFVector->count(); i++) {
            if(_atagger->tempMSFVector->at(i)->name == bf) {

                /// Insert as top child in MSF
                MBF* mbf = new MBF(name, currentF, bf, true);
                if(!(currentF->addMSF(mbf))) {
                    QMessageBox::warning(this, "Warning", "Couldn't process entered data!");
                    return;
                }
                _atagger->tempMSFVector->at(i)->usedCount = _atagger->tempMSFVector->at(i)->usedCount + 1;

                QStringList data;
                data << name << bf << QString();
                QTreeWidgetItem* tItem = new QTreeWidgetItem(treeMSF, data);
                editFormula->setText(currentF->print());
                isDirty = true;
                return;
            }
        }

        /// Insert as top child in MSF
        MBF* mbf = new MBF(name, currentF, bf);
        if(!(currentF->addMSF(mbf))) {
            QMessageBox::warning(this, "Warning", "Couldn't process entered data!");
            return;
        }

        QStringList data;
        data << name << bf << QString();
        QTreeWidgetItem* tItem = new QTreeWidgetItem(treeMSF, data);
        editFormula->setText(currentF->print());
        isDirty = true;
    }
    else {
        /// No MBFs are selected, invalid move
        QMessageBox::warning(this, "Warning", "Invalid Move!");
        return;
    }
}

void CustomizeMSFView::btnUnselect_clicked() {

    if(!(treeMSF->selectedItems().isEmpty())) {

        if(treeMSF->selectedItems().count() == 1) {
            /// One item is selected to remove

            QTreeWidgetItem* item = treeMSF->selectedItems().at(0);
            QString name = item->text(0);

            /// Check if selected item is a top level item in tree
            if(!(item->parent())) {

                /// Remove as top child in MSF
                if(!(currentF->removeMSF(currentF->name, name))) {
                    QMessageBox::warning(this, "Warning", "Couldn't remove entry!");
                    return;
                }
                int index = treeMSF->indexOfTopLevelItem(item);
                delete treeMSF->takeTopLevelItem(index);

                editFormula->setText(currentF->print());
                isDirty = true;
                return;
            }

            MSF* msf = currentF->map.value(item->parent()->text(0));
            if(!(msf->isSequential())) {
                QMessageBox::warning(this, "Warning", "Invalid Move: Can't remove this item!");
                return;
            }

            SequentialF* parent = (SequentialF*)msf;
            if(parent->vector.count() < 2) {
                QMessageBox::warning(this, "Warning", "Invalid Move: Can't remove this item!");
                return;
            }

            /// Remove as subformula
            if(!(currentF->removeMSF(parent->name, name))) {
                QMessageBox::warning(this, "Warning", "Couldn't remove entry!");
                return;
            }
            int index = item->parent()->indexOfChild(item);
            delete item->parent()->takeChild(index);
            editFormula->setText(currentF->print());
            isDirty = true;
        }
        else {
            /// More than one item is selected!!

            QMessageBox::warning(this, "Warning", "Select one entry to remove at a time!");
            return;
        }
    }
    else {
        /// No MBFs are selected, invalid move
        QMessageBox::warning(this, "Warning", "No selected items in formula tree!");
        return;
    }
}

void CustomizeMSFView::btnStar_clicked() {
    /// Star is selected as unary operation
    if(treeMSF->selectedItems().count() != 1) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: Only one MSF for unary operations!");
        return;
    }

    QString name = "s";
    QTreeWidgetItem* item = treeMSF->selectedItems().at(0);
    QString childName = item->text(0);
    name.append(QString::number(currentF->i));

    QStringList unaryOp;
    unaryOp << "*" << "+" << "?";
    /// Check if item is a unary operation
    if(item->childCount() == 1 || item->text(3).contains("^") || unaryOp.contains(item->text(3))) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: Can't apply unary operation to a unary formula!");
        return;
    }

    /// Check if parent exist
    if(!(item->parent())) {
        /// Selected item is a top level item in tree

        UNARYF* uf = new UNARYF(name, currentF, STAR);
        if(!(currentF->updateMSF(currentF->name, childName, uf))) {
            QMessageBox::warning(this, "Warning", "Couldn't process entered data!");
            return;
        }

        int index;
        index = treeMSF->indexOfTopLevelItem(item);
        QTreeWidgetItem* newItem = new QTreeWidgetItem(treeMSF, item);
        newItem->setText(0,name);
        newItem->setText(2,"*");

        QTreeWidgetItem* childItem = treeMSF->takeTopLevelItem(index);
        newItem->addChild(childItem);
        editFormula->setText(currentF->print());
        isDirty = true;
        return;
    }

    QString parentName = item->parent()->text(0);
    /// Check if parent is a Unary formula
    if(item->parent()->childCount() == 1) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: Can't apply multiple unary operations to a MBF!");
        return;
    }

    /// No selected item, should insert as top child in main MSF

    /// Check if parent node exists in internal tree
    if(currentF->map.find(parentName) == currentF->map.end()) {
        QMessageBox::warning(this, "Warning", "Can't find parent!");
        return;
    }

    UNARYF* uf = new UNARYF(name, currentF->map.value(parentName), STAR);
    if(!(currentF->updateMSF(parentName, childName, uf))) {
        QMessageBox::warning(this, "Warning", "Couldn't process entered data!");
        return;
    }

    /// Update tree data
    int index;
    index = item->parent()->indexOfChild(item);
    QTreeWidgetItem* newItem = new QTreeWidgetItem(item->parent(),item);
    newItem->setText(0,name);
    newItem->setText(2,"*");

    QTreeWidgetItem* childItem = item->parent()->takeChild(index);
    newItem->addChild(childItem);
    editFormula->setText(currentF->print());
    isDirty = true;
}

void CustomizeMSFView::btnPlus_clicked() {
    /// Plus is selected as unary operation

    if(treeMSF->selectedItems().count() != 1) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: Only one MSF for unary operations!");
        return;
    }

    QString name = "s";
    QTreeWidgetItem* item = treeMSF->selectedItems().at(0);
    QString childName = item->text(0);
    name.append(QString::number(currentF->i));

    QStringList unaryOp;
    unaryOp << "*" << "+" << "?";
    /// Check if item is a unary operation
    if(item->childCount() == 1 || item->text(3).contains("^") || unaryOp.contains(item->text(3))) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: Can't apply unary operation to a unary formula!");
        return;
    }

    /// Check if parent exist
    if(!(item->parent())) {
        /// Selected item is a top level item in tree

        UNARYF* uf = new UNARYF(name, currentF, PLUS);
        if(!(currentF->updateMSF(currentF->name, childName, uf))) {
            QMessageBox::warning(this, "Warning", "Couldn't process entered data!");
            return;
        }

        int index;
        index = treeMSF->indexOfTopLevelItem(item);
        QTreeWidgetItem* newItem = new QTreeWidgetItem(treeMSF, item);
        newItem->setText(0,name);
        newItem->setText(2,"+");

        QTreeWidgetItem* childItem = treeMSF->takeTopLevelItem(index);
        newItem->addChild(childItem);
        editFormula->setText(currentF->print());
        isDirty = true;
        return;
    }

    QString parentName = item->parent()->text(0);
    /// Check if parent is a Unary formula
    if(item->parent()->childCount() == 1) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: Can't apply multiple unary operations to a MBF!");
        return;
    }

    /// No selected item, should insert as top child in main MSF

    /// Check if parent node exists in internal tree
    if(currentF->map.find(parentName) == currentF->map.end()) {
        QMessageBox::warning(this, "Warning", "Can't find parent!");
        return;
    }

    UNARYF* uf = new UNARYF(name, currentF->map.value(parentName), PLUS);
    if(!(currentF->updateMSF(parentName, childName, uf))) {
        QMessageBox::warning(this, "Warning", "Couldn't process entered data!");
        return;
    }

    /// Update tree data
    int index;
    index = item->parent()->indexOfChild(item);
    QTreeWidgetItem* newItem = new QTreeWidgetItem(item->parent(),item);
    newItem->setText(0,name);
    newItem->setText(2,"+");

    QTreeWidgetItem* childItem = item->parent()->takeChild(index);
    newItem->addChild(childItem);
    editFormula->setText(currentF->print());
    isDirty = true;
}

void CustomizeMSFView::btnQuestion_clicked() {
    /// Star is selected as unary operation
    if(treeMSF->selectedItems().count() != 1) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: Only one MSF for unary operations!");
        return;
    }

    QString name = "s";
    QTreeWidgetItem* item = treeMSF->selectedItems().at(0);
    QString childName = item->text(0);
    name.append(QString::number(currentF->i));

    QStringList unaryOp;
    unaryOp << "*" << "+" << "?";
    /// Check if item is a unary operation
    if(item->childCount() == 1 || item->text(3).contains("^") || unaryOp.contains(item->text(3))) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: Can't apply unary operation to a unary formula!");
        return;
    }

    /// Check if parent exist
    if(!(item->parent())) {
        /// Selected item is a top level item in tree

        UNARYF* uf = new UNARYF(name, currentF, KUESTION);
        if(!(currentF->updateMSF(currentF->name, childName, uf))) {
            QMessageBox::warning(this, "Warning", "Couldn't process entered data!");
            return;
        }

        int index;
        index = treeMSF->indexOfTopLevelItem(item);
        QTreeWidgetItem* newItem = new QTreeWidgetItem(treeMSF, item);
        newItem->setText(0,name);
        newItem->setText(2,"?");

        QTreeWidgetItem* childItem = treeMSF->takeTopLevelItem(index);
        newItem->addChild(childItem);
        editFormula->setText(currentF->print());
        isDirty = true;
        return;
    }

    QString parentName = item->parent()->text(0);
    /// Check if parent is a Unary formula
    if(item->parent()->childCount() == 1) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: Can't apply multiple unary operations to a MBF!");
        return;
    }

    /// No selected item, should insert as top child in main MSF

    /// Check if parent node exists in internal tree
    if(currentF->map.find(parentName) == currentF->map.end()) {
        QMessageBox::warning(this, "Warning", "Can't find parent!");
        return;
    }

    UNARYF* uf = new UNARYF(name, currentF->map.value(parentName), KUESTION);
    if(!(currentF->updateMSF(parentName, childName, uf))) {
        QMessageBox::warning(this, "Warning", "Couldn't process entered data!");
        return;
    }

    /// Update tree data
    int index;
    index = item->parent()->indexOfChild(item);
    QTreeWidgetItem* newItem = new QTreeWidgetItem(item->parent(),item);
    newItem->setText(0,name);
    newItem->setText(2,"?");

    QTreeWidgetItem* childItem = item->parent()->takeChild(index);
    newItem->addChild(childItem);
    editFormula->setText(currentF->print());
    isDirty = true;
}

void CustomizeMSFView::btnLimit_clicked() {
    /// Upto is selected as unary operation

    if(treeMSF->selectedItems().count() != 1) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: Only one MSF for unary operations!");
        return;
    }

    bool ok;
    if(editLimit->text().isEmpty()) {
        QMessageBox::warning(this, "Warning", "No limit for upto operation!");
        return;
    }
    int limit = editLimit->text().toInt(&ok);
    if(!ok || limit<=0) {
        QMessageBox::warning(this, "Warning", "Invalid limit for upto operation!");
        return;
    }
    QString opText = "^";
    opText.append(QString::number(limit));

    QString name = "s";
    QTreeWidgetItem* item = treeMSF->selectedItems().at(0);
    QString childName = item->text(0);
    name.append(QString::number(currentF->i));

    QStringList unaryOp;
    unaryOp << "*" << "+" << "?";
    /// Check if item is a unary operation
    if(item->childCount() == 1 || item->text(3).contains("^") || unaryOp.contains(item->text(3))) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: Can't apply unary operation to a unary formula!");
        return;
    }

    /// Check if parent exist
    if(!(item->parent())) {
        /// Selected item is a top level item in tree

        UNARYF* uf = new UNARYF(name, currentF, UPTO, limit);
        if(!(currentF->updateMSF(currentF->name, childName, uf))) {
            QMessageBox::warning(this, "Warning", "Couldn't process entered data!");
            return;
        }

        int index;
        index = treeMSF->indexOfTopLevelItem(item);
        QTreeWidgetItem* newItem = new QTreeWidgetItem(treeMSF, item);
        newItem->setText(0,name);
        newItem->setText(2,opText);

        QTreeWidgetItem* childItem = treeMSF->takeTopLevelItem(index);
        newItem->addChild(childItem);
        editFormula->setText(currentF->print());
        isDirty = true;
        return;
    }

    QString parentName = item->parent()->text(0);
    /// Check if parent is a Unary formula
    if(item->parent()->childCount() == 1) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: Can't apply multiple unary operations to a MBF!");
        return;
    }

    /// No selected item, should insert as top child in main MSF

    /// Check if parent node exists in internal tree
    if(currentF->map.find(parentName) == currentF->map.end()) {
        QMessageBox::warning(this, "Warning", "Can't find parent!");
        return;
    }

    UNARYF* uf = new UNARYF(name, currentF->map.value(parentName), UPTO, limit);
    if(!(currentF->updateMSF(parentName, childName, uf))) {
        QMessageBox::warning(this, "Warning", "Couldn't process entered data!");
        return;
    }

    /// Update tree data
    int index;
    index = item->parent()->indexOfChild(item);
    QTreeWidgetItem* newItem = new QTreeWidgetItem(item->parent(),item);
    newItem->setText(0,name);
    newItem->setText(2,opText);

    QTreeWidgetItem* childItem = item->parent()->takeChild(index);
    newItem->addChild(childItem);
    editFormula->setText(currentF->print());
    isDirty = true;
}

void CustomizeMSFView::btnOr_clicked() {
    /// Selection of or as binary operation

    if(treeMSF->selectedItems().count() != 2) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: Two MSFs for binary operations!");
        return;
    }

    QString name = "s";
    QTreeWidgetItem* item1 = treeMSF->selectedItems().at(0);
    QString childName1 = item1->text(0);
    QTreeWidgetItem* item2 = treeMSF->selectedItems().at(1);
    QString childName2 = item2->text(0);
    name.append(QString::number(currentF->i));

    if((!(item1->parent())) && (!(item2->parent()))) {
        /// Selected item is a top level item in tree

        BINARYF* bf = new BINARYF(name, currentF, OR);
        if(!(currentF->updateMSF(currentF->name, childName1, childName2, bf))) {
            QMessageBox::warning(this, "Warning", "Couldn't process data!");
            return;
        }

        int index1;
        index1 = treeMSF->indexOfTopLevelItem(item1);
        int index2;
        index2 = treeMSF->indexOfTopLevelItem(item2);
        QTreeWidgetItem* newItem = new QTreeWidgetItem(treeMSF, item2);
        newItem->setText(0,name);
        newItem->setText(2,"|");

        QTreeWidgetItem* childItem2 = treeMSF->takeTopLevelItem(index2);
        newItem->addChild(childItem2);
        QTreeWidgetItem* childItem1 = treeMSF->takeTopLevelItem(index1);
        newItem->addChild(childItem1);
        editFormula->setText(currentF->print());
        isDirty = true;
        return;
    }
    else if(!(item1->parent()) || !(item2->parent())) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: The MSFs are not on same level!");
        return;
    }

    /// Check if item is a unary operation
    if(item1->parent()->childCount() == 2) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: Invalid Update!");
        return;
    }

    /// Check if selected MSFs are siblings
    if(item1->parent()->text(0) != item2->parent()->text(0)) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: The MSFs are not on same level!");
        return;
    }
    QString parentName = item1->parent()->text(0);

    BINARYF* bf = new BINARYF(name, currentF->map.value(parentName),OR);
    if(!(currentF->updateMSF(parentName, childName1, childName2, bf))) {
        QMessageBox::warning(this, "Warning", "Couldn't process data!");
        return;
    }

    int index1;
    index1 = item1->parent()->indexOfChild(item1);
    int index2;
    index2 = item2->parent()->indexOfChild(item2);
    QTreeWidgetItem* newItem = new QTreeWidgetItem(item2->parent(), item2);
    newItem->setText(0,name);
    newItem->setText(2,"|");

    QTreeWidgetItem* childItem2 = item2->parent()->takeChild(index2);
    newItem->addChild(childItem2);
    QTreeWidgetItem* childItem1 = item1->parent()->takeChild(index1);
    newItem->addChild(childItem1);
    editFormula->setText(currentF->print());
    isDirty = true;
}

void CustomizeMSFView::btnAnd_clicked() {
    /// Selection of a binary operation

    if(treeMSF->selectedItems().count() != 2) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: Two MSFs for binary operations!");
        return;
    }

    QString name = "s";
    QTreeWidgetItem* item1 = treeMSF->selectedItems().at(0);
    QString childName1 = item1->text(0);
    QTreeWidgetItem* item2 = treeMSF->selectedItems().at(1);
    QString childName2 = item2->text(0);
    name.append(QString::number(currentF->i));

    if((!(item1->parent())) && (!(item2->parent()))) {
        /// Selected item is a top level item in tree

        BINARYF* bf = new BINARYF(name, currentF, AND);
        if(!(currentF->updateMSF(currentF->name, childName1, childName2, bf))) {
            QMessageBox::warning(this, "Warning", "Couldn't process data!");
            return;
        }

        int index1;
        index1 = treeMSF->indexOfTopLevelItem(item1);
        int index2;
        index2 = treeMSF->indexOfTopLevelItem(item2);
        QTreeWidgetItem* newItem = new QTreeWidgetItem(treeMSF, item2);
        newItem->setText(0,name);
        newItem->setText(2,"&");

        QTreeWidgetItem* childItem2 = treeMSF->takeTopLevelItem(index2);
        newItem->addChild(childItem2);
        QTreeWidgetItem* childItem1 = treeMSF->takeTopLevelItem(index1);
        newItem->addChild(childItem1);
        editFormula->setText(currentF->print());
        isDirty = true;
        return;
    }
    else if(!(item1->parent()) || !(item2->parent())) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: The MSFs are not on same level!");
        return;
    }

    /// Check if item is a unary operation
    if(item1->parent()->childCount() == 2) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: Invalid Update!");
        return;
    }

    /// Check if selected MSFs are siblings
    if(item1->parent()->text(0) != item2->parent()->text(0)) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: The MSFs are not on same level!");
        return;
    }
    QString parentName = item1->parent()->text(0);

    BINARYF* bf = new BINARYF(name, currentF->map.value(parentName),AND);
    if(!(currentF->updateMSF(parentName, childName1, childName2, bf))) {
        QMessageBox::warning(this, "Warning", "Couldn't process data!");
        return;
    }

    int index1;
    index1 = item1->parent()->indexOfChild(item1);
    int index2;
    index2 = item2->parent()->indexOfChild(item2);
    QTreeWidgetItem* newItem = new QTreeWidgetItem(item2->parent(), item2);
    newItem->setText(0,name);
    newItem->setText(2,"&");

    QTreeWidgetItem* childItem2 = item2->parent()->takeChild(index2);
    newItem->addChild(childItem2);
    QTreeWidgetItem* childItem1 = item1->parent()->takeChild(index1);
    newItem->addChild(childItem1);
    editFormula->setText(currentF->print());
    isDirty = true;
}

void CustomizeMSFView::btnSequence_clicked() {
    /// Selection of sequential operation

    if(treeMSF->selectedItems().count() < 2) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: Two MSFs for binary operations!");
        return;
    }
    QString name = "s";
    name.append(QString::number(currentF->i));

    QTreeWidgetItem* item1 = treeMSF->selectedItems().at(0);
    if(!(item1->parent())) {
        /// First item is a top level item
        /// Check if other selected items are top level too
        foreach(QTreeWidgetItem* item, treeMSF->selectedItems()) {

            if(item->parent()) {
                QMessageBox::warning(this, "Warning", "Invalid Formula: The MSFs are not on same level!");
                return;
            }
        }

        SequentialF* sf = new SequentialF(name, currentF);
        QTreeWidgetItem* newItem = new QTreeWidgetItem(treeMSF, treeMSF->selectedItems().last());
        newItem->setText(0,name);
        newItem->setText(2,"()");

        QVector<QString> msfs;
        foreach(QTreeWidgetItem* item, treeMSF->selectedItems()) {
            msfs.append(item->text(0));

            int index;
            index = treeMSF->indexOfTopLevelItem(item);
            QTreeWidgetItem* childItem = treeMSF->takeTopLevelItem(index);
            newItem->addChild(childItem);
        }

        if(!(currentF->updateMSF(currentF->name, &msfs, sf))) {
            QMessageBox::warning(this, "Warning", "Couldn't process data!");
            return;
        }
        editFormula->setText(currentF->print());
        isDirty = true;
        return;
    }

    QString parentName = item1->parent()->text(0);
    foreach(QTreeWidgetItem* item, treeMSF->selectedItems()) {

        if(item->parent()->text(0) != parentName) {
            QMessageBox::warning(this, "Warning", "Invalid Formula: The MSFs are not on same level!");
            return;
        }
    }

    QStringList biOps;
    biOps << "|" << "&";
    if(biOps.contains(item1->parent()->text(3))) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: The MSFs belong to a binary operation!");
        return;
    }

    if(item1->parent()->text(3).compare("()")==0 && treeMSF->selectedItems().count()==2) {
        QMessageBox::warning(this, "Warning", "Invalid Formula: MSFs are already sequential!");
        return;
    }

    SequentialF* sf = new SequentialF(name, currentF->map.value(parentName));
    QTreeWidgetItem* newItem = new QTreeWidgetItem(item1->parent(), treeMSF->selectedItems().last());
    newItem->setText(0,name);
    newItem->setText(2,"()");

    QVector<QString> msfs;
    foreach(QTreeWidgetItem* item, treeMSF->selectedItems()) {
        msfs.append(item->text(0));

        int index;
        index = item->parent()->indexOfChild(item);
        QTreeWidgetItem* childItem = item->parent()->takeChild(index);
        newItem->addChild(childItem);
    }

    if(!(currentF->updateMSF(parentName, &msfs, sf))) {
        QMessageBox::warning(this, "Warning", "Couldn't process data!");
        return;
    }
    editFormula->setText(currentF->print());
    isDirty = true;
}

void CustomizeMSFView::btnText_clicked() {
    if(currentF == NULL) {
        return;
    }

    QString text = "$";
    text.append(cbActionMSF->currentText());
    text.append(".text");
    editActions->textCursor().insertText(text);
    isDirty = true;
}

void CustomizeMSFView::btnPOS_clicked() {
    if(currentF == NULL) {
        return;
    }

    QString text = "$";
    text.append(cbActionMSF->currentText());
    text.append(".position");
    editActions->textCursor().insertText(text);
    isDirty = true;
}

void CustomizeMSFView::btnLength_clicked() {
    if(currentF == NULL) {
        return;
    }

    QString text = "$";
    text.append(cbActionMSF->currentText());
    text.append(".length");
    editActions->textCursor().insertText(text);
    isDirty = true;
}

void CustomizeMSFView::btnNumber_clicked() {
    if(currentF == NULL) {
        return;
    }

    QString text = "$";
    text.append(cbActionMSF->currentText());
    text.append(".number");
    editActions->textCursor().insertText(text);
    isDirty = true;
}

void CustomizeMSFView::includes_edited() {
    if(currentF == NULL) {
        return;
    }

    currentF->includes = editIncludes->toPlainText();
    isDirty = true;
}

void CustomizeMSFView::members_edited() {
    if(currentF == NULL) {
        return;
    }

    currentF->members = editMembers->toPlainText();
    isDirty = true;
}

void CustomizeMSFView::init_edited() {
    if(currentF == NULL) {
        return;
    }

    QString msfName = cbActionMSF->currentText();
    if(msfName.isEmpty()) {
        return;
    }
    MSF* msf = currentF->map.value(msfName);
    msf->init = editInit->toPlainText();
    isDirty = true;
}

void CustomizeMSFView::after_edited() {
    if(currentF == NULL) {
        return;
    }

    QString msfName = cbActionMSF->currentText();
    if(msfName.isEmpty()) {
        return;
    }
    MSF* msf = currentF->map.value(msfName);
    msf->after = editAfter->toPlainText();
    isDirty = true;
}

void CustomizeMSFView::actions_edited() {
    if(currentF == NULL) {
        return;
    }

    QString msfName = cbActionMSF->currentText();
    if(msfName.isEmpty()) {
        return;
    }
    MSF* msf = currentF->map.value(msfName);
    msf->actions = editActions->toPlainText();
    isDirty = true;
}

void CustomizeMSFView::returns_edited(QString returns) {
    if(currentF == NULL) {
        return;
    }

    QString msfName = cbActionMSF->currentText();
    if(msfName.isEmpty()) {
        return;
    }
    MSF* msf = currentF->map.value(msfName);
    msf->returns = returns;
    isDirty = true;
}

void CustomizeMSFView::cbMSF_changed(QString name) {
    treeMSF->clear();
    editLimit->clear();
    editActions->clear();
    editFormula->clear();
    if(name.isEmpty() || name.isNull()) {
        return;
    }

    disconnect_Signals();
    for(int i=0; i<_atagger->tempMSFVector->count(); i++) {
        MSFormula * msf = _atagger->tempMSFVector->at(i);
        if(msf->name == name) {
            btnSelect->setEnabled(true);
            btnUnselect->setEnabled(true);
            msf->buildTree(treeMSF);
            editActions->setText(msf->actions);
        }
    }
    connect_Signals();
}

void CustomizeMSFView::listMBF_itemclicked(QListWidgetItem *item) {

    disconnect_Signals();
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
    connect_Signals();
}

void CustomizeMSFView::save() {
    /** Replace _atagger msfVector by _atagger->tempMSFVector **/
    for(int i=0; i<_atagger->msfVector->count(); i++) {
        delete (_atagger->msfVector->at(i));
    }
    _atagger->msfVector->clear();
    _atagger->msfVector = _atagger->tempMSFVector;
    _atagger->tempMSFVector = NULL;

    /** Save data to output file **/
    QByteArray msfsData = _atagger->dataInJsonFormat(sarfTTV);
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

    QFile tfile(fileName);
    if (!tfile.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this,"Warning","Can't open tagtypes file to Save");
        return;
    }
    _atagger->tagtypeFile = fileName;

    QTextStream outtags(&tfile);
    outtags << msfsData;
    tfile.close();
    isDirty = false;
}

void CustomizeMSFView::disconnect_Signals() {

    disconnect(btnText, SIGNAL(clicked()), this, SLOT(btnText_clicked()));
    disconnect(btnPOS, SIGNAL(clicked()), this, SLOT(btnPOS_clicked()));
    disconnect(btnLength, SIGNAL(clicked()), this, SLOT(btnLength_clicked()));
    disconnect(btnNumber, SIGNAL(clicked()), this, SLOT(btnNumber_clicked()));
    disconnect(editIncludes, SIGNAL(textChanged()), this, SLOT(includes_edited()));
    disconnect(editMembers, SIGNAL(textChanged()), this, SLOT(members_edited()));
    disconnect(editInit, SIGNAL(textChanged()), this, SLOT(init_edited()));
    disconnect(editAfter, SIGNAL(textChanged()), this, SLOT(after_edited()));
    disconnect(editActions, SIGNAL(textChanged()), this, SLOT(actions_edited()));
    disconnect(editReturns, SIGNAL(textChanged(QString)), this, SLOT(returns_edited(QString)));
    disconnect(colorfgcolor, SIGNAL(currentIndexChanged(QString)), this, SLOT(fgcolor_changed(QString)));
    disconnect(colorbgcolor, SIGNAL(currentIndexChanged(QString)), this, SLOT(bgcolor_changed(QString)));
    disconnect(editDescription, SIGNAL(textChanged()), this, SLOT(description_edited()));
    disconnect(btnSelect, SIGNAL(clicked()), this, SLOT(btnSelect_clicked()));
    disconnect(btnUnselect, SIGNAL(clicked()), this, SLOT(btnUnselect_clicked()));
    disconnect(btnAdd, SIGNAL(clicked()), this, SLOT(btnAdd_clicked()));
    disconnect(btnRemove, SIGNAL(clicked()), this, SLOT(btnRemove_clicked()));
    disconnect(cbMSF, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbMSF_changed(QString)));
    disconnect(listMBF, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listMBF_itemclicked(QListWidgetItem*)));
    disconnect(btnStar, SIGNAL(clicked()), this, SLOT(btnStar_clicked()));
    disconnect(btnPlus, SIGNAL(clicked()), this, SLOT(btnPlus_clicked()));
    disconnect(btnQuestion, SIGNAL(clicked()), this, SLOT(btnQuestion_clicked()));
    disconnect(btnLimit, SIGNAL(clicked()), this, SLOT(btnLimit_clicked()));
    disconnect(btnOr, SIGNAL(clicked()), this, SLOT(btnOr_clicked()));
    disconnect(btnAnd, SIGNAL(clicked()), this, SLOT(btnAnd_clicked()));
    disconnect(btnSequence, SIGNAL(clicked()), this, SLOT(btnSequence_clicked()));
}

void CustomizeMSFView::connect_Signals() {

    connect(btnText, SIGNAL(clicked()), this, SLOT(btnText_clicked()));
    connect(btnPOS, SIGNAL(clicked()), this, SLOT(btnPOS_clicked()));
    connect(btnLength, SIGNAL(clicked()), this, SLOT(btnLength_clicked()));
    connect(btnNumber, SIGNAL(clicked()), this, SLOT(btnNumber_clicked()));
    connect(editIncludes, SIGNAL(textChanged()), this, SLOT(includes_edited()));
    connect(editMembers, SIGNAL(textChanged()), this, SLOT(members_edited()));
    connect(editInit, SIGNAL(textChanged()), this, SLOT(init_edited()));
    connect(editAfter, SIGNAL(textChanged()), this, SLOT(after_edited()));
    connect(editActions, SIGNAL(textChanged()), this, SLOT(actions_edited()));
    connect(editReturns, SIGNAL(textChanged(QString)), this, SLOT(returns_edited(QString)));
    connect(colorfgcolor, SIGNAL(currentIndexChanged(QString)), this, SLOT(fgcolor_changed(QString)));
    connect(colorbgcolor, SIGNAL(currentIndexChanged(QString)), this, SLOT(bgcolor_changed(QString)));
    connect(editDescription, SIGNAL(textChanged()), this, SLOT(description_edited()));
    connect(btnSelect, SIGNAL(clicked()), this, SLOT(btnSelect_clicked()));
    connect(btnUnselect, SIGNAL(clicked()), this, SLOT(btnUnselect_clicked()));
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(btnAdd_clicked()));
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(btnRemove_clicked()));
    connect(cbMSF, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbMSF_changed(QString)));
    connect(listMBF, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listMBF_itemclicked(QListWidgetItem*)));
    connect(btnStar, SIGNAL(clicked()), this, SLOT(btnStar_clicked()));
    connect(btnPlus, SIGNAL(clicked()), this, SLOT(btnPlus_clicked()));
    connect(btnQuestion, SIGNAL(clicked()), this, SLOT(btnQuestion_clicked()));
    connect(btnLimit, SIGNAL(clicked()), this, SLOT(btnLimit_clicked()));
    connect(btnOr, SIGNAL(clicked()), this, SLOT(btnOr_clicked()));
    connect(btnAnd, SIGNAL(clicked()), this, SLOT(btnAnd_clicked()));
    connect(btnSequence, SIGNAL(clicked()), this, SLOT(btnSequence_clicked()));
}

void CustomizeMSFView::closeEvent(QCloseEvent *event) {

    if(isDirty) {
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

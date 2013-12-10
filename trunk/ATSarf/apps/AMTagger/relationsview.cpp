#include "relationsview.h"
#include "global.h"

RelationsView::RelationsView(MSFormula* formula, bool* isDirty, QWidget *parent):
        QMainWindow(parent)
{
    this->isDirty = isDirty;
    *(isDirty) = false;
    this->formula = formula;
    this->parent = parent;
    for(int i=0; i<formula->relationVector.count(); i++) {
        const Relation* rel = formula->relationVector.at(i);
        Relation* relation = new Relation(rel->name, rel->entity1, rel->e1Label, rel->entity2,
                                          rel->e2Label, rel->edge, rel->edgeLabel);
        vector.append(relation);
    }
    this->currentR = NULL;

    QGridLayout *grid = new QGridLayout();

    treeMSF = new QTreeWidget(this);
    treeMSF->setColumnCount(3);
    QStringList columnsD;
    columnsD << "Name" << "MBF" << "Operation";
    treeMSF->setHeaderLabels(columnsD);
    treeMSF->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeMSF->setStyleSheet("QTreeWidget::branch:has-siblings:!adjoins-item {border-image: url(./resources/vline.png) 0;}"
        "QTreeWidget::branch:has-siblings:adjoins-item { border-image: url(./resources/branch-more.png) 0;}"
        "QTreeWidget::branch:!has-children:!has-siblings:adjoins-item { border-image: url(./resources/branch-end.png) 0;}"
        "QTreeWidget::branch:has-children:!has-siblings:closed,QTreeWidget::branch:closed:has-children:has-siblings {border-image: none;image: url(./resources/branch-closed.png);}"
        "QTreeWidget::branch:open:has-children:!has-siblings,QTreeWidget::branch:open:has-children:has-siblings  {border-image: none;image: url(./resources/branch-open.png);}"
        );
    grid->addWidget(treeMSF,0,0,7,1);

    lblName = new QLabel(tr("Relation Name"), this);
    lblEdge = new QLabel(tr("Edge"), this);
    lblE1 = new QLabel(tr("Entity 1"), this);
    lblE2 = new QLabel(tr("Entity 2"), this);
    lblMatch1 = new QLabel(tr("Match"), this);
    lblMatch2 = new QLabel(tr("Match"), this);
    lblNodeLabel = new QLabel(tr("Node Label"), this);
    lblEdgeLabel = new QLabel(tr("Edge Label"), this);

    grid->addWidget(lblName,0,1);
    grid->addWidget(lblEdge,5,1);
    grid->addWidget(lblE1,2,1);
    grid->addWidget(lblE2,3,1);
    grid->addWidget(lblMatch1,1,2);
    grid->addWidget(lblMatch2,4,2);
    grid->addWidget(lblNodeLabel,1,3);
    grid->addWidget(lblEdgeLabel,4,3);

    btnAdd = new QPushButton(tr("Add\nRelation"), this);
    btnRemove = new QPushButton(tr("Remove\nRelation"), this);

    grid->addWidget(btnAdd,6,3);
    grid->addWidget(btnRemove,6,2);

    QStringList labels;
    labels << "text" << "position" << "length" << "number";
    QStringList entities = formula->getMSFNames();
    cbName = new QComboBox(this);
    for(int i=0; i< vector.count(); i++) {
        cbName->addItem(vector.at(i)->name);
    }
    cbE1 = new QComboBox(this);
    cbE1->addItems(entities);
    cbE2 = new QComboBox(this);
    cbE2->addItems(entities);
    cbEdge = new QComboBox(this);
    entities.prepend("");
    cbEdge->addItems(entities);
    cbE1Label = new QComboBox(this);
    cbE1Label->setEditable(true);
    cbE1Label->addItems(labels);
    cbE2Label = new QComboBox(this);
    cbE2Label->setEditable(true);
    cbE2Label->addItems(labels);
    cbEdgeLabel = new QComboBox(this);
    cbEdgeLabel->setEditable(true);
    cbEdgeLabel->addItems(labels);

    grid->addWidget(cbName,0,2,1,2);
    grid->addWidget(cbEdge,5,2);
    grid->addWidget(cbE1,2,2);
    grid->addWidget(cbE2,3,2);
    grid->addWidget(cbEdgeLabel,5,3);
    grid->addWidget(cbE1Label,2,3);
    grid->addWidget(cbE2Label,3,3);

    QWidget *widget = new QWidget(this);
    widget->setLayout(grid);
    setCentralWidget(widget);
    setWindowTitle(tr("Binary Semantic Relation Editor"));

    /** Set MSF tree and current relations **/

    this->formula->buildTree(treeMSF);

    if(!(vector.isEmpty())) {
        currentR = vector[0];
        cbName->setCurrentIndex(0);
        cbE1->setCurrentIndex(cbE1->findText(currentR->entity1->name));
        cbE2->setCurrentIndex(cbE2->findText(currentR->entity2->name));
        if(currentR->edge == NULL) {
            cbEdge->setCurrentIndex(0);
        }
        else {
            cbEdge->setCurrentIndex(cbEdge->findText(currentR->edge->name));
        }
        int index = cbE1Label->findText(currentR->e1Label);
        if(index == -1) {
            cbE1Label->setEditText(currentR->e1Label);
        }
        else {
            cbE1Label->setCurrentIndex(index);
        }
        index = cbE2Label->findText(currentR->e2Label);
        if(index == -1) {
            cbE2Label->setEditText(currentR->e2Label);
        }
        else {
            cbE2Label->setCurrentIndex(index);
        }
        index = cbEdgeLabel->findText(currentR->edgeLabel);
        if(index == -1) {
            cbEdgeLabel->setEditText(currentR->edgeLabel);
        }
        else {
            cbEdgeLabel->setCurrentIndex(index);
        }
    }

    connect_Signals();
}

void RelationsView::btnAdd_clicked() {
    bool ok;
    QString relationName = QInputDialog::getText(this,"Relation Name","Please insert a relation Name", QLineEdit::Normal,
                                            QString(), &ok);
    if(!ok || relationName.isEmpty()) {
        return;
    }

    for(int i=0; i< vector.count(); i++) {
        const Relation* rel = vector.at(i);
        if(rel->name == relationName) {
            QMessageBox::warning(this, "Warning", "This relation Name already exists!");
            return;
        }
    }

    disconnect_Signals();

    cbName->addItem(relationName);
    cbName->setCurrentIndex(cbName->findText(relationName));
    cbE1->setCurrentIndex(0);
    cbE2->setCurrentIndex(0);
    cbE1Label->setCurrentIndex(0);
    cbE2Label->setCurrentIndex(0);
    cbEdge->setCurrentIndex(0);
    cbEdgeLabel->setCurrentIndex(0);

    Relation* relation = new Relation(relationName,
                                      formula->map.value(cbE1->currentText()),
                                      cbE1Label->currentText(),
                                      formula->map.value(cbE2->currentText()),
                                      cbE2Label->currentText(),
                                      NULL,
                                      cbEdgeLabel->currentText());
    vector.append(relation);
    currentR = relation;

    connect_Signals();
    *(isDirty) = true;
}

void RelationsView::btnRemove_clicked() {
    if(vector.isEmpty()) {
        return;
    }

    QString relationRemoved = cbName->currentText();
    for(int i=0; i<vector.count(); i++) {
        const Relation * relation = vector.at(i);
        if(relation->name == relationRemoved) {
                delete (vector.at(i));
                vector.remove(i);
                break;
        }
    }

    cbName->removeItem(cbName->currentIndex());

    if(vector.isEmpty()) {
        disconnect_Signals();
        cbE1->setCurrentIndex(0);
        cbE2->setCurrentIndex(0);
        cbE1Label->setCurrentIndex(0);
        cbE2Label->setCurrentIndex(0);
        cbEdge->setCurrentIndex(0);
        cbEdgeLabel->setCurrentIndex(0);
        currentR = NULL;
        connect_Signals();
    }
    *(isDirty) = true;
}

void RelationsView::cbName_changed(QString name) {
    if(name.isEmpty() || currentR == NULL) {
        return;
    }

    for(int i=0; i< vector.count(); i++) {
        if(vector.at(i)->name == name) {
            currentR = vector[i];
            break;
        }
    }

    disconnect_Signals();
    int index = cbE1->findText(currentR->entity1->name);
    cbE1->setCurrentIndex(index);

    index = cbE2->findText(currentR->entity2->name);
    cbE2->setCurrentIndex(index);

    if(currentR->edge != NULL) {
        index = cbEdge->findText(currentR->edge->name);
        cbEdge->setCurrentIndex(index);
    }
    else {
        cbEdge->setCurrentIndex(0);
    }

    index = cbE1Label->findText(currentR->e1Label);
    if(index != -1) {
        cbE1Label->setCurrentIndex(index);
    }
    else {
        cbE1Label->setEditText(currentR->e1Label);
    }

    index = cbE2Label->findText(currentR->e2Label);
    if(index != -1) {
        cbE2Label->setCurrentIndex(index);
    }
    else {
        cbE2Label->setEditText(currentR->e2Label);
    }

    index = cbEdgeLabel->findText(currentR->edgeLabel);
    if(index != -1) {
        cbEdgeLabel->setCurrentIndex(index);
    }
    else {
        cbEdgeLabel->setEditText(currentR->edgeLabel);
    }
    connect_Signals();
}

void RelationsView::cbEdge_changed(QString edge) {
    if(edge.isEmpty() || currentR == NULL) {
        return;
    }

    if(edge.isEmpty()) {
        currentR->edge = NULL;
    }
    else {
        currentR->edge = formula->map.value(edge);
    }
}

void RelationsView::cbE1_changed(QString entity) {
    if(entity.isEmpty() || currentR == NULL) {
        return;
    }

    currentR->entity1 = formula->map.value(entity);
    *(isDirty) = true;
}

void RelationsView::cbE2_changed(QString entity) {
    if(entity.isEmpty() || currentR == NULL) {
        return;
    }

    currentR->entity2 = formula->map.value(entity);
    *(isDirty) = true;
}

void RelationsView::cbEdgeLabel_changed(QString edgeLabel) {
    if(edgeLabel.isEmpty() || currentR == NULL) {
        return;
    }

    currentR->edgeLabel = edgeLabel;
    *(isDirty) = true;
}

void RelationsView::cbE1Label_changed(QString e1Label) {
    if(e1Label.isEmpty() || currentR == NULL) {
        return;
    }

    currentR->e1Label = e1Label;
    *(isDirty) = true;
}

void RelationsView::cbE2Label_changed(QString e2Label) {
    if(e2Label.isEmpty() || currentR == NULL) {
        return;
    }

    currentR->e2Label = e2Label;
    *(isDirty) = true;
}

void RelationsView::disconnect_Signals() {
    disconnect(btnAdd, SIGNAL(clicked()), this, SLOT(btnAdd_clicked()));
    disconnect(btnRemove, SIGNAL(clicked()), this, SLOT(btnRemove_clicked()));
    disconnect(cbName, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbName_changed(QString)));
    disconnect(cbEdge, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbEdge_changed(QString)));
    disconnect(cbE1, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbE1_changed(QString)));
    disconnect(cbE2, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbE2_changed(QString)));
    disconnect(cbEdgeLabel, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbEdgeLabel_changed(QString)));
    disconnect(cbEdgeLabel, SIGNAL(editTextChanged(QString)), this, SLOT(cbEdgeLabel_changed(QString)));
    disconnect(cbE1Label, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbE1Label_changed(QString)));
    disconnect(cbE2Label, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbE2Label_changed(QString)));
}

void RelationsView::connect_Signals() {
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(btnAdd_clicked()));
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(btnRemove_clicked()));
    connect(cbName, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbName_changed(QString)));
    connect(cbEdge, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbEdge_changed(QString)));
    connect(cbE1, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbE1_changed(QString)));
    connect(cbE2, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbE2_changed(QString)));
    connect(cbEdgeLabel, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbEdgeLabel_changed(QString)));
    connect(cbEdgeLabel, SIGNAL(editTextChanged(QString)), this, SLOT(cbEdgeLabel_changed(QString)));
    connect(cbE1Label, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbE1Label_changed(QString)));
    connect(cbE2Label, SIGNAL(currentIndexChanged(QString)), this, SLOT(cbE2Label_changed(QString)));
}

void RelationsView::closeEvent(QCloseEvent *event) {

    if(*(isDirty)) {
        QMessageBox msgBox;
         msgBox.setText("The data has been modified.");
         msgBox.setInformativeText("Do you want to save your changes?");
         msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
         msgBox.setDefaultButton(QMessageBox::Save);
         int ret = msgBox.exec();

         switch (ret) {
         case QMessageBox::Save:
             /** Update formula relations **/
             for(int i=0; i<formula->relationVector.count(); i++) {
                 delete formula->relationVector.at(i);
             }
             formula->relationVector.clear();

             for(int i=0; i< vector.count(); i++) {
                 formula->relationVector.append(vector[i]);
             }
             vector.clear();
             break;
         case QMessageBox::Discard:
             break;
         default:
             break;
         }
     }
}

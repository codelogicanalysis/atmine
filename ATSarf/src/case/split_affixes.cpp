#include <QComboBox>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QHeaderView>
#include <QStringList>
#include <QPushButton>
#include <QGridLayout>
#include <QComboBox>
#include <QTableWidget>
#include <QTextBrowser>
#include <QInputDialog>
#include "Retrieve_Template.h"
#include "Search_by_item.h"
#include "Search_Compatibility.h"
#include "database_info_block.h"
#include "sql_queries.h"
#include "logger.h"
#include <iostream>
#include "split_affixes.h"
#include "test.h"
#include "Search_by_category.h"


SplitDialog::SplitDialog(): QDialog() {
    errors = new QTextBrowser(this);
    errors->resize(errors->width(), 50);
    errors_text = new QString();
    split = new QPushButton("&Split", this);
    exportLists = new QPushButton("&Export", this);
    reverse = new QPushButton("&Reverse Check", this);
    specialize = new QPushButton("S&pecialize", this);
    renameCat = new QPushButton("&Differentiate Catgeories", this);
    removeStale = new QPushButton("Remove Stale Items and Categories", this);
    affixType = new QComboBox(this);
    affixType->addItem("Prefix", PREFIX);
    affixType->addItem("Suffix", SUFFIX);
    originalAffixList = new QTableWidget(0, 7, this);
    compatRulesList = new QTableWidget(0, 3, this);
    grid = new QGridLayout(this);
    grid->addWidget(split, 0, 0);
    grid->addWidget(affixType, 0, 1);
    grid->addWidget(exportLists, 0, 2);
    grid->addWidget(reverse, 0, 3);
    grid->addWidget(specialize, 0, 4);
    grid->addWidget(renameCat, 0, 5);
    grid->addWidget(removeStale, 0, 6);
    grid->addWidget(originalAffixList, 1, 0, 1, 7);
    grid->addWidget(compatRulesList, 2, 0, 1, 7);
    grid->addWidget(errors, 3, 0, 1, 7);
    connect(split, SIGNAL(clicked()), this, SLOT(split_clicked()));
    connect(exportLists, SIGNAL(clicked()), this, SLOT(export_clicked()));
    connect(affixType, SIGNAL(currentIndexChanged(int)), this, SLOT(affixType_currentIndexChanged(int)));
    connect(reverse, SIGNAL(clicked()), this, SLOT(reverse_clicked()));
    connect(specialize, SIGNAL(clicked()), this, SLOT(specialize_clicked()));
    connect(renameCat, SIGNAL(clicked()), this, SLOT(specializeAllDuplicateEntries()));
    connect(removeStale, SIGNAL(clicked()), this, SLOT(removeStaleCategoriesAndAffixes()));
    theSarf->displayed_error.setString(errors_text);
    loadAffixList();
    loadCompatibilityList();
    this->resize(900, 700);
    errors->setText(*errors_text);
    source_id = -1;
}

void SplitDialog::loadAffixList() {
    QString table = interpret_type((item_types)affixType->itemData(affixType->currentIndex()).toInt());
    Columns list;
    originalAffixList->clear();
    QStringList v;
    v << table + " ID" << "Affix" << "Category" << "Raw Data" << "POS" << "Description" << "Reverse";
    originalAffixList->verticalHeader()->setHidden(true);
    originalAffixList->setHorizontalHeaderLabels(v);
    originalAffixList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    originalAffixList->setSelectionBehavior(QAbstractItemView::SelectRows);
    originalAffixList->setSelectionMode(QAbstractItemView::SingleSelection);
    list.append(table + "_id");
    list.append("category_id");
    list.append("raw_data");
    list.append("POS");
    list.append("description_id");
    int size = 0;
    cat_empty = -1;

    for (int i = 0; i < 2; i++) {
        Retrieve_Template s(table + "_category", list, tr("reverse_description=%1 ORDER BY ").arg(i) + table + "_id ASC");
        int row = size;
        size += s.size();
        originalAffixList->setRowCount(size);

        while (s.retrieve()) {
            long id = s.get(0).toLongLong();
            QString affix = getAffix(id);
            QString category = database_info.comp_rules->getCategoryName(s.get(1).toLongLong());
            QString raw_data = s.get(2).toString();
            QString pos = s.get(3).toString();
            originalAffixList->setItem(row, 0, new QTableWidgetItem(tr("%1").arg(id)));
            originalAffixList->setItem(row, 1, new QTableWidgetItem(affix));
            originalAffixList->setItem(row, 2, new QTableWidgetItem(category));
            originalAffixList->setItem(row, 3, new QTableWidgetItem(raw_data));
            originalAffixList->setItem(row, 4, new QTableWidgetItem(pos));
            long description_id = s.get(4).toLongLong();
            QString desc;

            if (description_id > 0 && description_id < database_info.descriptions->size()) {
                desc = database_info.descriptions->at(description_id);
            } else {
                desc = QString::null;
            }

            originalAffixList->setItem(row, 5, new QTableWidgetItem(desc));
            originalAffixList->setItem(row, 6, new QTableWidgetItem(tr("%1").arg(i)));

            if (affix.isEmpty() && raw_data.isEmpty() && pos.isEmpty() && desc.isEmpty()) {
                cat_empty = database_info.comp_rules->getCategoryID(category);
            }

            row++;
        }
    }

    assert(size == 0 || cat_empty != -1);
    findDuplicates();
}

void SplitDialog::loadCompatibilityList() {
    QStringList v;
    QString table = interpret_type((item_types)affixType->itemData(affixType->currentIndex()).toInt());
    v << table + " Category 1" << table + " Category 2" << "Resulting Category";
    compatRulesList->setHorizontalHeaderLabels(v);
    compatRulesList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    compatRulesList->setSelectionBehavior(QAbstractItemView::SelectRows);
    compatRulesList->setSelectionMode(QAbstractItemView::NoSelection);
    compatRulesList->setRowCount(0);
}

void SplitDialog::findDuplicates() {
    int rowCount = originalAffixList->rowCount();

    for (int i = 0; i < rowCount; i++) {
        //long affix_id1=originalAffixList->item(i,0)->text().toLongLong();
        QString affix1 = originalAffixList->item(i, 1)->text();
        QString category1 = originalAffixList->item(i, 2)->text();
        QString raw_data1 = originalAffixList->item(i, 3)->text();
        QString pos1 = originalAffixList->item(i, 4)->text();
        QString description1 = originalAffixList->item(i, 5)->text();

        for (int j = i + 1; j < rowCount; j++) {
            //long affix_id2=originalAffixList->item(j,0)->text().toLongLong();
            QString affix2 = originalAffixList->item(j, 1)->text();
            QString category2 = originalAffixList->item(j, 2)->text();
            QString raw_data2 = originalAffixList->item(j, 3)->text();
            QString pos2 = originalAffixList->item(j, 4)->text();
            QString description2 = originalAffixList->item(j, 5)->text();

            if (affix2 == affix1 &&
                raw_data2 == raw_data1 &&
                pos2 == pos1 &&
                description2 == description1
               ) {
                _warning << "(" << affix2 << "," << raw_data2 << "," << description2 << "," << pos2 << ") found with 2 categories: " <<
                         category1 << " & " << category2 << "\n";
            }
        }
    }

    errors->setText(*errors_text);
}

void SplitDialog::makeNonAcceptStatesAccept() {
    item_types type = (item_types)affixType->itemData(affixType->currentIndex()).toInt();
    rules rule = (type == PREFIX ? AA : CC);
    Retrieve_Template t("compatibility_rules", "category_id2", "resulting_category",
                        tr("type=%1 AND category_id1=%2").arg(rule).arg(cat_empty));

    while (t.retrieve()) {
        long category_id = t.get(0).toLongLong();
        long resulting_category = t.get(1).toLongLong();

        if (!isAcceptState(type, resulting_category)) {
            continue;
        }

        Search_by_category c(resulting_category);

        if (c.size() > 0) {
            continue;
        }

        Retrieve_Template t2("compatibility_rules", "category_id2",
                             tr("type=%1 AND category_id1=%2 AND resulting_category=%3"/*+" AND category_id2<>%4"*/).arg(rule).arg(cat_empty).arg(
                                 resulting_category)/*.arg(category_id)*/);

        if (t2.size() > 1) {
            bool stop = false;

            while (t2.retrieve()) {
                long cat = t2.get(0).toLongLong();
                Retrieve_Template t3("compatibility_rules", "category_id2",
                                     tr("type=%1 AND (category_id1=%2 OR (category_id2=%2 AND category_id1<>%3))").arg(rule).arg(cat).arg(cat_empty));

                if (t3.size() > 0) {
                    stop = true;
                    break;
                }
            }

            if (stop) {
                continue;
            }
        }

        QString cat = database_info.comp_rules->getCategoryName(category_id);
        QString catres = database_info.comp_rules->getCategoryName(resulting_category);
        _warning << QString("Set category %1 back into %2 \n").arg(cat).arg(catres);
        QSqlQuery query(theSarf->db);
        QString stmt = QString(tr("UPDATE %1_category ") +
                               "SET category_id=%3 " +
                               "WHERE category_id=%2")
                       .arg(interpret_type(type)).arg(category_id).arg(resulting_category);
        execute_query(stmt, query);
        //assert(query.numRowsAffected()>0);
        stmt = QString(tr("DELETE  ") +
                       "FROM  compatibility_rules " +
                       "WHERE type=%1 AND category_id1=%2 AND category_id2=%3")
               .arg((int)rule).arg(cat_empty).arg(category_id);
        execute_query(stmt, query);
        //assert(query.numRowsAffected()>0);
        stmt = QString(tr("UPDATE compatibility_rules ") +
                       "SET category_id1=%3 " +
                       "WHERE type=%1 AND category_id1=%2")
               .arg((int)rule).arg(category_id).arg(resulting_category);
        execute_query(stmt, query);
        stmt = QString(tr("UPDATE compatibility_rules ") +
                       "SET category_id2=%3 " +
                       "WHERE type=%1 AND category_id2=%2")
               .arg((int)rule).arg(category_id).arg(resulting_category);
        execute_query(stmt, query);
        stmt = QString(tr("UPDATE compatibility_rules ") +
                       "SET resulting_category=%3 " +
                       "WHERE type=%1 AND resulting_category=%2")
               .arg((int)rule).arg(category_id).arg(resulting_category);
        execute_query(stmt, query);
        //assert(query.numRowsAffected()>0);
    }

    loadAffixList();
    errors->setText(*errors_text);
}

void SplitDialog::removeStaleCategoriesAndAffixes() {
    item_types type = (item_types)affixType->itemData(affixType->currentIndex()).toInt();
    rules rule = (type == PREFIX ? AA : CC);
    Retrieve_Template t("compatibility_rules", "category_id1", "category_id2", "resulting_category",
                        tr("type=%1").arg(rule));

    while (t.retrieve()) {
        long category_id1 = t.get(0).toLongLong();
        long category_id2 = t.get(1).toLongLong();
        long resulting_category = t.get(2).toLongLong();
        Search_Compatibility s(rule, resulting_category);
        Search_by_category s2(category_id2);

        if ((s.size() == 0 && !isAcceptState(type, resulting_category)) || s2.size() == 0) {
            QString cat1 = database_info.comp_rules->getCategoryName(category_id1);
            QString cat2 = database_info.comp_rules->getCategoryName(category_id2);
            QString catr = database_info.comp_rules->getCategoryName(resulting_category);
            _warning << "Removing Rule (" << cat1 << "," << cat2 << "," << catr << ")\n";
            QString stmt = QString(tr("DELETE  ") +
                                   "FROM  compatibility_rules " +
                                   "WHERE type=%1 AND (category_id1=%2 AND category_id2=%3)")
                           .arg((int)rule).arg(category_id1).arg(category_id2);
            QSqlQuery query(theSarf->db);
            execute_query(stmt, query);
            assert(query.numRowsAffected() == 1);
        }
    }

    int rowCount = originalAffixList->rowCount();

    for (int i = 0; i < rowCount; i++) {
        long affix_id = originalAffixList->item(i, 0)->text().toLongLong();
        //QString affix=originalAffixList->item(i,1)->text();
        QString category = originalAffixList->item(i, 2)->text();
        QString raw_data = originalAffixList->item(i, 3)->text();
        QString pos = originalAffixList->item(i, 4)->text();
        QString description = originalAffixList->item(i, 5)->text();
        long cat_id = database_info.comp_rules->getCategoryID(category);

        if (!isAcceptState(type, cat_id)) {
            Search_Compatibility s(rule, cat_id);
            Search_Compatibility s2(rule, cat_id, false);

            if (s.size() == 0 && s2.size() == 0) {
                long description_id = database_info.descriptions->indexOf(description);
                remove_item(type, affix_id, raw_data, cat_id, description_id, pos);
            }
        }
    }

    loadAffixList();
    errors->setText(*errors_text);
}

void SplitDialog::mergeSimilarCategories(bool first) {
    QString mainField, secondaryField;

    if (first) {
        mainField = "category_id1";
        secondaryField = "category_id2";
    } else {
        mainField = "category_id2";
        secondaryField = "category_id1";
    }

    item_types type = (item_types)affixType->itemData(affixType->currentIndex()).toInt();
    rules rule = (type == PREFIX ? AA : CC);
    typedef QPair<long, long> CatPair;
    typedef QSet<CatPair> CatSet;
    CatSet dontAskSet;
    Retrieve_Template t("compatibility_rules", "category_id1", "category_id2", "resulting_category",
                        tr("type=%1").arg(rule));

    while (t.retrieve()) {
        long category_id1_1 = t.get(0).toLongLong();
        long category_id2 = t.get(1).toLongLong();
        long resulting_category = t.get(2).toLongLong();
        Retrieve_Template t2("compatibility_rules", mainField,
                             tr("type=%1 AND %5=%2 AND resulting_category=%3 AND %6<>%4").arg(rule).arg(category_id2).arg(resulting_category).arg(
                                 category_id1_1).arg(secondaryField).arg(mainField));

        while (t2.retrieve()) {
            long category_id1_2 = t2.get(0).toLongLong();
            bool stop = false;

            for (int i = 0; i < 2; i++) {
                bool first = (i == 0); //so that to do one check for category1 and other for category2

                for (int j = 0; j < 2; j++) {
                    long ca1 = (j == 0 ? category_id1_1 : category_id1_2);
                    long ca2 = (j == 0 ? category_id1_2 : category_id1_1);
                    Search_Compatibility c1(rule, ca1, first);
                    long cat2_1, cat2_2, result1, result2;

                    while (c1.retrieve(cat2_1, result1)) {
                        Search_Compatibility c2(rule, ca2, first);

                        if (c1.size() != c2.size()) {
                            stop = true;
                            break;
                        }

                        bool found = false;

                        while (c2.retrieve(cat2_2, result2)) {
                            if (cat2_1 == cat2_2 && result1 == result2) {
                                found = true;
                                break;
                            }
                        }

                        if (!found) {
                            stop = true;
                            break;
                        }
                    }

                    if (stop) {
                        break;
                    }
                }

                if (stop) {
                    break;
                }
            }

            if (stop) {
                continue;
            }

            bool acc1 = isAcceptState(type, category_id1_1);
            bool acc2 = isAcceptState(type, category_id1_2);

            if (acc1 != acc2) {
                continue;
            }

            CatPair p(category_id1_1, category_id1_2);

            if (dontAskSet.contains(p)) {
                continue;
            }

            CatPair p2(p.second, p.first);
            dontAskSet.insert(p);
            dontAskSet.insert(p2);
            QMessageBox msgBox;
            QString cat1 = database_info.comp_rules->getCategoryName(category_id1_1),
                    cat2 = database_info.comp_rules->getCategoryName(category_id1_2);
            msgBox.setText(tr("About to merge ") + cat2 + "," + cat1 + "\n");
            msgBox.setInformativeText("Do you want to add it?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            int ret = msgBox.exec();

            if (ret == QMessageBox::Yes) {
                _warning << QString("Merged the 2 categories: %1, %2 into %1\n").arg(cat1).arg(cat2);
                QSqlQuery query(theSarf->db);
                QString stmt = QString(tr("UPDATE %1_category ") +
                                       "SET category_id=%2 " +
                                       "WHERE category_id=%3")
                               .arg(interpret_type(type)).arg(category_id1_1).arg(category_id1_2);
                execute_query(stmt, query);
                //assert(query.numRowsAffected()>0);
                stmt = QString(tr("DELETE  ") +
                               "FROM  compatibility_rules " +
                               "WHERE type=%1 AND (category_id1=%2 OR category_id2=%2)")
                       .arg((int)rule).arg(category_id1_2);
                execute_query(stmt, query);
                //assert(query.numRowsAffected()>0);
                return mergeSimilarCategories(first);
            }
        }
    }

    loadAffixList();
    errors->setText(*errors_text);
}

void SplitDialog::removeDummyRulesForConsistencyIfNotNeeded() {
    item_types type = (item_types)affixType->itemData(affixType->currentIndex()).toInt();
    rules rule = (type == PREFIX ? AA : CC);
    QSqlQuery query(theSarf->db);
    QString stmt = QString(tr("SELECT t1.category_id2, t2.category_id2  ") +
                           "FROM  compatibility_rules t1, compatibility_rules t2 " +
                           "WHERE t1.type=%1 AND t2.type=%1 AND t1.category_id1=%2 AND t2.category_id1=%2 AND t1.resulting_category=t2.resulting_category AND t1.category_id2<t2.category_id2")
                   .arg((int)rule).arg(cat_empty);
    execute_query(stmt, query);

    while (query.next()) {
        long id1 = query.value(0).toULongLong();
        long id2 = query.value(1).toULongLong();
        /*if (id1!=13 && id2!=66)
          continue;*/
        long idMain = -1;
        long idOld = -1;

        if (isAcceptState(type, id1)) {
            assert(!isAcceptState(type, id2));
            idMain = id1;
            idOld = id2;
        } else if (isAcceptState(type, id2)) {
            idMain = id2;
            idOld = id1;
        }

        QString catMain = database_info.comp_rules->getCategoryName(idMain),
                catOld = database_info.comp_rules->getCategoryName(idOld);
        _warning << QString("Revert the 2 categories: %1, %2 into %1\n").arg(catMain).arg(catOld);
        QSqlQuery query(theSarf->db);
        QString stmt = QString(tr("UPDATE %1_category ") +
                               "SET category_id=%2 " +
                               "WHERE category_id=%3")
                       .arg(interpret_type(type)).arg(idMain).arg(idOld);
        execute_query(stmt, query);
        //assert(query.numRowsAffected()>0);
        stmt = QString(tr("DELETE  ") +
                       "FROM  compatibility_rules " +
                       "WHERE type=%1 AND (category_id1=%2 AND category_id2=%3)")
               .arg((int)rule).arg(cat_empty).arg(idOld);
        execute_query(stmt, query);
        stmt = QString(tr("UPDATE compatibility_rules ") +
                       "SET category_id1=%2 " +
                       "WHERE type=%1 AND category_id1=%3")
               .arg((int)rule).arg(idMain).arg(idOld);
        execute_query(stmt, query);
        stmt = QString(tr("UPDATE compatibility_rules ") +
                       "SET category_id2=%2 " +
                       "WHERE type=%1 AND category_id2=%3")
               .arg((int)rule).arg(idMain).arg(idOld);
        execute_query(stmt, query);
        stmt = QString(tr("UPDATE compatibility_rules ") +
                       "SET resulting_category=%2 " +
                       "WHERE type=%1 AND resulting_category=%3")
               .arg((int)rule).arg(idMain).arg(idOld);
        execute_query(stmt, query);
    }
}

void SplitDialog::split_clicked() {
    QList<QTableWidgetSelectionRange>  selection = originalAffixList->selectedRanges();
    item_types t = (item_types)affixType->itemData(affixType->currentIndex()).toInt();

    if (selection.size() == 1 && selection[0].rowCount() == 1) {
        if (source_id < 0) {
            source_id = insert_source("Manual Work by Jad", "Splitting Affixes by an automatic process", "Jad Makhlouta");
        }

        //select the items to be splitted
        originalAffixList->setSelectionMode(QAbstractItemView::MultiSelection);
        int rowIndex = selection[0].topRow();
        //long affix_id=originalAffixList->item(rowIndex,0)->text().toLongLong();
        QString affix = originalAffixList->item(rowIndex, 1)->text();
        QString category = originalAffixList->item(rowIndex, 2)->text();
        QString raw_data = originalAffixList->item(rowIndex, 3)->text();
        QString pos = originalAffixList->item(rowIndex, 4)->text();
        QString description = originalAffixList->item(rowIndex, 5)->text();
        originalAffixList->clearSelection();
        int rowCount = originalAffixList->rowCount();

        for (int i = 0; i < rowCount; i++) {
            long affix_id1 = originalAffixList->item(i, 0)->text().toLongLong();
            QString affix1 = originalAffixList->item(i, 1)->text();
            QString category1 = originalAffixList->item(i, 2)->text();
            QString raw_data1 = originalAffixList->item(i, 3)->text();
            QString pos1 = originalAffixList->item(i, 4)->text();
            QString description1 = originalAffixList->item(i, 5)->text();
            bool canBeEmpty = description.startsWith('[') && description.endsWith(']') && pos.count('+') == 1;

            if (affix1.startsWith(affix) &&
                raw_data1.startsWith(raw_data) &&
                pos1.startsWith(pos) &&
                (description1.startsWith(description) || description1.endsWith(description) ||
                 canBeEmpty)) {
                bool reverse_description = true;

                if (description1.startsWith(description)) {
                    reverse_description = false;
                }

                originalAffixList->selectRow(i);
                //split the item selected
                QString affix2 = affix1.mid(affix.size());
                QString raw_data2 = raw_data1.mid(raw_data.size());
                QString pos2 = pos1.mid(pos.size());
                int size_of_gloss_seperator = (t == PREFIX ? 3 : 1); //' + ' or ' '
                QString description2;

                if (canBeEmpty && !description1.startsWith(description)) {
                    description2 = description1;
                } else if (!reverse_description) {
                    description2 = description1.mid((description.size() == 0 ? -1 : description.size()) +
                                                    size_of_gloss_seperator);    //' + ' or ' '
                } else {
                    description2 = description1.mid(0, description1.size() - (description.size() == 0 ? -1 : description.size()) -
                                                    size_of_gloss_seperator);    //' '
                }

                int rowIndex2 = getRow(affix2, raw_data2, pos2, description2);
                QString category2;
                bool addRule = true;

                if (rowIndex2 < 0) {
                    if (getPartialConnectedRow(affix2, raw_data2, pos2, description2, category, category1) > 0) {
                        addRule = false; //already added
                        _warning << "Added 3-way rule!\n";
                    } else {
                        if (!pos2.isEmpty()) {
                            _warning << "(" << affix2 << "," << raw_data2 << "," << description2 << "," << pos2 << ") not found.\n";
                        }

                        continue;
                    }
                } else {
                    bool reverse_description2 = (bool)originalAffixList->item(i, 6)->text().toInt();

                    if (reverse_description2 && ! reverse_description) {
                        _error << "Inconsistent Reverse Description.\n";
                        continue;
                    }

                    if (reverse_description) {
                        originalAffixList->setItem(rowIndex2, 6, new QTableWidgetItem(QString("1")));
                        QString category2 = originalAffixList->item(rowIndex2, 2)->text();
                        _warning << "updating reverse_description...\n";
                        KEEP_OLD = false;
                        insert_item(t, affix2, raw_data2, category2, source_id, new QList<long>, description2, pos2, "", "1");
                    }

                    category2 = originalAffixList->item(rowIndex2, 2)->text();
                }

                long category_id1 = database_info.comp_rules->getCategoryID(category1);
                remove_item(t, affix_id1, raw_data1, category_id1, database_info.descriptions->indexOf(description1), pos1);

                if (addRule) {
                    //in case this category has been renamed by some number in differentiate categories
                    long cat_result;

                    if ((*database_info.comp_rules)(cat_empty, category_id1, cat_result)) {
                        //category_id1=cat_result; //not needed
                        category1 = database_info.comp_rules->getCategoryName(cat_result);
                    }

                    insert_compatibility_rules((t == PREFIX ? AA : CC), category, category2, category1, source_id);
                    int compRowIndex = compatRulesList->rowCount();
                    compatRulesList->setRowCount(compRowIndex + 1);
                    compatRulesList->setItem(compRowIndex, 0, new QTableWidgetItem(category));
                    compatRulesList->setItem(compRowIndex, 1, new QTableWidgetItem(category2));
                    compatRulesList->setItem(compRowIndex, 2, new QTableWidgetItem(category1));
                }
            }
        }
    }

    loadAffixList();
    errors->setText(*errors_text);
}

void SplitDialog::reverse_clicked() {
    item_types type = (item_types)affixType->itemData(affixType->currentIndex()).toInt();
    rules rule = (type == PREFIX ? AA : CC);
    Retrieve_Template t("compatibility_rules", "category_id1", "category_id2", "resulting_category",
                        tr("type=%1").arg(rule));

    while (t.retrieve()) {
        long category_id2 = t.get(0).toLongLong();
        long category_id3 = t.get(1).toLongLong();
        long resulting_category1 = t.get(2).toLongLong();
        Search_Compatibility s(rule, resulting_category1, false);
        long category_id1, resulting_category2;

        while (s.retrieve(category_id1, resulting_category2)) {
            if (!(category_id2 == category_id1 && resulting_category1 == category_id3)) {
                assert(category_id1 != category_id2);

                if (!areCompatible(rule, category_id1, category_id2)) {
                    QMessageBox msgBox;
                    msgBox.setText(tr("About to add Rule (") + database_info.comp_rules->getCategoryName(
                                       category_id1) + "," + database_info.comp_rules->getCategoryName(category_id2) + ")\n"
                                   + "\tcategory3=" + database_info.comp_rules->getCategoryName(category_id3) + ",resulting1=" +
                                   database_info.comp_rules->getCategoryName(resulting_category1) + ",resulting2=" +
                                   database_info.comp_rules->getCategoryName(resulting_category2));
                    msgBox.setInformativeText("Do you want to add it?");
                    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    msgBox.setDefaultButton(QMessageBox::Yes);
                    int ret = msgBox.exec();

                    if (ret == QMessageBox::Yes) {
                        insert_compatibility_rules(rule, category_id1, category_id2, source_id);
                        _warning << "Added Rule (" << database_info.comp_rules->getCategoryName(category_id1) << "," <<
                                 database_info.comp_rules->getCategoryName(category_id2) << ")\n"
                                 << "\tcategory3=" << database_info.comp_rules->getCategoryName(category_id3) << ",resulting1=" <<
                                 database_info.comp_rules->getCategoryName(resulting_category1) << ",resulting2=" <<
                                 database_info.comp_rules->getCategoryName(resulting_category2) << "\n";
                    }
                }
            }
        }
    }
}

void SplitDialog::specializeAllDuplicateEntries() {
    item_types t = (item_types)affixType->itemData(affixType->currentIndex()).toInt();
    typedef QMap<QString, int> NumberMap;
    NumberMap map;
    int rowCount = originalAffixList->rowCount();

    for (int i = 0; i < rowCount; i++) {
        //long affix_id1=originalAffixList->item(i,0)->text().toLongLong();
        QString category = originalAffixList->item(i, 2)->text();
        long category_id = getID("category", category, QString("type=%1").arg((int)t));

        if (cat_empty != category_id && isAcceptState(t, category_id)) {
            NumberMap::iterator itr = map.find(category);
            QString category_changed;

            if (itr != map.end()) {
                int i = ++(itr.value());
                category_changed = (category + "_%1").arg(i);
            } else {
                map[category] = 0;
                category_changed = category + "_0";
            }

            renameCategory(i, category, category_changed);
        }
    }

    system(QString("rm ").append(compatibility_rules_path).toStdString().data());
    database_info.comp_rules->buildFromFile();
    loadAffixList();
    errors->setText(*errors_text);
}

void SplitDialog::renameCategory(int row, QString category_original, QString category_specialized) {
    originalAffixList->selectRow(row);
    specializeHelper(category_original, category_specialized, "", false);
}

void SplitDialog::specializeHelper(QString category_original, QString category_specialized, QString category_left,
                                   bool modifyLeftEntries) {
    item_types t = (item_types)affixType->itemData(affixType->currentIndex()).toInt();
    rules rule = (t == PREFIX ? AA : CC);
    long cat_org_id = getID("category", category_original, QString("type=%1").arg((int)t));
    long specialized_id = insert_category(category_specialized, t, source_id, false);
    long left_id = insert_category(category_left, t, source_id, false);
    Search_Compatibility s(rule, cat_org_id, true);
    long cat_id2, cat_result;

    while (s.retrieve(cat_id2, cat_result)) {
        insert_compatibility_rules(rule, specialized_id, cat_id2, cat_result, source_id);
        insert_compatibility_rules(rule, left_id, cat_id2, cat_result, source_id);
    }

    Search_Compatibility s2(rule, cat_org_id, false);

    while (s2.retrieve(cat_id2, cat_result)) {
        insert_compatibility_rules(rule, cat_id2, specialized_id, cat_result, source_id);
        insert_compatibility_rules(rule, cat_id2, left_id, cat_result, source_id);
    }

    //second change categories of those selected
    QSqlQuery query(theSarf->db);
    QList<QTableWidgetSelectionRange>  selection = originalAffixList->selectedRanges();

    for (int i = 0; i < selection.size(); i++) {
        int top = selection[i].topRow();

        for (int j = 0; j < selection[i].rowCount(); j++) {
            int rowIndex = top + j;
            long affix_id = originalAffixList->item(rowIndex, 0)->text().toLongLong();
            QString raw_data = originalAffixList->item(rowIndex, 3)->text();
            QString pos = originalAffixList->item(rowIndex, 4)->text();
            QString description = originalAffixList->item(rowIndex, 5)->text();
            QString stmt = QString(tr("UPDATE %1_category ") +
                                   "SET category_id=%2 " +
                                   "WHERE %1_id=%3 AND category_id=%4 AND raw_data=\"%5\" AND POS=\"%6\" AND " +
                                   "description_id IN (SELECT id FROM description WHERE name =\"%7\" AND type=%8)")
                           .arg(interpret_type(t)).arg(specialized_id).arg(affix_id).arg(cat_org_id).arg(raw_data)
                           .arg(pos).arg(description).arg((int)t);
            execute_query(stmt, query);
            int num = query.numRowsAffected();
            assert(num == 1 || num == -1);
        }
    }

    //third: add 2 dummy rules to keep consistency with AB,AC,BC
    insert_compatibility_rules(rule, cat_empty, specialized_id, cat_org_id, source_id);

    if (modifyLeftEntries) {
        //forth: change category of those not selected
        execute_query(QString(tr("UPDATE %1_category ") +
                              "SET category_id=%2 " +
                              "WHERE category_id=%3")
                      .arg(interpret_type(t)).arg(left_id).arg(cat_org_id), query);

        if (query.numRowsAffected() == 0) {
            _warning << "Operation performed performed just renaming because there are no left entries of this query\n";
        }

        if (left_id != cat_org_id) {
            insert_compatibility_rules(rule, cat_empty, left_id, cat_org_id, source_id);
        }

        //fifth: remove all rules AA or CC having the old_category if (old not used again)
        if (category_original != category_left) {
            QString stmt = QString(tr("DELETE  ") +
                                   "FROM  compatibility_rules " +
                                   "WHERE type=%1 AND (category_id1=%2 OR category_id2=%2)")
                           .arg((int)rule).arg(cat_org_id);
            execute_query(stmt, query);
        }
    }
}

void SplitDialog::specialize_clicked() {
    if (originalAffixList->selectionMode() == QAbstractItemView::MultiSelection) {
        QList<QTableWidgetSelectionRange>  selection = originalAffixList->selectedRanges();

        if (selection.size() >= 1) {
            QString category_original = "";

            for (int i = 0; i < selection.size(); i++) {
                int top = selection[i].topRow();

                for (int j = 0; j < selection[i].rowCount(); j++) {
                    int rowIndex = top + j;

                    if (category_original == "") {
                        category_original = originalAffixList->item(rowIndex, 2)->text();
                    } else if (category_original != originalAffixList->item(rowIndex, 2)->text()) {
                        _error << "Operation Failed: because not all selected rows have same category\n";
                        return;
                    }
                }
            }

            //first add category of new Names for the specialized category and category of those left and duplicate previous rules to them
            bool ok;
            QString category_specialized = category_original;

            while (category_specialized == category_original) {
                category_specialized = QInputDialog::getText(this, tr("Name for Scpecialized Category"),
                                                             tr("New Name: (must be different from original)"), QLineEdit::Normal,
                                                             category_original, &ok);
            }

            QString category_left = QInputDialog::getText(this, tr("Name of Category for entries left"),
                                                          tr("New Name:"), QLineEdit::Normal,
                                                          category_original, &ok);

            if (ok && !category_specialized.isEmpty() && !category_left.isEmpty()) {
                specializeHelper(category_original, category_specialized, category_left);
                system(QString("rm ").append(compatibility_rules_path).toStdString().data());
                database_info.comp_rules->buildFromFile();
                loadAffixList();
            }
        }
    } else {
        originalAffixList->setSelectionMode(QAbstractItemView::MultiSelection);
    }
}

void SplitDialog::export_clicked() {
    item_types t = (item_types)affixType->itemData(affixType->currentIndex()).toInt();
    QFile f(tr("%1List.txt").arg(interpret_type(t)));
    assert(f.open(QIODevice::WriteOnly));
    QTextStream file(&f);
    file.setCodec("utf-8");

    for (int i = 0; i < originalAffixList->rowCount(); i++) {
        //long affix_id=originalAffixList->item(i,0)->text().toLongLong();
        QString affix = originalAffixList->item(i, 1)->text();
        QString category = originalAffixList->item(i, 2)->text();
        QString raw_data = originalAffixList->item(i, 3)->text();
        QString pos = originalAffixList->item(i, 4)->text();
        QString description = originalAffixList->item(i, 5)->text();
        QString reverse_description = originalAffixList->item(i, 6)->text();
        file << affix << "\t" << raw_data << "\t" << category << "\t" << description << "\t" << pos << "\t" <<
             (reverse_description == "0" ? "" : "1") << "\n";
    }

    f.close();
    rules rule = (t == PREFIX ? AA : CC);
    QFile f2(tr("%1.txt").arg(interpret_type(rule)));
    assert(f2.open(QIODevice::WriteOnly));
    QTextStream file2(&f2);
    file2.setCodec("utf-8");
    Retrieve_Template r("compatibility_rules", "category_id1", "category_id2", "resulting_category", "inflections",
                        tr("type=%1").arg(rule));

    while (r.retrieve()) {
        QString inflections = r.get(3).toString();

        if (!inflections.isEmpty() && !inflections.contains("\\\\")) {
            inflections.replace("\\", "\\\\");
        }

        file2 << database_info.comp_rules->getCategoryName(r.get(0).toLongLong()) << "\t"
              << database_info.comp_rules->getCategoryName(r.get(1).toLongLong()) << "\t"
              << database_info.comp_rules->getCategoryName(r.get(2).toLongLong()) << "\t"
              << inflections << "\n";
    }

    f2.close();
}

void SplitDialog::affixType_currentIndexChanged(int) {
    errors->clear();
    loadAffixList();
    loadCompatibilityList();
    //removeDummyRulesForConsistencyIfNotNeeded();
    mergeSimilarCategories();
    mergeSimilarCategories(false);
    //makeNonAcceptStatesAccept();
}

QString SplitDialog::getAffix(long id) {
    item_types t = (item_types)affixType->itemData(affixType->currentIndex()).toInt();
    QString table = interpret_type(t);
    Retrieve_Template s(table, "name", tr("id=%1").arg(id));

    if (s.retrieve()) {
        return s.get(0).toString();
    } else {
        return "--";
    }
}

bool SplitDialog::hasSubjectObjectInconsistency(QString descriptionDesired,
                                                QString descriptionFound) { //works for prefixes
    descriptionFound.replace("they", "them");
    descriptionFound.replace("he/", "him/");
    descriptionFound.replace("she", "her");
    descriptionFound.replace("I", "me");
    descriptionFound.replace("we", "us");
    return (descriptionDesired.startsWith(descriptionFound)
            && (descriptionDesired.endsWith("to") || descriptionDesired.endsWith("(to)")));
}

int SplitDialog::getPartialConnectedRow(QString affix, QString raw_data, QString pos, QString description,
                                        QString category_1, QString resultCat, bool partial) {
    if (pos.isEmpty()) {
        return -1;
    }

    item_types t = (item_types)affixType->itemData(affixType->currentIndex()).toInt();
    rules rule = (t == PREFIX ? AA : CC);
    int rowCount = originalAffixList->rowCount();

    for (int i = 0; i < rowCount; i++) {
        //long affix_id1=originalAffixList->item(i,0)->text().toLongLong();
        QString affix1 = originalAffixList->item(i, 1)->text();
        QString category1 = originalAffixList->item(i, 2)->text();
        QString raw_data1 = originalAffixList->item(i, 3)->text();
        QString pos1 = originalAffixList->item(i, 4)->text();
        QString description1 = originalAffixList->item(i, 5)->text();
        bool con_full = !partial && affix1 == affix &&
                        raw_data1 == raw_data &&
                        pos1 == pos;
        bool con_partial = partial && affix.startsWith(affix1) &&
                           raw_data.startsWith(raw_data1) &&
                           pos.startsWith(pos1) &&
                           description.startsWith(description1);

        if (con_full || con_partial) {
            if ((con_full && description1 == description)) {
                return i;
            }

            if (con_partial) {
                if (!pos.contains("+")) {
                    return -1;
                }

                QStringList p = pos.split("+");
                QStringList d = description.split(" + ");
                assert(p.size() == 2);
                assert(d.size() == 2);
                QString p1 = p[0];
                QString p2 = p[1];
                QString d1 = d[0];
                QString d2 = d[1];
                ItemCatRaw2AbsDescPosMapPtr map = (t == PREFIX ? database_info.map_prefix : database_info.map_suffix);
                ItemCatRaw2AbsDescPosMapItr itr = map->begin();

                for (; itr != map->end(); itr++) {
                    long itr1DescId = itr->second;
                    QString itr1Desc = database_info.descriptions->at(itr1DescId);
                    QString itr1Pos = itr->third;

                    if (p1 != itr1Pos || d1 != itr1Desc) {
                        continue;
                    }

                    long itr1Id = itr.key().first;
                    QString itr1Affix = getAffix(itr1Id);
                    long itr1CatId = itr.key().second;
                    QString itr1Cat = database_info.comp_rules->getCategoryName(itr1CatId);
                    QString itr1Raw = itr.key().third;
                    ItemCatRaw2AbsDescPosMapItr itr2 = map->begin();

                    for (; itr2 != map->end(); itr2++) {
                        long itr2DescId = itr2->second;
                        QString itr2Desc = database_info.descriptions->at(itr2DescId);
                        QString itr2Pos = itr2->third;

                        if (p2 != itr2Pos || d2 != itr2Desc) {
                            continue;
                        }

                        long itr2Id = itr2.key().first;
                        QString itr2Affix = getAffix(itr2Id);
                        long itr2CatId = itr2.key().second;
                        QString itr2Cat = database_info.comp_rules->getCategoryName(itr2CatId);
                        QString itr2Raw = itr2.key().third;

                        if (itr1Affix + itr2Affix == affix && itr1Raw + itr2Raw == raw_data) {
                            //long cat_r1;
                            //if ((*database_info.comp_rules)(itr1CatId,itr2CatId,cat_r1)) {
                            QString resCat1 = category_1 + "_" + itr1Cat;
                            insert_category(resCat1, t, 1, false); //1 is dummy
                            insert_compatibility_rules(rule, category_1, itr1Cat, resCat1, 1); //1 is dummy
                            //in case this category has been renamed by some number in differentiate categories
                            long cat_result;
                            long resultCatID = database_info.comp_rules->getCategoryID(resultCat);

                            if ((*database_info.comp_rules)(cat_empty, resultCatID, cat_result)) {
                                resultCat = database_info.comp_rules->getCategoryName(cat_result);
                            }

                            insert_compatibility_rules(rule, resCat1, itr2Cat, resultCat, 1); //1 is dummy
                            return 1;
                            //}
                        }
                    }
                }

                return -1;
            } else if (t == PREFIX &&
                       hasSubjectObjectInconsistency(description, description1)) { //prefix subject/object confusion for "for"
                QString newCat = category1 + "(object)";
                long added_affix_id = insert_item(PREFIX, affix, raw_data, newCat, source_id, NULL, description, pos);
                //TODO: solve staleness
                /*system(QString(tr("rm ")+compatibility_rules_path+" "+description_path).toStdString().data());
                  database_info.comp_rules->buildFromFile();
                  database_info.fill((ATMProgressIFC*)parentWidget());*/
                _warning << "Adding prefix (" << affix << "," << raw_data << "," << newCat << "," << description << "," << pos << "\n";
                assert(added_affix_id >= 0);
                originalAffixList->setRowCount(rowCount + 1);
                originalAffixList->setItem(rowCount, 0, new QTableWidgetItem(tr("%1").arg(added_affix_id)));
                originalAffixList->setItem(rowCount, 1, new QTableWidgetItem(affix));
                originalAffixList->setItem(rowCount, 2, new QTableWidgetItem(newCat));
                originalAffixList->setItem(rowCount, 3, new QTableWidgetItem(raw_data));
                originalAffixList->setItem(rowCount, 4, new QTableWidgetItem(pos));
                originalAffixList->setItem(rowCount, 5, new QTableWidgetItem(description));
                return rowCount;
            }
        }
    }

    return -1;
}

int SplitDialog::getRow(const QString &affix, const QString &raw_data, const QString &pos, const QString &description) {
    if (pos.isEmpty()) {
        return -1;
    }

    item_types t = (item_types)affixType->itemData(affixType->currentIndex()).toInt();
    int rowCount = originalAffixList->rowCount();

    for (int i = 0; i < rowCount; i++) {
        //long affix_id1=originalAffixList->item(i,0)->text().toLongLong();
        QString affix1 = originalAffixList->item(i, 1)->text();
        QString category1 = originalAffixList->item(i, 2)->text();
        QString raw_data1 = originalAffixList->item(i, 3)->text();
        QString pos1 = originalAffixList->item(i, 4)->text();
        QString description1 = originalAffixList->item(i, 5)->text();

        if (affix1 == affix &&
            raw_data1 == raw_data &&
            pos1 == pos) {
            if (description1 == description) {
                return i;
            } else if (t == PREFIX &&
                       hasSubjectObjectInconsistency(description, description1)) { //prefix subject/object confusion for "for"
                QString newCat = category1 + "(object)";
                long added_affix_id = insert_item(PREFIX, affix, raw_data, newCat, source_id, NULL, description, pos);
                //TODO: solve staleness
                /*system(QString(tr("rm ")+compatibility_rules_path+" "+description_path).toStdString().data());
                  database_info.comp_rules->buildFromFile();
                  database_info.fill((ATMProgressIFC*)parentWidget());*/
                _warning << "Adding prefix (" << affix << "," << raw_data << "," << newCat << "," << description << "," << pos << "\n";
                assert(added_affix_id >= 0);
                originalAffixList->setRowCount(rowCount + 1);
                originalAffixList->setItem(rowCount, 0, new QTableWidgetItem(tr("%1").arg(added_affix_id)));
                originalAffixList->setItem(rowCount, 1, new QTableWidgetItem(affix));
                originalAffixList->setItem(rowCount, 2, new QTableWidgetItem(newCat));
                originalAffixList->setItem(rowCount, 3, new QTableWidgetItem(raw_data));
                originalAffixList->setItem(rowCount, 4, new QTableWidgetItem(pos));
                originalAffixList->setItem(rowCount, 5, new QTableWidgetItem(description));
                return rowCount;
            }
        }
    }

    return -1;
}

SplitDialog::~SplitDialog() {
    delete affix;
    delete split;
    delete exportLists;
    delete reverse;
    delete affixType;
    delete errors;
    delete errors_text;
    delete originalAffixList;
    delete compatRulesList;
    delete grid;
    delete renameCat;
    delete removeStale;
}

void splitRecursiveAffixes() {
    SplitDialog *d = new SplitDialog();
    d->exec();
}


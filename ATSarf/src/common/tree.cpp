#include <QFile>
#include <QDataStream>
#include "tree.h"
#include "sql_queries.h"
#include "Search_Compatibility.h"
#include "Search_by_category.h"
#include "Search_by_item.h"
#include "text_handling.h"
#include <assert.h>
#include "database_info_block.h"
#include "Retrieve_Template.h"
#include "inflections.h"

inline QString cache_version() {
    return "RD";
}

inline bool hasCompatibleAffixes(item_types type, long cat_r_id) {
    bool hasComp = false;
    Retrieve_Template check("compatibility_rules", "COUNT(*)",
                            QString("category_id1=%1 AND type=%2").arg(cat_r_id).arg((int)(type == PREFIX ? AA : CC)));

    if (check.retrieve() && check.get(0).toInt() > 0) {
        hasComp = true;
    }

    return hasComp;
}

void tree::print_tree_helper(node *current_node, int level) {
    theSarf->out << QString().fill(' ', level * 7) << current_node->to_string(isAffix) << "\n";
    QVector<letter_node * > list = current_node->getLetterChildren();

    for (int i = 0; i < list.count(); i++) {
        print_tree_helper(list.at(i), level + 1);
    }

    QList<result_node *> *list2 = current_node->getResultChildren();

    for (int i = 0; i < list2->count(); i++) {
        print_tree_helper(list2->at(i), level + 1);
    }
}

int tree::build_helper(item_types type, long cat_id1, int size, node *current) {
    if (size <= 0) {
        return 0;
    }

    long cat_id2, cat_r_id;
    Search_Compatibility s2((type == PREFIX ? AA : CC), cat_id1);

    while (s2.retrieve(cat_id2, cat_r_id)) {
        QString inflections = s2.getInflectionRules();
        bool isAccept = isAcceptState(type, cat_r_id);

        if (isAccept ||
            hasCompatibleAffixes(type, cat_r_id)) { //dont add to tree branches that have no rules and connect to nothing else that may have rules
            Search_by_category s3(cat_id2);
            all_item_info inf;

            while (s3.retrieve(inf)) {
                QString name = getColumn(interpret_type(type), "name", inf.item_id);
                name = removeDiacritics(name);
                QString inflectedRawData = inf.raw_data;
                applyRawDataInflections(inflections, name, inflectedRawData); //name and inflectedRawData are changed
                node *next;
                    next = addElement(name, inf.item_id, cat_id2, cat_r_id, isAccept, inf.raw_data, inflectedRawData, inflections, current);
                build_helper(type, cat_r_id, size - name.length(), next);
            }
        }
    }

    return 0;
}

node *tree::addElement(QString letters, long affix_id, long category_id, long resulting_category_id, bool isAccept,
                       QString raw_data, QString inflected_raw_data, QString descriptionInflectionRule, node * current)
{
    assert(current->isLetterNode() || equal(letters, inflected_raw_data));

    if (file != NULL) {
        (*file) << letters << affix_id << category_id << resulting_category_id << isAccept
                << raw_data << inflected_raw_data << descriptionInflectionRule
                << generateNodeID(current);
    }

    //pre-condition: assumes category_id is added to the right place and results in the appropraite resulting_category
    QChar current_letter;
    //QList<letter_node *>* current_letter_children;
    letter_node *matching_letter_node = NULL;

    if (current->isLetterNode() && current != base) {
        _error << "Unexpected Error: provided node was a letter node and not a result one\n";

        if (file != NULL) {
            (*file) << generateNodeID(NULL);
        }

        return NULL;
    }

    int i;

    if (letters.count() == 0) {
        current_letter = '\0';

        if (current == base) {
            goto result;
        }
    } else {
        current_letter = letters[0];
    }

    i = 0;

    do {
        matching_letter_node = current->getLetterChild(current_letter);

        if (matching_letter_node != NULL) {
            current = matching_letter_node;
            i++;
            current_letter = letters[i];
        } else {
            break;
        }
    } while (i < letters.count());

    if (letters.count() == 0 && i == 0) {
        //add null letter
        letter_node *new_node = new letter_node('\0');
        current->addChild(new_node);
        current = new_node;
        letter_nodes++;
    }

    for (; i < letters.count(); i++) {
        //add necessary letters
        letter_node *new_node = new letter_node(letters[i]);
        current->addChild(new_node);
        current = new_node;
        letter_nodes++;
    }

result:
    int size = current->getResultChildren()->size();

    for (int i = 0; i < size; i++) { //check if this result node is already present
        result_node *old_result = current->getResultChildren()->at(i);

        if (old_result->get_previous_category_id() == category_id &&
            old_result->get_resulting_category_id() == resulting_category_id && old_result->get_affix_id() == affix_id) {
            old_result->add_raw_data(raw_data, inflected_raw_data);
            old_result->setInflectionRule(descriptionInflectionRule);

            if (file != NULL) {
                (*file) << generateNodeID(old_result);
            }

            return old_result;
        }
    }

    result_node *result = new result_node(affix_id, category_id, resulting_category_id, isAccept, raw_data,
                                          inflected_raw_data);
    result->setInflectionRule(descriptionInflectionRule);
    current->addChild(result);
    current = result;
    result_nodes++;

    if (file != NULL) {
        (*file) << generateNodeID(current);
    }

    return current;
    //post-condition: returns node of resulting category reached after addition
}
tree::tree() {
    base = new letter_node('\0');
    letter_nodes = 1;
    result_nodes = 0;
    isAffix = false;
    file = NULL;
}
tree::tree(item_types type) {
    base = new letter_node('\0');
    letter_nodes = 1;
    result_nodes = 0;
    isAffix = true;
    file = NULL;
    build_affix_tree(type);
}
bool tree::getAffixType(item_types & type) {
    if (isAffix) {
        type = this->type;
        return true;
    } else {
        return false;
    }
}

int tree::generateNodeID(node * n) {
    int curr_id;
    IDMap::iterator it = idMap.find(n);

    if (it == idMap.end()) {
        if (n != base) {
            curr_id = ++last_id;
        } else {
            curr_id = 0;    //base is number 0
        }

        idMap.insert(n, curr_id);
    } else {
        curr_id = it.value();
    }

    return curr_id;
}
node *tree::getNodeID(int num) {
    ReverseIDMap::iterator it = reverseIDMap.find(num);
    node *n;

    if (num == 0) { //base is number 0
        n = base;
    } else {
        assert(it != reverseIDMap.end());
        n = it.value();
    }

    return n;
}
void tree::setNodeID(int num, node * n) {
    ReverseIDMap::iterator it = reverseIDMap.find(num);

    if (it == reverseIDMap.end()) {
        reverseIDMap.insert(num, n);
    } else {
        assert(n == it.value());
    }
}
int tree::build_affix_tree_from_file(item_types type) {
    file = NULL;
    reset();
    isAffix = true;
    this->type = type;
    QString fileName;

    if (type == PREFIX) {
        fileName = prefix_tree_path;
    } else if (type == SUFFIX) {
        fileName = suffix_tree_path;
    }

    reverseIDMap.clear();
    QFile file(fileName.toStdString().data());

    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);    // read the data serialized from the file
        QString version;
        in >> version;

        if (version == cache_version()) {
            int num1, num2;
            QString letters;
            long affix_id;
            long category_id;
            long resulting_category_id;
            bool isAccept;
            QString raw_data, inflected_raw_data, descriptionInflectionRule;
            while (!in.atEnd()) {
                in >> letters >> affix_id >> category_id >> resulting_category_id >> isAccept
                   >> raw_data >> inflected_raw_data >> descriptionInflectionRule
                   >> num1 >> num2;
                node *n = addElement(letters, affix_id, category_id, resulting_category_id, isAccept,
                                     raw_data, inflected_raw_data, descriptionInflectionRule,
                                     getNodeID(num1));
                setNodeID(num2, n);
            }

            file.close();
            return 0;
        } else {
            file.close();
            return build_affix_tree(type);
        }
    } else {
        return build_affix_tree(type);
    }
}
int tree::build_affix_tree(item_types type) {
    reset();
    isAffix = true;
    this->type = type;
    database_info.prgsIFC->setCurrentAction(interpret_type(type).toUpper() + " TREE");
    QString fileName;

    if (type == PREFIX) {
        fileName = prefix_tree_path;
    } else if (type == SUFFIX) {
        fileName = suffix_tree_path;
    }

    last_id = 1; //bc base =0
    idMap.clear();
    QFile rawFile(fileName.toStdString().data());

    if (rawFile.open(QIODevice::WriteOnly)) {
        file = new QDataStream(&rawFile); // we will serialize the data into the file
        (*file) << cache_version();
    } else {
        _error << "Unexpected Error: Unable to write AFFIX Table to file\n";
        file = NULL;
    }

    QSqlQuery query(theSarf->db);
    QString stmt = QString("SELECT id, name FROM %1").arg(interpret_type(type));
    QString name;
    unsigned long affix_id1;

    if (!execute_query(stmt, query)) {
        return -1;
    }

    int size = query.size();
    int i = 0;

    while (query.next()) {
        name = query.value(1).toString();
        name = removeDiacritics(name);
        affix_id1 = query.value(0).toULongLong();
        Search_by_item s1(type, affix_id1);
        minimal_item_info inf;

        while (s1.retrieve(inf)) {
            bool isAccept = isAcceptState(type, inf.category_id);

            if (isAccept || hasCompatibleAffixes(type, inf.category_id)) {
                node *next = addElement(name, affix_id1, inf.category_id, inf.category_id, isAccept, inf.raw_data, inf.raw_data, "",
                                        base);
                //if (type!=STEM)
                build_helper(type, inf.category_id, 6 - name.length(), next);
            }
        }

        i++;
        database_info.prgsIFC->report((double)i / size * 100 + 0.5);
    }

    rawFile.close();

    if (file != NULL) {
        delete file;
    }

    file = NULL;
    return 0;
}
void tree::print_tree() {
    if (isAffix)
        theSarf->out    << QString().fill('-', 40) << "\n"
                        << "\t" << interpret_type(type) << " Tree\n"
                        << QString().fill('-', 40) << "\n";

    print_tree_helper(base, 0);
    theSarf->out << QString().fill('-', 40) << "\n"
                 << "letter nodes count= " << letter_nodes << "\n"
                 << "result nodes count= " << result_nodes << "\n"
                 << QString().fill('-', 40) << "\n";
}

#include <QTextStream>
#include <QFile>
#include "common.h"
#include <QQueue>
#include "tree.h"
#include "test.h"
#include "database_info_block.h"
#include "inflections.h"
#include <QString>
#include <QMainWindow>
#include <QScrollArea>
#include <QLabel>
#include <QDir>

typedef QMap<node *, int> AffixGraphMap;

inline int getGraphID(AffixGraphMap &map, int &last_id, node *n) {
    int curr_id;
    AffixGraphMap::iterator it = map.find(n);

    if (it == map.end()) {
        curr_id = ++last_id;
        map.insert(n, curr_id);
    } else {
        curr_id = it.value();
    }

    return curr_id;
}


void drawAffixGraph(item_types type) {
    int last_id = 0;
    AffixGraphMap map;
    map.clear();
    QString t = interpret_type(type);
    QQueue<node *> queue;
    node *base = (type == PREFIX ? database_info.Prefix_Tree->getFirstNode() : database_info.Suffix_Tree->getFirstNode());
    QFile file(QString("%1.dot").arg(t));
    file.remove();

    if (!file.open(QIODevice::ReadWrite)) {
        theSarf->out << "Error openning file\n";
        return;
    }

    QTextStream d_out(&file);
    d_out.setCodec("utf-8");
    d_out << "digraph tree {\n";
    queue.enqueue(base);

    while (!queue.isEmpty()) {
        node &n = *(queue.dequeue());

        if (n.isLetterNode()) {
            letter_node &l = (letter_node &)n;
            QChar letter = l.getLetter();
            d_out << "n" << getGraphID(map, last_id,
                                       &n) << " [label=\"" << (letter != '\0' ? letter : '-') << "\", shape=circle];\n";
        } else {
            result_node &r = (result_node &)n;
            d_out << "n" << getGraphID(map, last_id, &n) << " [label=\""
                  << database_info.comp_rules->getCategoryName(r.get_resulting_category_id())
                  << "\", shape=rectangle];\n";
            int size = r.raw_datas.size();

            for (int i = 0; i < size; i++) {
                d_out << "r" << getGraphID(map, last_id,
                                           &n) << "_" << i << " [label=\"" << r.raw_datas[i].getActual() << "\", shape=oval, color=red];\n";
                d_out << "n" << getGraphID(map, last_id, &n)
                      << "->" << "r" << getGraphID(map, last_id, &n) << "_" << i << ";\n";
            }

        }

        QVector<letter_node *> l_nodes = n.getLetterChildren();
        QList<result_node *> r_nodes = *n.getResultChildren();
        int size = l_nodes.size();

        for (int i = 0; i < size; i++) {
            node *nc = l_nodes[i];

            if (nc != NULL) {
                queue.enqueue(nc);
                d_out << "n" << getGraphID(map, last_id, &n) << "->" << "n" << getGraphID(map, last_id, nc) << ";\n";
            }
        }

        size = r_nodes.size();

        for (int i = 0; i < size; i++) {
            node *nc = r_nodes[i];

            if (nc != NULL) {
                queue.enqueue(nc);
                d_out << "n" << getGraphID(map, last_id, &n) << "->" << "n" << getGraphID(map, last_id, nc) << ";\n";
            }
        }
    }

    d_out << "}\n";
    file.close();

    try {
#ifdef DISPLAY
        system(QString("dot -Tsvg %1.dot -o %1.svg").arg(t).toStdString().data());
        QMainWindow *mw = new QMainWindow(NULL);
        mw->setWindowTitle(QString("%1 Tree").arg(t));
        QScrollArea *sa = new QScrollArea(mw);
        mw->setCentralWidget(sa);
        QLabel *pic = new QLabel(sa);
        pic->setPixmap(QPixmap(QString("./%1.svg").arg(t)));
        sa->setWidget(pic);
        mw->show();
#else
        system(QString("kgraphviewer ./%1.dot -caption \"%1.dot\" &").arg(t).arg(QDir::currentPath()).toStdString().data());
#endif
    } catch (...) {
    }
}
class ListAllAffixes {
    private:
        item_types type;
        ItemCatRaw2AbsDescPosMapPtr map;
        QFile *file;
        QTextStream *dout;
#define d_out *(dout)

        void traverse(node *n, QString affix, QString raw_data, QString description, QString POS) {
            if (!n->isLetterNode()) {
                result_node *r = (result_node *)n;
                QString inflectionRule = r->getInflectionRule();

                for (int i = 0; i < r->raw_datas.size(); i++) {
                    QString raw = raw_data + r->raw_datas[i].getActual();
                    QString cat = database_info.comp_rules->getCategoryName(r->get_resulting_category_id());
                    ItemEntryKey entry(r->get_affix_id(), r->get_previous_category_id(), r->raw_datas[i].getOriginal());
                    ItemCatRaw2AbsDescPosMapItr itr = map->find(entry);

                    while (itr != map->end() && itr.key() == entry) {
                        dbitvec d = itr.value().first;
                        QString pos2 = itr.value().third;
                        QString pos = POS;
                        applyPOSInflections(inflectionRule, pos2);
                        applyPOSInflections(inflectionRule, pos, true);
                        pos += pos2;
                        QString added_desc = (*database_info.descriptions)[itr.value().second];
                        applyDescriptionInflections(inflectionRule, added_desc);
                        applyDescriptionInflections(inflectionRule, description, true);
                        QString desc;

                        if (type == SUFFIX) {
                            QString suffix_delimitor = " ";
                            bool r = isReverseDirection(d);

                            //qDebug()<<raw_data<<" "<<r;
                            if (description[0] == '[' && description.size() > 0 && description[description.size() - 1] == ']' &&
                                !added_desc.isEmpty()) {
                                description = "";
                            }

                            if (description.contains("%1")) {
                                desc = description.arg(added_desc + (added_desc == "" || description == "%1" ? "" : suffix_delimitor));
                            } else {
                                if (r) {
                                    desc = "%1" + added_desc + (added_desc == "" || description == "" ? "" : suffix_delimitor) + description;
                                } else {
                                    desc = description + (added_desc == "" || description == "" ? "" : suffix_delimitor) + added_desc;
                                }
                            }

                            //desc=(isReverseDirection(d)?added_desc+(added_desc==""?"":" ")+description: description+(description==""?"":" ")+added_desc);
                            //desc=(isReverseDirection(d)?added_desc+(added_desc==""?"":" ")+description: description+(description==""?"":" ")+added_desc);
                        } else {
                            desc = description + (description == "" || added_desc == "" ? "" : " + ") + added_desc;
                        }

                        if (r->is_accept_state()) {
                            QString temp = desc;
                            temp.remove("%1");
                            d_out << affix << "\t" << raw << "\t" << cat << "\t" << temp << "\t" << pos << "\t\n";
                        }

                        QVector<letter_node *> letters = n->getLetterChildren();

                        for (int i = 0; i < letters.size(); i++) {
                            if (letters[i] != NULL) {
                                QChar letter = letters[i]->getLetter();
                                traverse(letters[i], (letter != '\0' ? affix + letter : affix), raw, desc, pos);
                            }
                        }

                        itr++;
                    }
                }
            } else {
                QVector<letter_node *> letters = n->getLetterChildren();
                QList<result_node *> *results = n->getResultChildren();

                for (int i = 0; i < results->size(); i++) {
                    traverse((*results)[i], affix, raw_data, description, POS);
                }

                for (int i = 0; i < letters.size(); i++) {
                    if (letters[i] != NULL) {
                        QChar letter = letters[i]->getLetter();
                        traverse(letters[i], (letter != '\0' ? affix + letter : affix), raw_data, description, POS);
                    }
                }
            }
        }
    public:
        ListAllAffixes(item_types type) {
            this->type = type;
            QString t = interpret_type(type);
            file = new QFile(QString("%1.list").arg(t));
            file->remove();

            if (!file->open(QIODevice::ReadWrite)) {
                theSarf->out << "Error openning file\n";
                return;
            }

            dout = new QTextStream(file);
            dout->setCodec("utf-8");
            map = (type == PREFIX ? database_info.map_prefix : database_info.map_suffix);
        }
        void operator()() {
            node *top = (type == PREFIX ? database_info.Prefix_Tree : database_info.Suffix_Tree)->getFirstNode();
            traverse(top, "", "", "", "");
        }
        ~ListAllAffixes() {
            file->close();
            QString t = interpret_type(type);
            system(QString("sort %1.list >%1_sorted.list ").arg(t).toStdString().data());
            delete file;
            delete dout;
        }
};

void listAllAffixes(item_types type) {
    ListAllAffixes l(type);
    l();
}

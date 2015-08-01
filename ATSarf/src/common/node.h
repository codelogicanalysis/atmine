#ifndef NODE_H
#define NODE_H

#include <QList>
#include <QString>
#include <QVector>
#include <QHash>
#include "common.h"
#include "Ptr.h"


inline int getLetterIndex(const QChar &letter) {
    int unicode = letter.unicode();

    if (unicode == 0) {
        return 36;
    }

    static int letter_map[42] = {0 /*0x621*/, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25/*0x63A*/,
                                 -1, -1, -1, -1, -1, -1, 26 /*0x641*/, 27, 28, 29, 30, 31, 32, 33, 34, 35
                                };
    int diff = unicode - 1569/*hamza.unicode()*/;

    if (!(diff > 0 && diff < 42)) {
        return -1;
    }

    return letter_map[diff];
}

class letter_node;
class result_node;

class node {
    private:
        QList<result_node *> *result_children;
        QVector<letter_node *> *letter_children;
        void removeChildren();
        void initialize(const node &n) {
            parent = n.parent;
            letter_children = new QVector<letter_node *>(*n.letter_children);
            result_children = new QList<result_node *>(*n.result_children);
        }
    public:
        node *parent;  //public member and can be defined by other classes such as treesearch upon traversal.
        node *getPrevious() {
            return parent;
        }
        node(): parent(NULL) {
            result_children = new QList<result_node *>;
            letter_children = new QVector<letter_node *>(37);
        }
        node(const node &n) {
            initialize(n);
        }
        node &operator=(const node &n) {
            initialize(n);
            return *this;
        }
        virtual bool isLetterNode() const = 0;
        virtual QString to_string(bool isAffix = false) = 0;
        bool hasChildren() {
            return (result_children->count() != 0 || letter_children->count() != 0);
        }
        QList<result_node *> *getResultChildren() {
            return result_children;
        }
        QVector<letter_node * > getLetterChildren() { //inefficient copy constructor, this function's use should be avoided
            return *letter_children; //must be changed later to filter empty nodes
        }
        letter_node *getLetterChild(QChar &letter) {
            int i = getLetterIndex(letter);

            if (i >= 0) {
                return letter_children->at(i);
            } else {
                return NULL;
            }
        }
        void addChild(node *child);
        void resetChildren() {
            removeChildren();
            result_children->clear();
        }
        virtual ~node() {
            delete letter_children;
            delete result_children;
        }
};

#endif

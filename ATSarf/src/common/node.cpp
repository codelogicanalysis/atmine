#include "node.h"
#include "letter_node.h"
#include "result_node.h"
#include "letters.h"
#include <assert.h>
#include <QDebug>

void node::removeChildren() {
    int length = result_children->count();

    for (int i = 0; i < length; i++) {
        result_children->at(i)->removeChildren();
        delete result_children->at(i);
    }

    #if defined(EXTENSIVE_TREE)
    length = letter_children->count();

    for (int i = 0; i < length; i++) {
        letter_node *child = letter_children->at(i);

        if (child != NULL) {
            child->removeChildren();
            delete child;
        }
    }

    #endif
}

void node::addChild(node *child) {
    if (child->isLetterNode()) {
        letter_node *l = (letter_node *)child;
        #if defined(EXTENSIVE_TREE)
        (*letter_children)[getLetterIndex(l->getLetter())] = l;
        #endif
    } else {
        result_children->append((result_node *)child);
    }

    child->parent = this;
}

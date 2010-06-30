#ifndef _LETTER_NODE_H
#define	_LETTER_NODE_H

#include "node.h"

class letter_node:public node
{
    private:
        QChar letter;
    public:
        letter_node(QChar letter);
		bool isLetterNode() const;
		QChar getLetter() const;
        void setLetter(QChar letter);
        QString to_string(bool =true);
        ~letter_node();
};

bool operator==(const  letter_node &p1, const  letter_node &p2);

bool operator!=(const  letter_node &p1, const  letter_node &p2);

bool operator<(const  letter_node &p1, const  letter_node &p2);

bool operator>(const  letter_node &p1, const  letter_node &p2);

bool operator<=(const  letter_node &p1, const  letter_node &p2);

bool operator>=(const  letter_node &p1, const letter_node &p2);


#endif	/* _LETTER_NODE_H */


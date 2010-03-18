#ifndef _LETTER_NODE_H
#define	_LETTER_NODE_H

class letter_node:public node
{
    private:
        QChar letter;
    public:
        letter_node(QChar letter);
        bool isLetterNode();
        QChar getLetter();
        void setLetter(QChar letter);
        QString to_string(bool =true);
        ~letter_node();
};

#endif	/* _LETTER_NODE_H */


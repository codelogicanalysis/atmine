#include "letter_node.h"

letter_node::letter_node(QChar letter)
{
    setLetter(letter);
}
bool letter_node::isLetterNode()
{
    return true;
}
QChar letter_node::getLetter()
{
    return letter;
}
void letter_node::setLetter(QChar letter)
{
    this->letter=letter;
}
QString letter_node::to_string(bool =true)
{
    return QString("--->(").append((letter=='\0'?'$':letter)).append(")");
}
letter_node::~letter_node(){	}



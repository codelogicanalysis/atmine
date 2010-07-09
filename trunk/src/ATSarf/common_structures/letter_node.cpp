#include "letter_node.h"


letter_node::letter_node(const letter_node & n):node(n)
{
	letter=n.letter;
}
letter_node& letter_node::operator=(const letter_node& n)
{
	letter=n.letter;
	return operator =(n);
}
letter_node::letter_node(QChar letter):node()
{
	setLetter(letter);
}
bool letter_node::isLetterNode() const
{
    return true;
}
QChar letter_node::getLetter() const
{
    return letter;
}
void letter_node::setLetter(QChar letter)
{
    this->letter=letter;
}
QString letter_node::to_string(bool)
{
    return QString("--->(").append((letter=='\0'?'$':letter)).append(")");
}
letter_node::~letter_node(){	}

bool operator==(const  letter_node &p1, const  letter_node &p2)
{ return (p1.getLetter() == p2.getLetter()); }

bool operator!=(const  letter_node &p1, const  letter_node &p2)
{ return !(p1 == p2); }

bool operator<(const  letter_node &p1, const  letter_node &p2)
{
	return p1.getLetter() < p2.getLetter();
}

bool operator>(const  letter_node &p1, const  letter_node &p2)
{
	return p2 < p1;
}

bool operator<=(const  letter_node &p1, const  letter_node &p2)
{
	return !(p2 < p1);
}

bool operator>=(const  letter_node &p1, const letter_node &p2)
{
	return !(p1 < p2);
}


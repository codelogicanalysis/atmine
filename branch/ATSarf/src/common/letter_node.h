#ifndef _LETTER_NODE_H
#define	_LETTER_NODE_H

#include "node.h"

class letter_node:public node
{
private:
	QChar letter;
public:
	letter_node(const letter_node & n):node(n)
	{
		letter=n.letter;
	}
	letter_node& operator=(const letter_node& n)
	{
		letter=n.letter;
		return operator =(n);
	}
	letter_node(QChar letter)
	{
		setLetter(letter);
	}
	bool isLetterNode() const
	{
		return true;
	}
	QChar getLetter() const
	{
		return letter;
	}
	void setLetter(QChar letter)
	{
		this->letter=letter;
	}
	QString to_string(bool =true)
	{
		return QString("--->(").append((letter=='\0'?'$':letter)).append(")");
	}
	~letter_node(){ }
};

inline bool operator==(const  letter_node &p1, const  letter_node &p2)
{ return (p1.getLetter() == p2.getLetter()); }

inline bool operator!=(const  letter_node &p1, const  letter_node &p2)
{ return !(p1 == p2); }

inline bool operator<(const  letter_node &p1, const  letter_node &p2)
{
	return p1.getLetter() < p2.getLetter();
}

inline bool operator>(const  letter_node &p1, const  letter_node &p2)
{
	return p2 < p1;
}

inline bool operator<=(const  letter_node &p1, const  letter_node &p2)
{
	return !(p2 < p1);
}

inline bool operator>=(const  letter_node &p1, const letter_node &p2)
{
	return !(p1 < p2);
}


#endif	/* _LETTER_NODE_H */


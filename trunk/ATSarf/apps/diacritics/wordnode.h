#ifndef WORDNODE_H
#define WORDNODE_H

#include <QString>
#include <QVector>
#include <QDataStream>
#include <QHash>

class WordNode {
public:
    int solutions;
    QHash<QString, int> vocalizations;
    WordNode() {
        solutions = 0;
        vocalizations.clear();
    }
};

inline QDataStream &operator>>(QDataStream &in, long & l) {
	qint64 s;
	in>>s;
	l=s;
	return in;
}

inline QDataStream &operator<<(QDataStream &out, const long & l) {
	out<<(qint64)l;
	return out;
}

inline QDataStream &operator>>(QDataStream &in, WordNode &node)
{
    in >> node.solutions;
    return in;
}
inline QDataStream &operator<<(QDataStream &out, const WordNode &node)
{
	out << node.solutions;
	return out;
}
QDataStream &operator>>(QDataStream &in, WordNode* &node);
QDataStream &operator<<(QDataStream &out, const WordNode* &node);

#endif // WORDNODE_H

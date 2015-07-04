#ifndef _VW_TRIE_H
#define	_VW_TRIE_H

#include <QString>
#include <QList>
#include <QVector>

class VWTrieData;

class VWTrie {
    private:
        VWTrieData * data;
    public:
        VWTrie();
        VWTrie(const char * path);
        ~VWTrie() ;

        void save(const char * path);
        bool store(const QString & key, long long index);
        bool remove(const QString & key);
        bool isDirty() const;

        typedef void * Position;

        Position startWalk();

        Position clonePosition(Position p);

        void savePosition(Position dst, Position src);

        void freePosition(Position p);

        void rewindPosition(Position p);

        bool walk(Position pos, QChar c);

        bool isWalkable(Position pos, QChar c);

        bool isTerminal(Position pos);

        bool isLeaf(Position pos);

        bool isSingle(Position pos);

        //given a terminal position
        // returns the node associated with the position
	int getData(Position pos,bool integer);
};

#endif

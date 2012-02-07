#ifndef _ATM_TRIE_H
#define	_ATM_TRIE_H


#include <QString>
#include <QList>
#include <QVector>
#include "common.h"
#include "StemNode.h"

class ATTrieData;
/*
class ATTrieEnumerator {
public:
    void * UserData;
	//virtual bool enumerator(const QString & key, StemNode * node) = 0;
	virtual bool enumerator(const QString & key, int index) = 0;
};
*/
#if 0
class ATTrieEnumeratorExample : public ATTrieEnumerator {
public:
    ATTrieEnumeratorExample(int * i) {
        UserData = (void*)i;
    }

    virtual bool enumerator(const QString & key, StemNode * node)
    {
        // we can cast and use UserData as needed here
        fprintf(stderr,"This is an example!\n");
        fprintf(stderr,"Node is %x\n", node);
        fprintf(stderr,"key is %s\n", key);
        return true;
    }
};
#endif

typedef QVector<StemNode> StemNodesList;

class ATTrie {
    private:
        ATTrieData * data;
    public:
		StemNodesList * nodes;

        ATTrie();
        ATTrie(const char * path);
        ~ATTrie() ;

        void save(const char * path);
		//bool store(const QString & key, StemNode * node) ;
		bool retreive(const QString & key, const StemNode ** node) ;
		bool retreive(const QString & key, int* index);
		bool store(const QString & key, int index) ;
        bool remove(const QString & key);
        bool isDirty() const;

        typedef void * Position;

        // starts the walk and returns a position pointer to
        // be used in the walk
        Position startWalk();
        // create a clone position, useful for backtracking
        Position clonePosition(Position p);
        // saves a position, useful not to create many clones
        void savePosition(Position dst, Position src);
        // frees the position
        void freePosition(Position p);
        // resets the position to the root of the trie
        void rewindPosition(Position p);

        // This is the actual walk routine
        // it advances pos if 'c' was an edge from the current
        // position and return true
        // otherwise it returns false
        bool walk(Position pos, QChar c);

        // Checks wherthe 'c' is an edge from the current position
        bool isWalkable(Position pos, QChar c);

        // checks if the current position is a terminal
        // position, in that case a StemNode is attached
        // to it and you can use getData
        bool isTerminal(Position pos);

        // checks whether we are at a leaf
        // this means we have a single and a terminal node
        bool isLeaf(Position pos);

        // checks whether we are at a single node
        // this means there is no other path from here to
        // the leaf (in other words we are on a single path)
        bool isSingle(Position pos);

        //given a terminal position
        // returns the node associated with the position
		int getData(Position pos,bool integer);
		const StemNode* getData(Position pos);

        // you need to implement the abstract class ATTrieEnumerator
        // to enumerate all stems in the trie
		//bool enumerate(ATTrieEnumerator* e);
};

#endif

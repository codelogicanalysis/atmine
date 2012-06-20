/**
  * @file   atmTrie.h
  * @brief  this header file contains the definition of the ATTrie structure, which is the trie storing the stem data
  * @author Jad Makhlouta
  */
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

/**
  * @class  ATTrie
  * @brief  This class implements the trie structure used to store the root arabic words present in our lexer
  * @author Jad Makhlouta
  */
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

        /**
          * This method starts the walk and returns a position pointer to be used in the walk
          */
        Position startWalk();

        /**
          * This method creates a clone position, useful for backtracking
          */
        Position clonePosition(Position p);

        /**
          * This method saves a position, useful not to create many clones
          */
        void savePosition(Position dst, Position src);

        /**
          * This method frees the position
          */
        void freePosition(Position p);

        /**
          * This method resets the position to the root of the trie
          */
        void rewindPosition(Position p);

        /**
          * This method implements the actual walk routine it advances pos if 'c' was an edge from the current position
          * @return This method returns true if such an advance is done, otherwise it returns false
          */
        bool walk(Position pos, QChar c);

        /**
          * This method checks whether 'c' is an edge from the current position
          */
        bool isWalkable(Position pos, QChar c);

        /**
          * This method checks if the current position is a terminal position, in that case a StemNode is attached
          * to it and you can use getData
          */
        bool isTerminal(Position pos);

        /**
          * This method checks whether we are at a leaf this means we have a single and a terminal node
          */
        bool isLeaf(Position pos);

        /**
          * This method checks whether we are at a single node this means there is no other path from here to
          * the leaf (in other words we are on a single path)
          */
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

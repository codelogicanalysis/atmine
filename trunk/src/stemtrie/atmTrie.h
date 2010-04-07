#include <QString>
#include <QList>

class ATTrieData;

class StemNode {
public:
  QString key;
  long catID;
};

class ATTrieEnumerator {
public:
    void * UserData;
    virtual bool enumerator(const QString & key, StemNode * node) = 0;
};
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

class ATTrie {
    private:
        ATTrieData * data;
    public:
        ATTrie();
        ATTrie(const char * path);
        ~ATTrie() ;

        void save(const char * path) ;
        bool store(const QString & key, StemNode * node) ;
        bool retreive(const QString & key, StemNode ** node) ;
        bool remove(const QString & key);
        bool isDirty() const;

        typedef void * Position;

        Position startWalk();
        Position clonePosition(Position p);
        void freePosition(Position p);
        void rewindPosition(Position p);
        bool walk(Position pos, QChar c);
        bool isWalkable(Position pos, QChar c);
        bool isTerminal(Position pos);

        bool isLeaf(Position pos);
        bool isSingle(Position pos);
        StemNode * getData(Position pos);


        // you need to implement the abstract class ATTrieEnumerator
        // to enumerate all stems in the trie
        bool enumerate(ATTrieEnumerator* e);
};

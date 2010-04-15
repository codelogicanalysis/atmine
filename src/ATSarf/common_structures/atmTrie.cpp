
extern "C" {
#include <config.h>
#include <datrie/trie.h>
}
#include "../common_structures/atmTrie.h"

#define AT_ALPHA_KEY_MAX_LENGTH 1024

class ATTrieData {
    public:
        AlphaMap * amap;
        Trie * trie;
        AlphaChar alpha_key[AT_ALPHA_KEY_MAX_LENGTH];
};


ATTrie :: ATTrie() : data(NULL) {
    data = new ATTrieData;
    data->amap = alpha_map_new();
    if (data->amap == NULL)
        throw "AlphaMapConstructionException";

    // add all characters except 0 and FF FF FF FF 
    // AlphaChar is uint32
    // this needs to change later to allow only Arabic and Latin
    alpha_map_add_range(data->amap, 1, 0xFFFFFFFE);
    data->trie = trie_new(data->amap);

    if (data->trie == NULL){
        alpha_map_free(data->amap);
        throw "TrieConstructionException";
    }
}

ATTrie :: ATTrie(const char * path) : data(NULL) {
    data = new ATTrieData;
    data->trie = trie_new_from_file(path);
    if (data->trie == NULL)
        throw "TrieConstructionException";
}

void 
ATTrie :: save(const char * path) 
{
    if (data->trie == NULL)
        return;

    int ret = trie_save(data->trie, path);
    if (ret != 0)
        throw "TrieSaveException";
}

bool 
ATTrie :: store(const QString & key, StemNode * node) 
{
    if (node == NULL)
        return false;
    if (data->trie == NULL)
        return false;
    if (key.length() > AT_ALPHA_KEY_MAX_LENGTH - 1)
        return false;

    for (int i = 0; i < key.length(); i++)
        data->alpha_key[i] = key[i].unicode();
    data->alpha_key[key.length()] = 0;

    TrieData d = (TrieData) node;

    Bool ret = trie_store(data->trie, data->alpha_key, d);
    return ret == TRUE;
}

bool 
ATTrie :: retreive(const QString & key, StemNode ** node) 
{
    if (node == NULL)
        return false;
    if (data->trie == NULL)
        return false;
    if (key.length() > AT_ALPHA_KEY_MAX_LENGTH - 1)
        return false;

    for (int i = 0; i < key.length(); i++)
        data->alpha_key[i] = key[i].unicode();
    data->alpha_key[key.length()] = 0;

    TrieData * o_d = (TrieData*) node;

    Bool ret = trie_retrieve (data->trie, data->alpha_key, o_d);
    return ret == true;
}

bool 
ATTrie :: remove(const QString & key)
{
    if (data->trie == NULL)
        return false;
    if (key.length() > AT_ALPHA_KEY_MAX_LENGTH - 1)
        return false;

    for (int i = 0; i < key.length(); i++)
        data->alpha_key[i] = key[i].unicode();
    data->alpha_key[key.length()] = 0;

    Bool ret = trie_delete(data->trie, data->alpha_key);
    return ret == true;
}

bool 
ATTrie :: isDirty() const
{
    Bool ret = trie_is_dirty(data->trie);
    return ret == true;
}

Bool 
at_trie_enum_func (
        const AlphaChar * key, 
        TrieData key_data, 
        void * user_data)
{
    ATTrieEnumerator* e = (ATTrieEnumerator*) user_data;
    StemNode * node = (StemNode*)key_data;
    return (Bool)(e->enumerator(node->key, node)? 1 : 0);
}


bool 
ATTrie :: enumerate(ATTrieEnumerator* e)
{
    return trie_enumerate(data->trie, at_trie_enum_func, (void*) e);
}

    ATTrie::Position 
ATTrie :: startWalk()
{
    return (void*) trie_root(data->trie);
}

ATTrie::Position ATTrie :: clonePosition(ATTrie::Position p)
{
    return trie_state_clone( (const TrieState*)p);
}

void ATTrie :: freePosition(ATTrie::Position p)
{
    trie_state_free((TrieState*)p);
}

void 
ATTrie :: savePosition(ATTrie :: Position dst, ATTrie :: Position src)
{
    trie_state_copy((TrieState*)dst, (TrieState*)src);
}

void ATTrie :: rewindPosition(ATTrie::Position p)
{
    trie_state_rewind((TrieState*)p);
}

bool ATTrie :: walk(ATTrie::Position pos, QChar c)
{
    Bool ret = trie_state_walk( (TrieState*)pos, c.unicode());
    return ret == true;
}

bool ATTrie :: isWalkable(ATTrie::Position pos, QChar c)
{
    Bool ret = trie_state_is_walkable( (TrieState*)pos, c.unicode());
    return ret == true;
}

bool ATTrie :: isTerminal(ATTrie::Position pos)
{
    Bool ret = trie_state_is_walkable( (TrieState*)pos,
            TRIE_CHAR_TERM);
    return ret == true;
}

bool ATTrie :: isLeaf(ATTrie::Position pos)
{
    return trie_state_is_leaf( (TrieState*)pos);
}

bool ATTrie :: isSingle(ATTrie::Position pos)
{
    Bool ret = trie_state_is_single( (TrieState*)pos);
    return ret == true;
}

StemNode *
ATTrie :: getData(ATTrie::Position pos)
{
    TrieData d = trie_state_get_data( (const TrieState*) pos);
    return (StemNode*) d;
}


ATTrie :: ~ATTrie() 
{
    if (data->trie != NULL)
        trie_free(data->trie);
    if (data->amap != NULL)
        alpha_map_free(data->amap);
    delete data;
}


extern "C" {
#include <datrie/trie.h>
}
#include "vwtrie.h"

#define AT_ALPHA_KEY_MAX_LENGTH 1024

class VWTrieData {
    public:
        AlphaMap * amap;
        Trie * trie;
        AlphaChar alpha_key[AT_ALPHA_KEY_MAX_LENGTH];
};

VWTrie :: VWTrie() : data(NULL) {
    data = new VWTrieData;
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

VWTrie :: VWTrie(const char * path) : data(NULL) {
        data = new VWTrieData;
        if (path == NULL) {
	data->amap = alpha_map_new();
	if (data->amap == NULL)
		throw "AlphaMapConstructionException";

	// add all characters except 0 and FF FF FF FF
	// AlphaChar is uint32
	// this needs to change later to allow only Arabic and Latin
	alpha_map_add_range(data->amap, 1, 0xFFFFFFFE);
        //data->trie = trie_new(data->amap);
    } else {
        data->trie = trie_new_from_file(path);
        data->amap = NULL;
        if (data->trie == NULL)
            throw "TrieConstructionException";
    }
}

void 
VWTrie :: save(const char * path)
{
    if (data->trie == NULL)
        return;

    int ret = trie_save(data->trie, path);
    if (ret != 0)
        throw "TrieSaveException";
}

bool 
VWTrie :: store(const QString & key, int index)
{
    if (data->trie == NULL)
        return false;
    if (key.length() > AT_ALPHA_KEY_MAX_LENGTH - 1)
        return false;

    for (int i = 0; i < key.length(); i++)
        data->alpha_key[i] = key[i].unicode();
    data->alpha_key[key.length()] = 0;

    TrieData d = (TrieData) index;

    Bool ret = trie_store(data->trie, data->alpha_key, d);
    return ret == TRUE;
}

bool 
VWTrie :: remove(const QString & key)
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
VWTrie :: isDirty() const
{
    Bool ret = trie_is_dirty(data->trie);
    return ret == true;
}

VWTrie::Position
VWTrie :: startWalk()
{
    return (void*) trie_root(data->trie);
}

VWTrie::Position VWTrie :: clonePosition(VWTrie::Position p)
{
    return trie_state_clone( (const TrieState*)p);
}

void VWTrie :: freePosition(VWTrie::Position p)
{
    trie_state_free((TrieState*)p);
}

void 
VWTrie :: savePosition(VWTrie :: Position dst, VWTrie :: Position src)
{
    trie_state_copy((TrieState*)dst, (TrieState*)src);
}

void VWTrie :: rewindPosition(VWTrie::Position p)
{
    trie_state_rewind((TrieState*)p);
}

bool VWTrie :: walk(VWTrie::Position pos, QChar c)
{
    Bool ret = trie_state_walk( (TrieState*)pos, c.unicode());
    return ret == true;
}

bool VWTrie :: isWalkable(VWTrie::Position pos, QChar c)
{
    Bool ret = trie_state_is_walkable( (TrieState*)pos, c.unicode());
    return ret == true;
}

bool VWTrie :: isTerminal(VWTrie::Position pos)
{
    Bool ret = trie_state_is_walkable( (TrieState*)pos,
            TRIE_CHAR_TERM);
    return ret == true;
}

bool VWTrie :: isLeaf(VWTrie::Position pos)
{
    return trie_state_is_leaf( (TrieState*)pos);
}

bool VWTrie :: isSingle(VWTrie::Position pos)
{
    Bool ret = trie_state_is_single( (TrieState*)pos);
    return ret == true;
}

int
VWTrie :: getData(VWTrie::Position pos, bool)
{
    TrieData d = trie_state_get_data( (const TrieState*) pos);
    if (d==TRIE_DATA_ERROR)
        return -1;
    return (int) d;
}

VWTrie :: ~VWTrie()
{
    if (data->trie != NULL)
        trie_free(data->trie);
    if (data->amap != NULL)
        alpha_map_free(data->amap);
    delete data;
}

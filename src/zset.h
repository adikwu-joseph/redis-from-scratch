#pragma once
#include "avl.h"
#include "hashtable.h"

struct ZNode {
    AVLNode tree;
    HNode hmap;
    double score = 0;
    size_t len = 0;
    char name[0];
};

struct ZSet {
    AVLNode *root = NULL;
    HMap hmap;
};

ZNode *zset_lookup(ZSet *zset, const char *name, size_t len);
bool   zset_insert(ZSet *zset, const char *name, size_t len, double score);
void   zset_delete(ZSet *zset, ZNode *node);
ZNode *zset_seekge(ZSet *zset, double score, const char *name, size_t len);
ZNode *znode_offset(ZNode *node, int64_t offset);
void   zset_clear(ZSet *zset);
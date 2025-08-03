#ifndef DS_H
#define DS_H
#include <stdbool.h>
#include <stddef.h>

typedef struct substring {
  char *str;
  size_t len;
} substring;

typedef struct ASTNode ASTNode;
typedef struct tokenStream tokenStream;

typedef struct memPool {
  ASTNode *nodes;
  size_t capacity;
  size_t used;
} memPool;

bool memPool_init(memPool *pool, size_t capacity);
ASTNode *memPool_alloc(memPool *nodes);
void memPool_reset(memPool *nodes);
void memPool_free(memPool *nodes);

typedef struct entry {
  substring key;
  ASTNode *value;
  size_t treeSize;
  size_t declarationStartIndex;
  struct entry *next;
} entry;

typedef struct hashMap {
  size_t size;
  entry **buckets;
} hashMap;

hashMap *hashMap_init(hashMap *map, size_t size);
bool hashmap_setKey(hashMap *map, const substring key, ASTNode *value,
                    size_t treeSize, size_t declarationStartIndex);
ASTNode *hashMap_getValue(const hashMap *map, const substring key,
                          size_t *treeSize, size_t *declaratonStartIndex);
void hashMap_free(hashMap *map);

#endif

#include "ds.h"
#include "parser.h"
#include <stdlib.h>
#include <string.h>

/*--SUBSTRING--*/
bool substringCmp(substring s1, substring s2) {
  if (s1.len != s2.len)
    return false;
  return memcmp(s1.str, s2.str, s1.len) == 0;
}

/*--MEMORY POOl--*/
bool memPool_init(memPool *pool, size_t capacity) {
  pool->nodes = malloc(sizeof(ASTNode) * (capacity * 2));
  if (!pool->nodes) {
    return false;
  }

  pool->capacity = capacity * 2;
  pool->used = 0;
  return true;
}

inline ASTNode *memPool_alloc(memPool *pool) {
  return (pool->used >= pool->capacity) ? NULL : &pool->nodes[pool->used++];
}

void memPool_reset(memPool *pool) { pool->used = 0; }

void memPool_free(memPool *pool) {
  if (pool->nodes) {
    free(pool->nodes);
  }
}

/*--HASH MAP--*/
static inline size_t hash(substring key) {
  size_t h = 5381;
  int c;

  for (size_t i = 0; i < key.len; i++) {
    c = key.str[i];
    h = ((h << 5) + h) + c;
  }
  return h;
}

static inline entry *entryInit(const substring key, ASTNode *value) {
  entry *e = malloc(sizeof(entry));
  if (!e)
    return NULL;

  *e = (entry){
      .key = key,
      .value = value,
      .next = NULL,
  };

  return e;
}

hashMap *hashMap_init(hashMap *map, size_t size) {
  map->size = size;
  map->buckets = calloc(size, sizeof(entry *));
  return map;
}

bool hashmap_setKey(hashMap *map, const substring key, ASTNode *value) {
  size_t idx = hash(key) % map->size;
  entry *cur = map->buckets[idx];

  while (cur) {
    if (substringCmp(cur->key, key)) {
      cur->value = value;
      return true;
    }
    cur = cur->next;
  }

  entry *entry = entryInit(key, value);
  if (!entry) {
    return false;
  }
  entry->next = map->buckets[idx];
  map->buckets[idx] = entry;
  return true;
}

ASTNode *hashMap_getValue(const hashMap *map, const substring key) {
  size_t idx = hash(key) % map->size;
  entry *cur = map->buckets[idx];

  while (cur) {
    if (substringCmp(cur->key, key)) {
      return cur->value;
    }
    cur = cur->next;
  }

  return NULL;
}

void hashMap_free(hashMap *map) {
  for (size_t i = 0; i < map->size; i++) {
    entry *cur = map->buckets[i];

    while (cur) {
      entry *tmp = cur;
      cur = cur->next;
      free(tmp);
    }
  }

  free(map->buckets);
}

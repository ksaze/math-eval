#include "ds.h"
#include "parser.h"
#include <stdlib.h>

bool memPool_init(memPool *pool, size_t capacity) {
  pool->nodes = malloc(sizeof(ASTNode) * capacity);
  if (!pool->nodes) {
    return false;
  }

  pool->capacity = capacity;
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

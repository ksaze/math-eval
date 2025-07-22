#ifndef DS_H
#define DS_H
#include <stdbool.h>
#include <stddef.h>

typedef struct parserNode parserNode;

typedef struct memPool {
  parserNode *nodes;
  size_t capacity;
  size_t used;
} memPool;

bool memPool_init(memPool *pool, size_t capacity);
parserNode *memPool_alloc(memPool *nodes);
void memPool_reset(memPool *nodes);
void memPool_free(memPool *nodes);

#endif

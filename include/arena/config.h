#ifndef ARENA_CONFIG_H
#define ARENA_CONFIG_H
#include <stdint.h>

typedef void (*ArenaFreeFunction)(void* data);

typedef struct {
  int64_t page_size;
  int64_t alignment;
  ArenaFreeFunction free_function;
} ArenaConfig;

#endif

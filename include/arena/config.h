#ifndef ARENA_CONFIG_H
#define ARENA_CONFIG_H
#include <stdint.h>

typedef void (*ArenaFreeFunction)(void* data);
typedef void (*ArenaIterFunction)(void* user_ptr, void* data_ptr);

typedef struct {
  int64_t item_size;
  int64_t items_per_page;
  int64_t page_size;
  int64_t alignment;
  ArenaFreeFunction free_function;

} ArenaConfig;

#endif

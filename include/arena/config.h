#ifndef ARENA_CONFIG_H
#define ARENA_CONFIG_H
#include <stdint.h>

typedef void (*ArenaFreeFunction)(void* data);
typedef void (*ArenaFreeFunctionWithUserPtr)(void* data, void* user_ptr);
typedef void (*ArenaIterFunction)(void* user_ptr, void* data_ptr);

typedef struct {
  int64_t item_size;
  int64_t items_per_page;
//  int64_t page_size;
  int64_t alignment;
  ArenaFreeFunction free_function;
  ArenaFreeFunctionWithUserPtr free_function_with_user_ptr;
  void* user_ptr_free;

} ArenaConfig;

#endif

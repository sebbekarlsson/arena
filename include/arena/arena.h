#ifndef ARENA_ARENA_H
#define ARENA_ARENA_H
#include <arena/config.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <arena/buffer.h>


typedef struct {
  int64_t page;

  int64_t data_start;
  int64_t data_size;


  void* ptr;
  struct ARENA_STRUCT* arena;

  int64_t id;
  bool in_use;
} ArenaRef;

ARENA_DEFINE_BUFFER(ArenaRef);

typedef struct ARENA_STRUCT {
  void* data;

  ArenaRef* last_free_ref;
  ArenaRef* refs;

//  ArenaArenaRefBuffer freed_memory;

  volatile int64_t size;
  volatile int64_t current;
  volatile int64_t malloc_length;
  volatile int64_t free_length;
  volatile int64_t pages;
  volatile int64_t total_count;

  int64_t page_size;

  struct ARENA_STRUCT* next;
  struct ARENA_STRUCT* prev;

  ArenaConfig config;

  bool initialized;
  bool broken;
  bool is_root;
} Arena;





int arena_init(Arena* arena, ArenaConfig cfg);

void* arena_malloc(Arena* arena, ArenaRef* ref);

int arena_free(ArenaRef ref);

int arena_clear(Arena* arena);

int arena_destroy(Arena* arena);

bool arena_is_broken(Arena arena);

typedef struct {
  Arena* arena;
  ArenaRef ref;
} ArenaIterator;

int arena_iterate(Arena* arena, ArenaIterator* it);

int64_t arena_get_allocation_count(Arena arena);

int arena_reset(Arena *arena);

int arena_defrag(Arena* arena);

#endif

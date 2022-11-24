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


  struct ARENA_STRUCT* arena;
} ArenaRef;

ARENA_DEFINE_BUFFER(ArenaRef);

typedef struct ARENA_STRUCT {
  void* data;

  ArenaArenaRefBuffer freed_memory;

  volatile int64_t size;
  volatile int64_t current;

  struct ARENA_STRUCT* next;

  ArenaConfig config;

  bool initialized;
  bool broken;
} Arena;




int arena_init(Arena* arena, ArenaConfig cfg);

void* arena_malloc(Arena* arena, int64_t size, ArenaRef* ref);

int arena_free(Arena* arena, ArenaRef ref);

int arena_clear(Arena* arena);

int arena_destroy(Arena* arena);

bool arena_is_broken(Arena arena);

#endif

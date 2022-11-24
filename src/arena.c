#include <arena/arena.h>
#include <arena/constants.h>
#include <arena/macros.h>
#include <stdio.h>
#include <stdlib.h>

ARENA_IMPLEMENT_BUFFER(ArenaRef);

int arena_init(Arena* arena, ArenaConfig cfg) {
  if (!arena) return 0;
  if (arena->initialized) return 1;
  arena->initialized = true;

  cfg.alignment = OR(cfg.alignment, ARENA_ALIGNMENT);
  cfg.page_size = OR(cfg.page_size, ARENA_PAGE_SIZE);
  cfg.page_size = ARENA_ALIGN_UP(cfg.page_size, cfg.alignment);
  arena_ArenaRef_buffer_init(&arena->freed_memory);
  arena->config = cfg;

  if (!ARENA_IS_POWER_OF_2(cfg.alignment)) {
    ARENA_WARNING(stderr, "alignment is not power of 2!\n");
    arena->initialized = false;
    return 0;
  }

  arena->next = 0;
  arena->data = 0;
  arena->current = 0;
  arena->size = 0;
  arena->broken = false;

  return 1;
}

int arena_free(Arena* arena, ArenaRef ref) {
  if (!arena) return 0;
  if (!arena->initialized) ARENA_WARNING_RETURN(0, stderr, "Arena not initialized.\n");
  if (arena_is_broken(*arena)) ARENA_WARNING_RETURN(0, stderr, "This arena is broken.\n");
  if (ref.data_size <= 0) ARENA_WARNING_RETURN(0, stderr, "Reference has no size.\n");



  if (ref.arena == 0) {
    int64_t id = 0;

    Arena* page = arena;

    if (ref.page != 0) {
      while (page != 0) {
        page = arena->next;

        if (id >= ref.page) break;

        id++;
      }
    }
    ref.arena = page;
  }




  if (!ref.arena) ARENA_WARNING_RETURN(0, stderr, "Could not find page.\n");
  if (!ref.arena->data || ref.arena->size <= 0) ARENA_WARNING_RETURN(0, stderr, "Arena page has no data.\n");


  arena_ArenaRef_buffer_push(&ref.arena->freed_memory, ref);
  return 1;
}

static void* arena_malloc_(Arena* arena, int64_t size, ArenaRef* ref) {
  if (!arena) ARENA_WARNING_RETURN(0, stderr, "arena == null.\n");
  if (!arena->initialized) ARENA_WARNING_RETURN(0, stderr, "Arena not initialized.\n");
  if (size <= 0) ARENA_WARNING_RETURN(0, stderr, "Invalid allocation size of %ld bytes.\n", size);
  if (arena_is_broken(*arena)) ARENA_WARNING_RETURN(0, stderr, "This arena is broken.\n");

  //int64_t og_size = size;
  size = ARENA_ALIGN_UP(size, arena->config.alignment);

  int64_t data_size = size > arena->config.page_size ? size : arena->config.page_size;


  if (!arena->data) {
    arena->data = calloc(1, data_size);
    arena->size = data_size;
  }

  if (!arena->data) {
    arena->broken = true;
    ARENA_WARNING_RETURN(0, stderr, "Arena has failed to allocate more memory.\n");
  }


  int64_t avail = arena->size - arena->current;


  if (avail >= size) {
    int64_t data_start = arena->current;

    arena->current += size;

    if (ref != 0) {
      ref->data_size = size;
      ref->data_start = data_start;
      ref->arena = arena;
    }

    return arena->data + data_start;
  }


  if (arena->freed_memory.length > 0) {
    for (int64_t i = 0; i < arena->freed_memory.length; i++) {
      ArenaRef range = arena->freed_memory.items[i];


      if (range.data_size >= size) {
        if (ref != 0) {
          ref->data_size = size;
          ref->data_start = range.data_start;
        }

        void* data = arena->data + range.data_start;

        if (arena->config.free_function != 0) {
          arena->config.free_function(data);
        }

        arena_ArenaRef_buffer_remove(&arena->freed_memory, i);

        return data;
      }
    }
  }

  return 0;
}


void* arena_malloc(Arena* arena, int64_t size, ArenaRef* ref) {
  if (!arena) ARENA_WARNING_RETURN(0, stderr, "arena == null.\n");
  if (!arena->initialized) ARENA_WARNING_RETURN(0, stderr, "Arena not initialized.\n");
  if (size <= 0) ARENA_WARNING_RETURN(0, stderr, "Invalid allocation size of %ld bytes.\n", size);
  if (arena_is_broken(*arena)) ARENA_WARNING_RETURN(0, stderr, "This arena is broken.\n");

  Arena* last = arena;

  void* data = 0;

  if (ref != 0) {
    ref->page = 0;
    ref->data_start = 0;
    ref->data_size = 0;
    ref->arena = last;
  }

  while (last != 0 && last->broken == false) {
    data = arena_malloc_(last, size, ref);

    if (data != 0) {
      ref->arena = last;
      return data;
    }

    if (ref != 0) {
      ref->page++;
    }

    if (last->next == 0) {
      Arena* next = NEW(Arena);
      arena_init(next, last->config);
      last->next = next;
    }
    last = last->next;
  }



  if (data == 0) {
    ARENA_WARNING_RETURN(0, stderr, "Failed to allocate memory.\n");
  }

  return data;
}



int arena_clear(Arena* arena) {
  if (!arena) return 0;
  if (!arena->initialized) ARENA_WARNING_RETURN(0, stderr, "Arena not initialized.\n");


  if (arena->freed_memory.length > 0) {
    for (int64_t i = 0; i < arena->freed_memory.length; i++) {
      ArenaRef range = arena->freed_memory.items[i];

      void* data = arena->data + range.data_start;

      if (arena->config.free_function != 0) {
        arena->config.free_function(data);
      }

    }
    arena_ArenaRef_buffer_clear(&arena->freed_memory);
  }

  if (arena->data != 0) {
    free(arena->data);
    arena->data = 0;
  }

  arena->size = 0;
  arena->current = 0;
  arena->broken = false;
  return 1;
}

int arena_destroy(Arena* arena) {
  if (!arena) return 0;
  if (!arena->initialized) ARENA_WARNING_RETURN(0, stderr, "Arena not initialized.\n");

  arena_clear(arena);

  if (arena->next != 0) {
    Arena* next = arena->next;
    while (next != 0) {
      arena_clear(next);

      Arena* prev = next;
      next = prev->next;

      free(prev);
      prev = 0;
    }
  }

  arena->next = 0;

  return 1;
}

bool arena_is_broken(Arena arena) {
  if (!arena.initialized) return false;
  return arena.broken;
}

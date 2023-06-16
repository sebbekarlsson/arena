#include <arena/arena.h>
#include <arena/constants.h>
#include <arena/macros.h>
#include <stdio.h>
#include <stdlib.h>

ARENA_IMPLEMENT_BUFFER(ArenaRef);

static bool arena_ref_can_be_used(ArenaRef ref) {
  return ref.in_use == false && ref.ptr != 0 && ref.arena != 0 &&
	 ref.data_size > 0;
}

int arena_init(Arena *arena, ArenaConfig cfg) {
  if (!arena)
    return 0;
  if (arena->initialized)
    return 1;

  cfg.items_per_page = OR(cfg.items_per_page, ARENA_ITEMS_PER_PAGE);

  if (!cfg.item_size) {
    ARENA_WARNING_RETURN(0, stderr, "No item_size provided.\n");
  }

  arena->initialized = true;

  cfg.alignment = OR(cfg.alignment, ARENA_ALIGNMENT);

  arena->page_size = cfg.item_size * cfg.items_per_page;
  arena->page_size = ARENA_ALIGN_UP(arena->page_size, cfg.alignment);

  arena->refs = 0;
  arena->last_free_ref = 0;
  arena->malloc_length = 0;
  arena->free_length = 0;
  arena->total_count = 0;

  // cfg.page_size = OR(cfg.page_size, ARENA_PAGE_SIZE);
  //  cfg.page_size = ARENA_ALIGN_UP(cfg.page_size, cfg.alignment);

  if (!ARENA_IS_POWER_OF_2(cfg.alignment)) {
    ARENA_WARNING(stderr, "alignment is not power of 2!\n");
    arena->initialized = false;
    return 0;
  }

  arena->config = cfg;
  arena->next = 0;
  arena->data = 0;
  arena->current = 0;
  arena->size = 0;
  arena->broken = false;

  return 1;
}

int arena_free(ArenaRef ref) {

  Arena *arena = ref.arena;

  if (!arena)
    return 0;
  if (!arena->initialized)
    ARENA_WARNING_RETURN(0, stderr, "Arena not initialized.\n");
  if (arena_is_broken(*arena))
    ARENA_WARNING_RETURN(0, stderr, "This arena is broken.\n");

  if (arena->refs == 0)
    ARENA_WARNING_RETURN(0, stderr, "refs == null.\n");

  if (ref.id >= arena->config.items_per_page || ref.id <= -1)
    ARENA_WARNING_RETURN(0, stderr, "ref.id is invalid.\n");

  ArenaRef *private_ref = &arena->refs[ref.id];
  private_ref->in_use = false;
  arena->last_free_ref = private_ref;
  arena->free_length++;

  // arena_ArenaRef_buffer_push(&ref.arena->freed_memory, ref);
  return 1;
}

static ArenaRef *arena_malloc_(Arena *arena) {
  if (!arena)
    ARENA_WARNING_RETURN(0, stderr, "arena == null.\n");
  if (!arena->initialized)
    ARENA_WARNING_RETURN(0, stderr, "Arena not initialized.\n");

  int64_t size = arena->config.item_size;

  if (size <= 0)
    ARENA_WARNING_RETURN(0, stderr, "Invalid allocation size of %ld bytes.\n",
			 size);
  if (arena_is_broken(*arena))
    ARENA_WARNING_RETURN(0, stderr, "This arena is broken.\n");

  // int64_t og_size = size;
  size = ARENA_ALIGN_UP(size, arena->config.alignment);

  int64_t data_size = size > arena->page_size ? size : arena->page_size;

  if (!arena->data) {
    arena->data = calloc(1, data_size);
    arena->size = data_size;
  }

  arena->size = MAX(arena->size, data_size);

  if (!arena->data) {
    arena->broken = true;
    ARENA_WARNING_RETURN(0, stderr,
			 "Arena has failed to allocate more memory.\n");
  }

  if (!arena->refs) {
    arena->refs =
	(ArenaRef *)calloc(arena->config.items_per_page, sizeof(ArenaRef));
  }

  if (arena->malloc_length >= arena->config.items_per_page) {
    goto find_free_ref;
  }

  int64_t id =
      arena->malloc_length; // arena->current / arena->config.items_per_page;
  ArenaRef *ref = &arena->refs[id];

  if (arena->malloc_length > 0) {
    if (ref->ptr != 0 && ref->in_use) {
      goto find_free_ref;
    }
  }

  int64_t avail = arena->size - arena->current;

  if (avail >= size) {
    int64_t data_start = arena->current;

    arena->current += size;
    arena->malloc_length++;

    ref->data_size = size;
    ref->data_start = data_start;
    ref->arena = arena;
    ref->id = id;

    ref->ptr = arena->data + data_start;
    ref->arena = arena;
    ref->in_use = true;
    return ref;
  }

find_free_ref:
  if (arena->last_free_ref != 0 &&
      arena_ref_can_be_used(*arena->last_free_ref)) {
    ref = arena->last_free_ref;

    if (ref->ptr != 0) {
      if (arena->config.free_function != 0) {
        arena->config.free_function(ref->ptr);
      } else if (arena->config.free_function_with_user_ptr != 0) {
        arena->config.free_function_with_user_ptr(ref->ptr, arena->config.user_ptr_free);
      }
    }
    ref->in_use = true;
    arena->free_length = MAX(0, arena->free_length - 1);
    arena->last_free_ref = 0;
    return ref;
  }

  if (arena->free_length > 0) {
    for (int64_t i = 0; i < arena->config.items_per_page; i++) {
      ref = &arena->refs[i];
      if (!arena_ref_can_be_used(*ref))
	continue;

      if (arena->config.free_function) {
	arena->config.free_function(ref->ptr);
      } else if (arena->config.free_function_with_user_ptr != 0) {
        arena->config.free_function_with_user_ptr(ref->ptr, arena->config.user_ptr_free);
      }

      ref->in_use = true;
      arena->free_length = MAX(0, arena->free_length - 1);
      return ref;
    }
  }

  return 0;
}

void *arena_malloc(Arena *arena, ArenaRef *user_ref) {
  if (!arena)
    ARENA_WARNING_RETURN(0, stderr, "arena == null.\n");
  if (!arena->initialized)
    ARENA_WARNING_RETURN(0, stderr, "Arena not initialized.\n");
  if (arena_is_broken(*arena))
    ARENA_WARNING_RETURN(0, stderr, "This arena is broken.\n");

  int64_t size = arena->config.item_size;

  if (size <= 0)
    ARENA_WARNING_RETURN(0, stderr, "Invalid allocation size of %ld bytes.\n",
			 size);
  if (arena->config.item_size > 0 && size != arena->config.item_size)
    ARENA_WARNING_RETURN(0, stderr, "size != item_size");

  arena->is_root = true;
  Arena *last = arena;

  ArenaRef *ref = 0;

  user_ref->page = 0;
  int64_t page = 0;

  while (last != 0 && last->broken == false) {
    ref = arena_malloc_(last);

    if (ref != 0 && ref->ptr != 0 && ref->arena != 0) {
      *user_ref = *ref;
      user_ref->page = page;
      arena->total_count++;
      return ref->ptr;
    }

    if (last->next == 0) {
      Arena *next = NEW(Arena);
      arena_init(next, arena->config);
      next->prev = last;
      last->next = next;
      arena->pages++;
    }
    Arena* tmp = last;
    last = last->next;

    page++;
  }

  printf("%ld\n", page);
  if (page > 0) exit(0);
  ARENA_WARNING_RETURN(0, stderr, "Failed to allocate memory.\n");
}

int arena_clear(Arena *arena) {
  if (!arena)
    return 0;
  if (!arena->initialized)
    ARENA_WARNING_RETURN(0, stderr, "Arena not initialized.\n");

  if (arena->refs != 0) {
    for (int64_t i = 0; i < arena->config.items_per_page; i++) {
      ArenaRef *ref = &arena->refs[i];
      if (ref->in_use || ref->ptr == 0)
	continue;

      if (arena->config.free_function != 0) {
	arena->config.free_function(ref->ptr);
      } else if (arena->config.free_function_with_user_ptr != 0) {
        arena->config.free_function_with_user_ptr(ref->ptr, arena->config.user_ptr_free);
      }

      ref->in_use = false;
      ref->ptr = 0;
      ref->arena = 0;
      ref->data_size = 0;
      ref->data_start = 0;
      ref->page = 0;
      arena->free_length = MAX(0, arena->free_length - 1);
    }

    free(arena->refs);
    arena->refs = 0;
  }

  // arena_ArenaRef_buffer_clear(&arena->freed_memory);

  if (arena->data != 0) {
    free(arena->data);
    arena->data = 0;
  }

  arena->malloc_length = 0;
  arena->free_length = 0;
  arena->total_count = 0;

  arena->size = 0;
  arena->current = 0;
  arena->broken = false;
  arena->pages = 0;
  return 1;
}

static int arena_destroy_private(Arena *arena, bool should_free) {
  if (!arena)
    return 0;
  if (!arena->initialized)
    ARENA_WARNING_RETURN(0, stderr, "Arena not initialized.\n");

  arena_reset(arena);
  arena_clear(arena);

  if (arena->next != 0) {
    arena_destroy_private(arena->next, true);
    arena->next = 0;
  }

  if (should_free) {
    free(arena);
    arena = 0;
  }

  return 1;
}

int arena_destroy(Arena *arena) {
  if (!arena)
    return 0;
  if (!arena->initialized)
    ARENA_WARNING_RETURN(0, stderr, "Arena not initialized.\n");

  return arena_destroy_private(arena, false);
}

bool arena_is_broken(Arena arena) {
  if (!arena.initialized)
    return false;
  return arena.broken;
}

int arena_iterate(Arena *arena, ArenaIterator *it) {
  if (!arena || !it)
    return 0;

find_arena:

  if (it->ref.id >= arena->malloc_length) {
    if (it->arena != 0 && it->arena->next != 0) {
      it->arena = it->arena->next;
    } else {
      return 0;
    }
  }

  if (it->arena != 0) {
    arena = it->arena;
  }

  int64_t id = it->ref.id;
  void *ptr = it->ref.ptr;

  for (int64_t i = id; i < arena->malloc_length; i++) {
    ArenaRef ref = arena->refs[i];
    if (!ref.ptr || ref.ptr == ptr)
      continue;

    it->ref = ref;
    it->ref.id = i;
    it->arena = arena;
    break;
  }

  bool found = it->ref.ptr != 0 && it->ref.ptr != ptr;

  if (!found && arena->next != 0) {
    it->ref.id = 0;
    it->arena = arena->next;
    goto find_arena;
  }

  return found ? 1 : 0;
}

int64_t arena_get_allocation_count(Arena arena) {
  if (!arena.initialized)
    return 0;
  if (arena.data == 0)
    return 0;
  return arena.total_count;
}

int arena_reset(Arena *arena) {
  if (!arena)
    return 0;

  if (!arena->initialized)
    ARENA_WARNING_RETURN(0, stderr, "Arena not initialized.\n");

  arena->current = 0;
  arena->malloc_length = 0;
  arena->free_length = 0;
  arena->pages = 0;
  arena->broken = false;
  arena->size = 0;
  arena->total_count = 0;

  if (arena->refs != 0) {
    for (int64_t i = 0; i < arena->config.items_per_page; i++) {
      ArenaRef *ref = &arena->refs[i];
      //      if (ref->in_use || ref->ptr == 0)
      //      continue;

      if (ref->arena != 0 && ref->ptr != 0 && ref->data_size > 0) {
	if (arena->config.free_function) {
	  arena->config.free_function(ref->ptr);
	} else if (arena->config.free_function_with_user_ptr != 0) {
          arena->config.free_function_with_user_ptr(ref->ptr, arena->config.user_ptr_free);
        }
      }

      ref->in_use = false;
      ref->ptr = 0;
      ref->arena = 0;
      ref->data_size = 0;
      ref->data_start = 0;
      ref->page = 0;
      arena->free_length = MAX(0, arena->free_length - 1);
    }

    //    free(arena->refs);
    //   arena->refs = 0;
  }

  // arena->last_free_ref = 0;

  if (arena->next != 0) {
    arena_reset(arena->next);
  }

  return 1;
}

bool arena_is_clean(Arena *arena) {
  if (arena->free_length >= arena->config.items_per_page)
    return true;

  int64_t count = 0;
  for (int64_t i = 0; i < arena->config.items_per_page; i++) {
    ArenaRef *ref = &arena->refs[i];
    count += (int)(arena_ref_can_be_used(*ref) || ref->ptr == 0);
  }

  return count >= arena->config.items_per_page;
}

static Arena *arena_get_root(Arena *arena) {
  if (!arena) return 0;
  if (arena->is_root) return arena;


  if (!arena->prev) return 0;
  Arena* left = arena->prev;

  while (left != 0) {
    if (left->is_root) return left;
    left = left->prev;
  }

  if (left != 0 && left->is_root) return left;
  return 0;
}

int arena_defrag(Arena *arena) {
  if (!arena)
    return 0;
  if (!arena->initialized)
    ARENA_WARNING_RETURN(0, stderr, "Arena not initialized.\n");

  Arena* prev = arena->prev;
  Arena* next = arena->next;


  if (!arena_is_clean(arena) && next != 0) return arena_defrag(next);


  if (arena->is_root || !arena_is_clean(arena)) return 0;

  if (prev && prev->next == arena) {
    prev->next = next;
  }

  if (next && next->prev == arena) {
    next->prev = prev;
  }


  Arena* root = arena_get_root(arena);

  if (!root) ARENA_WARNING_RETURN(0, stderr, "Expected root.\n");
  
  root->pages = MAX(root->pages-1, 0);

  arena_reset(arena);
  arena_clear(arena);
  free(arena);
  arena = 0;

  return 1;
}

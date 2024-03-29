#ifndef ARENA_TYPE_BUFFER_H
#define ARENA_TYPE_BUFFER_H
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define ARENA_DEFINE_BUFFER(T)                                                 \
  typedef struct {                                                             \
    T *items;                                                                  \
    volatile int64_t length;                                                   \
    int64_t avail;                                                             \
    int64_t capacity;                                                          \
    bool fast;                                                                 \
    volatile bool initialized;                                                 \
  } Arena##T##Buffer;                                                          \
  int arena_##T##_buffer_init(Arena##T##Buffer *buffer);                       \
  int arena_##T##_buffer_init_fast(Arena##T##Buffer *buffer,                   \
                                   int64_t capacity);                          \
  T *arena_##T##_buffer_push(Arena##T##Buffer *buffer, T item);                \
  int arena_##T##_buffer_clear(Arena##T##Buffer *buffer);                      \
  int arena_##T##_buffer_fill(Arena##T##Buffer *buffer, T item,                \
                              int64_t count);                                  \
  int arena_##T##_buffer_copy(Arena##T##Buffer src, Arena##T##Buffer *dest);   \
  int arena_##T##_buffer_popi(Arena##T##Buffer *buffer, int64_t index,         \
                              T *out);                                         \
  int arena_##T##_buffer_remove(Arena##T##Buffer *buffer, int64_t index);      \
  int arena_##T##_buffer_splice_remove(Arena##T##Buffer *buffer,               \
                                       int64_t start, int64_t end);            \
  bool arena_##T##_buffer_is_empty(Arena##T##Buffer buffer);                   \
  int arena_##T##_buffer_back(Arena##T##Buffer buffer, T *out);                \
  int arena_##T##_buffer_pop(Arena##T##Buffer *buffer);

#define ARENA_IMPLEMENT_BUFFER(T)                                              \
  int arena_##T##_buffer_init(Arena##T##Buffer *buffer) {                      \
    if (!buffer)                                                               \
      return 0;                                                                \
    if (buffer->initialized)                                                   \
      return 1;                                                                \
    buffer->initialized = true;                                                \
    buffer->items = 0;                                                         \
    buffer->avail = 0;                                                         \
    buffer->fast = false;                                                      \
    buffer->length = 0;                                                        \
    return 1;                                                                  \
  }                                                                            \
  int arena_##T##_buffer_init_fast(Arena##T##Buffer *buffer,                   \
                                   int64_t capacity) {                         \
    if (!buffer)                                                               \
      return 0;                                                                \
    if (buffer->initialized)                                                   \
      return 1;                                                                \
    buffer->initialized = true;                                                \
    buffer->items = 0;                                                         \
    buffer->length = 0;                                                        \
    buffer->capacity = capacity;                                               \
    buffer->fast = capacity > 0;                                               \
    return 1;                                                                  \
  }                                                                            \
  T *arena_##T##_buffer_push_fast(Arena##T##Buffer *buffer, T item) {          \
    if (!buffer)                                                               \
      return 0;                                                                \
    if (!buffer->initialized)                                                  \
      ARENA_WARNING_RETURN(0, stderr, "Buffer not initialized\n");             \
                                                                               \
    if (buffer->capacity <= 0)                                                 \
      ARENA_WARNING_RETURN(0, stderr, "No capacity!\n");                       \
                                                                               \
    if (buffer->avail <= 0) {                                                  \
      buffer->avail = buffer->capacity;                                        \
                                                                               \
      buffer->items = (T *)realloc(                                            \
          buffer->items, (buffer->length + buffer->capacity) * sizeof(T));     \
                                                                               \
      if (!buffer->items)                                                      \
        ARENA_WARNING_RETURN(0, stderr, "Could not realloc buffer.\n");        \
    }                                                                          \
                                                                               \
    T *ptr = &buffer->items[buffer->length];                                   \
    buffer->items[buffer->length++] = item;                                    \
    buffer->avail -= 1;                                                        \
    if (buffer->avail < 0)                                                     \
      ARENA_WARNING(stderr, "buffer->avail < 0, this should never happen.\n"); \
    return ptr;                                                                \
  }                                                                            \
  T *arena_##T##_buffer_push(Arena##T##Buffer *buffer, T item) {               \
    if (buffer->fast && buffer->capacity > 0)                                  \
      return arena_##T##_buffer_push_fast(buffer, item);                       \
    if (!buffer)                                                               \
      return 0;                                                                \
    if (!buffer->initialized)                                                  \
      ARENA_WARNING_RETURN(0, stderr, "Buffer not initialized\n");             \
    buffer->items =                                                            \
        (T *)realloc(buffer->items, (buffer->length + 1) * sizeof(T));         \
    if (!buffer->items)                                                        \
      ARENA_WARNING_RETURN(0, stderr, "Could not realloc buffer.\n");          \
    buffer->items[buffer->length] = item;                                      \
    T *ptr = &buffer->items[buffer->length];                                   \
    buffer->length++;                                                          \
    return ptr;                                                                \
  }                                                                            \
  int arena_##T##_buffer_copy(Arena##T##Buffer src, Arena##T##Buffer *dest) {  \
    if (!dest)                                                                 \
      return 0;                                                                \
                                                                               \
    if (src.length <= 0 || src.items == 0)                                     \
      return 0;                                                                \
    if (!dest->initialized)                                                    \
      ARENA_WARNING_RETURN(0, stderr, "destination not initialized\n");        \
                                                                               \
    if (!src.initialized)                                                      \
      ARENA_WARNING_RETURN(0, stderr, "source not initialized\n");             \
                                                                               \
    if (dest->length > 0 || dest->items != 0)                                  \
      ARENA_WARNING_RETURN(0, stderr, "Destination is not empty!\n");          \
                                                                               \
    dest->length = src.length;                                                 \
    dest->items = (T *)calloc(src.length, sizeof(T));                          \
                                                                               \
    if (dest->items == 0)                                                      \
      ARENA_WARNING_RETURN(0, stderr, "Failed to allocate memory.\n");         \
                                                                               \
    memcpy(&dest->items[0], &src.items[0], src.length * sizeof(T));            \
                                                                               \
    return dest->length > 0 && dest->items != 0;                               \
  }                                                                            \
  int arena_##T##_buffer_clear(Arena##T##Buffer *buffer) {                     \
    if (!buffer)                                                               \
      return 0;                                                                \
    if (!buffer->initialized)                                                  \
      ARENA_WARNING_RETURN(0, stderr, "Buffer not initialized\n");             \
    if (buffer->items != 0) {                                                  \
      free(buffer->items);                                                     \
      buffer->items = 0;                                                       \
    }                                                                          \
    buffer->items = 0;                                                         \
    buffer->avail = 0;                                                         \
    buffer->length = 0;                                                        \
    return 1;                                                                  \
  }                                                                            \
  int arena_##T##_buffer_fill(Arena##T##Buffer *buffer, T item,                \
                              int64_t count) {                                 \
    if (!buffer)                                                               \
      return 0;                                                                \
    if (count <= 0)                                                            \
      return 0;                                                                \
    if (buffer->fast)                                                          \
      ARENA_WARNING_RETURN(0, stderr, "Cannot fill a fast buffer.\n");         \
    if (!buffer->initialized)                                                  \
      ARENA_WARNING_RETURN(0, stderr, "Buffer not initialized\n");             \
    if (buffer->items != 0) {                                                  \
      arena_##T##_buffer_clear(buffer);                                        \
    }                                                                          \
    buffer->items = (T *)calloc(count, sizeof(T));                             \
    if (buffer->items == 0)                                                    \
      ARENA_WARNING_RETURN(0, stderr, "Failed to allocate memory.\n");         \
    buffer->length = count;                                                    \
    for (int64_t i = 0; i < buffer->length; i++) {                             \
      buffer->items[i] = item;                                                 \
    }                                                                          \
    return 1;                                                                  \
  }                                                                            \
  int arena_##T##_buffer_popi(Arena##T##Buffer *buffer, int64_t index,         \
                              T *out) {                                        \
    if (!buffer)                                                               \
      return 0;                                                                \
    if (!buffer->initialized)                                                  \
      ARENA_WARNING_RETURN(0, stderr, "Buffer not initialized\n");             \
                                                                               \
    if (arena_##T##_buffer_is_empty(*buffer) || index < 0 ||                   \
        index >= buffer->length)                                               \
      return 0;                                                                \
                                                                               \
    *out = buffer->items[index];                                               \
                                                                               \
    if (buffer->length - 1 <= 0) {                                             \
      arena_##T##_buffer_clear(buffer);                                        \
      return 0;                                                                \
    }                                                                          \
                                                                               \
    for (int i = index; i < buffer->length - 1; i++) {                         \
      buffer->items[i] = buffer->items[i + 1];                                 \
    }                                                                          \
                                                                               \
    buffer->items =                                                            \
        (T *)realloc(buffer->items, (buffer->length - 1) * sizeof(T));         \
    buffer->length -= 1;                                                       \
    buffer->length = MAX(0, buffer->length);                                   \
                                                                               \
    return 1;                                                                  \
  }                                                                            \
  int arena_##T##_buffer_remove(Arena##T##Buffer *buffer, int64_t index) {     \
    if (!buffer)                                                               \
      return 0;                                                                \
    if (!buffer->initialized)                                                  \
      ARENA_WARNING_RETURN(0, stderr, "Buffer not initialized\n");             \
                                                                               \
    if (arena_##T##_buffer_is_empty(*buffer) || index < 0 ||                   \
        index >= buffer->length)                                               \
      return 0;                                                                \
                                                                               \
    if (buffer->length - 1 <= 0) {                                             \
      arena_##T##_buffer_clear(buffer);                                        \
      return 0;                                                                \
    }                                                                          \
                                                                               \
    for (int i = index; i < buffer->length - 1; i++) {                         \
      buffer->items[i] = buffer->items[i + 1];                                 \
    }                                                                          \
                                                                               \
    buffer->items =                                                            \
        (T *)realloc(buffer->items, (buffer->length - 1) * sizeof(T));         \
    buffer->length -= 1;                                                       \
    buffer->length = MAX(0, buffer->length);                                   \
                                                                               \
    return 1;                                                                  \
  }                                                                            \
  bool arena_##T##_buffer_is_empty(Arena##T##Buffer buffer) {                  \
    return (buffer.items == 0 || buffer.length <= 0);                          \
  }                                                                            \
  int arena_##T##_buffer_splice_remove(Arena##T##Buffer *buffer,               \
                                       int64_t start, int64_t end) {           \
    if (buffer->length <= 0 || buffer->items == 0)                             \
      return 0;                                                                \
                                                                               \
    Arena##T##Buffer next_buffer = {0};                                        \
    arena_##T##_buffer_init(&next_buffer);                                     \
                                                                               \
    for (int64_t i = start; i < MIN(start + end, buffer->length - 1); i++) {   \
      T v = buffer->items[i % buffer->length];                                 \
      arena_##T##_buffer_push(&next_buffer, v);                                \
    }                                                                          \
                                                                               \
    arena_##T##_buffer_clear(buffer);                                          \
    *buffer = next_buffer;                                                     \
    return 1;                                                                  \
  }                                                                            \
  int arena_##T##_buffer_back(Arena##T##Buffer buffer, T *out) {               \
    if (buffer.length <= 0 || buffer.items == 0)                               \
      return 0;                                                                \
    *out = buffer.items[buffer.length - 1];                                    \
    return 1;                                                                  \
  }                                                                            \
  int arena_##T##_buffer_pop(Arena##T##Buffer *buffer) {                       \
    if (buffer->length <= 0 || buffer->items == 0)                             \
      return 0;                                                                \
                                                                               \
    if (buffer->length - 1 <= 0) {                                             \
      arena_##T##_buffer_clear(buffer);                                        \
      return 1;                                                                \
    }                                                                          \
    buffer->items =                                                            \
        (T *)realloc(buffer->items, (buffer->length - 1) * sizeof(T));         \
    buffer->length -= 1;                                                       \
    return 1;                                                                  \
  }

#endif

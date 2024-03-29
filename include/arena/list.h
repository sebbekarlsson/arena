#ifndef ARENA_TYPE_LIST_H
#define ARENA_TYPE_LIST_H
#include <stdbool.h>
#include <stdint.h>
#include <arena/macros.h>

#define ARENA_DEFINE_LIST(T)                                                    \
  typedef struct ARENA_##T##_LIST_STRUCT {                                      \
    T **items;                                                                 \
    int64_t length;                                                           \
    bool initialized;                                                          \
  } T##List;                                                                   \
  int arena_##T##_list_init(T##List *list);                                     \
  int arena_##T##_list_make_unique(T##List *list);                              \
  T *arena_##T##_list_push(T##List *list, T *item);                             \
  T *arena_##T##_list_push_unique(T##List *list, T *item);                      \
  T* arena_##T##_list_popi(T##List *list, int64_t index);                     \
  int arena_##T##_list_clear(T##List *list);                                    \
  T *arena_##T##_list_remove(T##List *list, T *item);                           \
  bool arena_##T##_list_includes(T##List list, T *item);                        \
  int64_t arena_##T##_list_count(T##List list, T *item);                        \
  int arena_##T##_list_concat(T##List *a, T##List b);                           \
  bool arena_##T##_list_is_empty(T##List list);

#define ARENA_IMPLEMENT_LIST(T)                                                 \
  int arena_##T##_list_init(T##List *list) {                                    \
    if (!list)                                                                 \
      return 0;                                                                \
    if (list->initialized)                                                     \
      return 1;                                                                \
    list->initialized = true;                                                  \
    list->items = 0;                                                           \
    list->length = 0;                                                          \
    return 1;                                                                  \
  }                                                                            \
  int arena_##T##_list_make_unique(T##List *list) {                             \
    if (arena_##T##_list_is_empty(*list))                                       \
      return 1;                                                                \
    for (int64_t i = 0; i < list->length; i++) {                               \
      if (arena_##T##_list_count(*list, list->items[i]) > 1) {                  \
        arena_##T##_list_remove(list, list->items[i]);                          \
      }                                                                        \
    }                                                                          \
    return 1;                                                                  \
  }                                                                            \
  int64_t arena_##T##_list_count(T##List list, T *item) {                       \
    if (arena_##T##_list_is_empty(list))                                        \
      return 0;                                                                \
    int64_t count = 0;                                                         \
    for (int64_t i = 0; i < list.length; i++) {                                \
      if (list.items[i] == item)                                               \
        count++;                                                               \
    }                                                                          \
    return count;                                                              \
  }                                                                            \
  bool arena_##T##_list_includes(T##List list, T *item) {                       \
    if (arena_##T##_list_is_empty(list))                                        \
      return false;                                                            \
    for (int64_t i = 0; i < list.length; i++) {                                \
      if (list.items[i] == item)                                               \
        return true;                                                           \
    }                                                                          \
    return false;                                                              \
  }                                                                            \
  T *arena_##T##_list_remove(T##List *list, T *item) {                          \
    if (!list)                                                                 \
      return item;                                                             \
    if (!list->initialized)                                                    \
      ARENA_WARNING_RETURN(item, stderr, "List not initialized\n");             \
                                                                               \
    if (arena_##T##_list_is_empty(*list))                                       \
      return item;                                                             \
                                                                               \
    if (list->length - 1 <= 0) {                                               \
      arena_##T##_list_clear(list);                                             \
      return item;                                                             \
    }                                                                          \
                                                                               \
    int64_t index = -1;                                                        \
    for (int64_t i = 0; i < list->length; i++) {                               \
      if (list->items[i] == item) {                                            \
        index = i;                                                             \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    if (index <= -1)                                                           \
      return item;                                                             \
                                                                               \
    for (int i = index; i < list->length - 1; i++) {                           \
      list->items[i] = list->items[i + 1];                                     \
    }                                                                          \
                                                                               \
    list->items = (T **)realloc(list->items, (list->length - 1) * sizeof(T));  \
    list->length -= 1;                                                         \
    list->length = MAX(0, list->length);                                       \
                                                                               \
    return item;                                                               \
  }\
  T* arena_##T##_list_popi(T##List *list, int64_t index) { \
    if (!list)                                                               \
      return 0;                                                                \
    if (!list->initialized)                                                  \
      ARENA_WARNING_RETURN(0, stderr, "List not initialized\n");              \
                                                                               \
    if (list->items == 0 || list->length <= 0 || index < 0 || index >= list->length)  \
      return 0;                                                                \
                                                                               \
    T *out = list->items[index];                                               \
                                                                               \
    if (list->length - 1 <= 0) {                                             \
      arena_##T##_list_clear(list);                                         \
      return out;                                                                \
    }                                                                          \
                                                                               \
    for (int i = index; i < MAX(0, list->length - 1); i++) {                         \
      list->items[i] = list->items[i + 1];                                 \
    }                                                                          \
                                                                               \
    list->items =                                                            \
        (T **)realloc(list->items, (list->length - 1) * sizeof(T));         \
    list->length -= 1;                                                       \
    list->length = MAX(0, list->length);                                   \
                                                                               \
    return out;                                                                  \
  }                                                                                  \
  T *arena_##T##_list_push_unique(T##List *list, T *item) {                     \
    if (arena_##T##_list_includes(*list, item))                                 \
      return item;                                                             \
    return arena_##T##_list_push(list, item);                                   \
  }                                                                            \
  T *arena_##T##_list_push(T##List *list, T *item) {                            \
    if (!list)                                                                 \
      return 0;                                                                \
    if (!list->initialized)                                                    \
      ARENA_WARNING_RETURN(0, stderr, "List not initialized\n");                \
    if (!item)                                                                 \
      return 0;                                                                \
    list->items =                                                              \
        (T **)realloc(list->items, (list->length + 1) * sizeof(T *));          \
    if (!list->items)                                                          \
      ARENA_WARNING_RETURN(0, stderr, "Could not realloc list.\n");             \
    list->items[list->length++] = item;                                        \
    return item;                                                               \
  }                                                                            \
  int arena_##T##_list_concat(T##List *a, T##List b) {                          \
    if (!a)                                                                    \
      return 0;                                                                \
    if (b.length <= 0 || b.items == 0)                                         \
      return 0;                                                                \
    for (int64_t i = 0; i < b.length; i++) {                                   \
      if (!arena_##T##_list_includes(*a, b.items[i])) {                         \
        arena_##T##_list_push(a, b.items[i]);                                   \
      }                                                                        \
    }                                                                          \
    return 1;                                                                  \
  }                                                                            \
  int arena_##T##_list_clear(T##List *list) {                                   \
    if (!list)                                                                 \
      return 0;                                                                \
    if (list->items != 0) {                                                    \
      free(list->items);                                                  \
      list->items = 0;                                                         \
    }                                                                          \
    list->length = 0;                                                          \
    return 1;                                                                  \
  }                                                                            \
  bool arena_##T##_list_is_empty(T##List list) {                                \
    return (list.items == 0 || list.length <= 0);                              \
  }

#endif

#ifndef ARENA_TEST_H
#define ARENA_TEST_H
#include <stdbool.h>

#define ARENA_ASSERT(condition) arena_assert(condition, #condition, __func__)


int arena_assert(bool condition, const char* message, const char* funcname);

#endif

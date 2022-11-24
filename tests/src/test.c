#include <arena_test/test.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>


int arena_assert(bool condition, const char* message, const char* funcname) {
  printf("Test: %s\n", funcname);
  printf("%s\n", message);
  if (!condition) {
    fprintf(stderr, "Condition failed: %s\n", message ? message : "?");
    exit(0);
  }

  printf("OK.\n");

  return 1;
}

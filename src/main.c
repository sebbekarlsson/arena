#include <arena/arena.h>
#include <arena/macros.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
  int age;
  char* name;
} Person;

void free_person(Person* person) {
  if (person->name != 0) {
    printf("Goodbye %s\n", person->name);
    free(person->name);
    person->name = 0;
  }
}

int main(int argc, char* argv[]) {

  Arena arena = {0};
  arena_init(&arena, (ArenaConfig){ .page_size = 0, .free_function = (ArenaFreeFunction)free_person });


  for (int i = 0; i < 10000; i++) {

    bool alt = i % 2 == 0;

    ArenaRef ref = {0};
    Person* p = arena_malloc(&arena, sizeof(Person), &ref);


    p->age = i;
    p->name = strdup(alt ? "Sarah": "John");

    arena_free(&arena, ref);

  }

  arena_destroy(&arena);

  return 0;
}

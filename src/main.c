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
  arena_init(&arena, (ArenaConfig){ .item_size = sizeof(Person), .items_per_page = 4, .free_function = (ArenaFreeFunction)free_person });

  int64_t nr_people = 100;

  for (int i = 0; i < nr_people; i++) {

    bool alt = i % 2 == 0;

    ArenaRef ref = {0};
    Person* p = arena_malloc(&arena, &ref);


    p->age = i;
    p->name = strdup(alt ? "Sarah": "John");

   // arena_free(ref);

  }

  ArenaIterator it = {0};
  int64_t count = 0;
  while (arena_iterate(&arena, &it)) {
    Person* p = (Person*)it.ref.ptr;
    printf("in_use: %d\n", it.ref.in_use);
    printf("%p, %ld, %s\n", it.ref.ptr, it.ref.id, p->name);;
    count++;
  }

  printf("%ld\n", count);

  arena_destroy(&arena);

  return 0;
}

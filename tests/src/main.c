#include <stdio.h>
#include <stdlib.h>
#include <arena_test/test.h>
#include <arena/arena.h>
#include <arena/macros.h>
#include <arena/list.h>
#include <assert.h>
#include <string.h>
#include <date/date.h>


typedef struct {
  int age;
  char* name;
  ArenaRef ref;
} Person;

ARENA_DEFINE_LIST(Person);
ARENA_IMPLEMENT_LIST(Person);


void person_free(Person* person) {
  assert(person != 0);
  assert(person->name != 0);
  free(person->name);
  person->name = 0;
}

void test_arena_various_page_size(int64_t count, int64_t items_per_page) {

  Arena arena = {0};
  arena_init(&arena, (ArenaConfig){ .item_size = sizeof(Person), .items_per_page = items_per_page, .free_function = (ArenaFreeFunction)person_free });

  const int nr_items = count;


  PersonList people = {0};
  arena_Person_list_init(&people);

  Person* prev = 0;
  for (int64_t i = 0; i < nr_items; i++) {
    ArenaRef ref = {0};
    Person* p = arena_malloc(&arena, &ref);
    ARENA_ASSERT(p != 0);
    ARENA_ASSERT(ref.data_size > 0);
    ARENA_ASSERT(ref.arena != 0);

    p->ref = ref;

    if (prev != 0) {
      ARENA_ASSERT(prev->name != 0);
    }

    p->age = i;
    p->name = strdup(i % 2 == 0 ? "Sarah" : "John");

    arena_Person_list_push(&people, p);

    prev = p;
  }

  ARENA_ASSERT(people.length == nr_items);


  for (int64_t i = 0; i < people.length; i++) {
    Person* p = people.items[i];
    ARENA_ASSERT(p != 0);
    ARENA_ASSERT(p->name != 0);
    ARENA_ASSERT(strcmp(p->name, i % 2 == 0 ? "Sarah" : "John") == 0);

    ARENA_ASSERT(arena_free(p->ref) != 0);
  }


  arena_destroy(&arena);



  ARENA_ASSERT(arena.next == 0);

  arena_Person_list_clear(&people);
}


void test_arena_randomly_free(int64_t count, int64_t items_per_page) {

  Arena arena = {0};
  arena_init(&arena, (ArenaConfig){ .item_size = sizeof(Person), .items_per_page = items_per_page, .free_function = (ArenaFreeFunction)person_free });

  const int nr_items = count;


  PersonList people = {0};
  arena_Person_list_init(&people);

  Person* prev = 0;
  for (int64_t i = 0; i < nr_items; i++) {
    ArenaRef ref = {0};
    Person* p = arena_malloc(&arena, &ref);
    ARENA_ASSERT(p != 0);
    ARENA_ASSERT(ref.data_size > 0);
    ARENA_ASSERT(ref.arena != 0);

    p->ref = ref;

    if (prev != 0) {
      ARENA_ASSERT(prev->name != 0);
    }

    p->age = i;
    p->name = strdup("John");


    if (i % 2 == 0) {
      arena_Person_list_push(&people, p);
      prev = p;
    } else {
      ARENA_ASSERT(arena_free(p->ref) != 0);
    }

  }

  ARENA_ASSERT(people.length == (nr_items / 2));


  for (int64_t i = 0; i < people.length; i++) {
    Person* p = people.items[i];
    ARENA_ASSERT(p != 0);
    ARENA_ASSERT(p->name != 0);
    ARENA_ASSERT(strcmp(p->name, "John") == 0);
    arena_free(p->ref);
  }


  arena_destroy(&arena);

  ARENA_ASSERT(arena.next == 0);

  arena_Person_list_clear(&people);
}



int main(int argc, char* argv[]) {

  test_arena_various_page_size(10, 16);
  test_arena_various_page_size(100, 16);
  test_arena_various_page_size(1000, 16);
  test_arena_various_page_size(1000, 3);
  test_arena_various_page_size(10000, 3);
  test_arena_various_page_size(10000, 500);
  test_arena_randomly_free(1000, 16);
 // test_arena_various_page_size(1000, 256);
  //test_arena_randomly_free(1000, 256);

  return 0;
}

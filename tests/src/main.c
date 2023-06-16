#include "arena/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <arena_test/test.h>
#include <arena/arena.h>
#include <arena/macros.h>
#include <arena/list.h>
#include <assert.h>
#include <string.h>
#include <date/date.h>
#include <stdlib.h>
#include <time.h>


typedef struct {
  int age;
  char* name;
  ArenaRef ref;
} Person;

ARENA_DEFINE_LIST(Person);
ARENA_IMPLEMENT_LIST(Person);


static void person_free(Person* person) {
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

void test_arena_randomly_reset(int64_t count, int64_t items_per_page) {

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

  ARENA_ASSERT(arena_get_allocation_count(arena) > 0);

  for (int64_t i = 0; i < people.length; i++) {
    Person* p = people.items[i];
    ARENA_ASSERT(p != 0);
    ARENA_ASSERT(p->name != 0);
    ARENA_ASSERT(strcmp(p->name, "John") == 0);
    arena_free(p->ref);
  }

  arena_Person_list_clear(&people);

  arena_reset(&arena);
//  arena_destroy(&arena);

  ARENA_ASSERT(arena.pages == 0);
  ARENA_ASSERT(arena.data != 0);
  ARENA_ASSERT(arena_get_allocation_count(arena) == 0);

  prev = 0;
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

void test_arena_defrag() {

  int count = 2024;
  int64_t items_per_page = 2;
  Arena arena = {0};
  arena_init(&arena, (ArenaConfig){ .item_size = sizeof(Person), .items_per_page = items_per_page, .free_function = (ArenaFreeFunction)person_free });
  const int nr_items = count;



  for (int i = 0; i < (count*4); i++) {
    ArenaRef ref = {0};
    Person* tmp = arena_malloc(&arena, &ref);
    ARENA_ASSERT(tmp != 0);
    tmp->name = strdup("hello\n");

    float k = ((float)rand() / (float)RAND_MAX);
    if (k >= 0.7f) {
      	arena_free(ref);
    }

    if (i % 2 == 0) {
            arena_defrag(&arena);
    }
  }

  arena_defrag(&arena);
  arena_destroy(&arena);
}

static void custom_free_function_with_ptr(void *data, void *user_ptr) {
  ARENA_ASSERT(data != 0);
  ARENA_ASSERT(user_ptr != 0);

  const char* value = (const char*)user_ptr;

  ARENA_ASSERT(strcmp(value, "my_pointer") == 0);


  Person* person = (Person*)data;
  assert(person != 0);
  assert(person->name != 0);
  free(person->name);
  person->name = 0;
}

void test_arena_custom_free_function_ptr(int64_t count, int64_t items_per_page) {


  const char* myptr = "my_pointer";
  
  Arena arena = {0};
  arena_init(&arena, (ArenaConfig){ .item_size = sizeof(Person), .items_per_page = items_per_page, .user_ptr_free = (void*)myptr, .free_function_with_user_ptr = (ArenaFreeFunctionWithUserPtr)custom_free_function_with_ptr });

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


   arena_Person_list_push(&people, p);

  }

  ARENA_ASSERT(people.length == nr_items);


  for (int64_t i = 0; i < people.length; i++) {
    Person* p = people.items[i];
    ARENA_ASSERT(p != 0);
    ARENA_ASSERT(p->name != 0);
    ARENA_ASSERT(strcmp(p->name, "John") == 0);
    ARENA_ASSERT(p->ref.arena != 0);
    arena_free(p->ref);
    
  }

  arena_destroy(&arena);

  ARENA_ASSERT(arena.next == 0);

  arena_Person_list_clear(&people);
}

int main(int argc, char* argv[]) {

  test_arena_defrag();
  test_arena_various_page_size(100, 16);
  test_arena_various_page_size(1000, 16);
  test_arena_various_page_size(1000, 3);
  test_arena_various_page_size(10000, 3);
  test_arena_various_page_size(10000, 500);
  test_arena_randomly_free(1000, 16);
  test_arena_randomly_reset(500, 16);
  test_arena_custom_free_function_ptr(1000, 16);
 // test_arena_various_page_size(1000, 256);
  //test_arena_randomly_free(1000, 256);

  return 0;
}

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

void test_arena_various_page_size(int64_t count, int64_t page_size) {
  printf("Page size: %ld\n", page_size);

  Arena arena = {0};
  arena_init(&arena, (ArenaConfig){ .page_size = page_size, .free_function = (ArenaFreeFunction)person_free });
  ARENA_ASSERT(arena.config.page_size == page_size);

  const int nr_items = count;


  PersonList people = {0};
  arena_Person_list_init(&people);

  Person* prev = 0;
  for (int64_t i = 0; i < nr_items; i++) {
    ArenaRef ref = {0};
    Person* p = arena_malloc(&arena, sizeof(Person), &ref);
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

    arena_free(&arena, p->ref);
  }


  arena_destroy(&arena);

  ARENA_ASSERT(arena.next == 0);

  arena_Person_list_clear(&people);
}


void test_arena_randomly_free(int64_t count, int64_t page_size) {
  printf("Page size: %ld\n", page_size);

  Arena arena = {0};
  arena_init(&arena, (ArenaConfig){ .page_size = page_size, .free_function = (ArenaFreeFunction)person_free });
  ARENA_ASSERT(arena.config.page_size == page_size);

  const int nr_items = count;


  PersonList people = {0};
  arena_Person_list_init(&people);

  Person* prev = 0;
  for (int64_t i = 0; i < nr_items; i++) {
    ArenaRef ref = {0};
    Person* p = arena_malloc(&arena, sizeof(Person), &ref);
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
      arena_free(&arena, p->ref);
    }

  }

  ARENA_ASSERT(people.length == (nr_items / 2));


  for (int64_t i = 0; i < people.length; i++) {
    Person* p = people.items[i];
    ARENA_ASSERT(p != 0);
    ARENA_ASSERT(p->name != 0);
    ARENA_ASSERT(strcmp(p->name, "John") == 0);
    arena_free(&arena, p->ref);
  }


  arena_destroy(&arena);

  ARENA_ASSERT(arena.next == 0);

  arena_Person_list_clear(&people);
}

void test_iter_arena_function(void* user_ptr, Person* person) {
  ARENA_ASSERT(person != 0);
  ARENA_ASSERT(person->name != 0);
  ARENA_ASSERT(strcmp(person->name,  "John") == 0);
}

void test_arena_iter_items_before_free(int64_t nr_items, int64_t items_per_page) {
  Arena arena = {0};
  arena_init(&arena, (ArenaConfig){
    .item_size = sizeof(Person),
    .items_per_page = items_per_page,
    .free_function = (ArenaFreeFunction)person_free
  });

  PersonList people = {0};
  arena_Person_list_init(&people);

  Person* prev = 0;
  for (int64_t i = 0; i < nr_items; i++) {
    ArenaRef ref = {0};
    Person* p = arena_malloc(&arena, sizeof(Person), &ref);
    p->ref = ref;

    ARENA_ASSERT(p != 0);
    ARENA_ASSERT(ref.arena != 0);
    ARENA_ASSERT(ref.data_size > 0);

    p->name = strdup(i % 2 == 0 ? "Sarah" : "John");


    if (prev != 0) {
      ARENA_ASSERT(prev->name != 0);
    }

    arena_Person_list_push(&people, p);

    prev = p;
  }

 // ARENA_ASSERT(arena_iter(&arena, 0, (ArenaIterFunction)test_iter_arena_function) == 1);

    Person* p = 0;
  ArenaIterator it = {0};

  int64_t loops = 0;
  Person* prev_p = 0;
  while ((p = arena_iter(&arena, &it)) != 0) {
    ARENA_ASSERT(p != 0);
    ARENA_ASSERT(p != prev_p);
    ARENA_ASSERT(p->name != 0);
    ARENA_ASSERT(strcmp(p->name, loops % 2 == 0 ? "Sarah" : "John") == 0);
    loops++;
    prev_p = p;
  }

  ARENA_ASSERT(loops == people.length);


  for (int64_t i = 0; i < people.length; i++) {
    Person* p = people.items[i];
    arena_free(&arena, p->ref);
  }

  arena_destroy(&arena);
  ARENA_ASSERT(arena.next == 0);



//  ARENA_ASSERT(arena_iter(&arena, 0, (ArenaIterFunction)test_iter_arena_function) == 0);

  arena_Person_list_clear(&people);
}

void test_arena_iter_items_after_free(int64_t nr_items, int64_t items_per_page) {
  Arena arena = {0};
  arena_init(&arena, (ArenaConfig){
    .item_size = sizeof(Person),
    .items_per_page = items_per_page,
    .free_function = (ArenaFreeFunction)person_free
  });

  Person* prev = 0;
  for (int64_t i = 0; i < nr_items; i++) {
    ArenaRef ref = {0};
    Person* p = arena_malloc(&arena, sizeof(Person), &ref);

    ARENA_ASSERT(p != 0);
    ARENA_ASSERT(ref.arena != 0);
    ARENA_ASSERT(ref.data_size > 0);

    p->name = strdup("John");

    arena_free(&arena, ref);

    if (prev != 0) {
      ARENA_ASSERT(prev->name != 0);
    }

    prev = p;
  }

  Person* p = 0;
  ArenaIterator it = {0};

  int64_t loops = 0;
  while ((p = arena_iter(&arena, &it)) != 0) {
    ARENA_ASSERT(p != 0);
    ARENA_ASSERT(p->name != 0);
    printf("%s\n", p->name);
    loops++;
  }

  ARENA_ASSERT(loops == 0);

 // ARENA_ASSERT(arena_iter(&arena, 0, (ArenaIterFunction)test_iter_arena_function) == 0);

  arena_destroy(&arena);

  ARENA_ASSERT(arena.next == 0);
}


void test_arena_perf(int64_t nr_items, int64_t items_per_page, int64_t nr_samples) {
  Arena arena = {0};
  arena_init(&arena, (ArenaConfig){
    .item_size = sizeof(Person),
    .items_per_page = items_per_page,
    .free_function = (ArenaFreeFunction)person_free
  });

  PersonList people = {0};
  PersonList people_c_malloc = {0};
  arena_Person_list_init(&people);
  arena_Person_list_init(&people_c_malloc);

  for (int64_t i = 0; i < nr_items; i++) {
    ArenaRef ref = {0};
    Person* p = arena_malloc(&arena, sizeof(Person), &ref);
    arena_Person_list_push(&people, p);

    Person* c = (Person*)calloc(1, sizeof(Person));
    arena_Person_list_push(&people_c_malloc, c);
  }


  for (int64_t k = 0; k < 4; k++) {
    {

      double duration = 0.0f;
      for (int64_t j = 0; j < nr_samples; j++) {
        Date before = date_now();

        for (int64_t i = 0; i < people_c_malloc.length; i++) {
          Person* p = people_c_malloc.items[i];
          if (p->age % 2 != 0) {
            p->age = p->age / 2;
          } else {
            p->age = 3 * p->age + 1;
          }
        }


        Date after = date_now();
        Date diff = date_diff(&after, &before);
        duration += diff.milliseconds_static;
      }

      duration /= (double)nr_samples;
      printf("Naive time C malloced: %12.6f\n", duration);
    }



    {
      double duration = 0.0f;
      for (int64_t j = 0; j < nr_samples; j++) {
        Date before = date_now();

        for (int64_t i = 0; i < people.length; i++) {
          Person* p = people.items[i];
          if (p->age % 2 != 0) {
            p->age = p->age / 2;
          } else {
            p->age = 3 * p->age + 1;
          }
        }


        Date after = date_now();
        Date diff = date_diff(&after, &before);
        duration += diff.milliseconds_static;
      }

      duration /= (double)nr_samples;
      printf("Naive time arena malloced: %12.6f\n", duration);
    }


    {
      double duration = 0.0f;
      for (int64_t j = 0; j < nr_samples; j++) {
        Person* p = 0;
        ArenaIterator it = {0};
        Date before = date_now();

        while ((p = arena_iter(&arena, &it)) != 0) {
          if (p->age % 2 != 0) {
            p->age = p->age / 2;
          } else {
            p->age = 3 * p->age + 1;
          }
        }

        Date after = date_now();
        Date diff = date_diff(&after, &before);
        duration += diff.milliseconds_static;
      }
      duration /= (double)nr_samples;
      printf("Iterator time: %12.6f\n", duration);
    }
  }

  arena_destroy(&arena);
  arena_Person_list_clear(&people);
  arena_Person_list_clear(&people_c_malloc);
}


#define ONLY_CHECK_PERF false

int main(int argc, char* argv[]) {

  #if ONLY_CHECK_PERF == false
  test_arena_various_page_size(1000, 16);
  test_arena_various_page_size(1000, 256);
  test_arena_randomly_free(1000, 16);
  test_arena_randomly_free(1000, 256);
  test_arena_iter_items_before_free(100, 5);
  test_arena_iter_items_after_free(100, 5);
  test_arena_iter_items_before_free(1000, 8);
  test_arena_iter_items_after_free(1000, 8);
  #endif

  #if 0
  {
    test_arena_perf(100, 900, 64);
    test_arena_perf(20000, 900, 64);
    test_arena_perf(100, 900, 64);
    test_arena_perf(3, 900, 64);
  }
  #endif

  return 0;
}

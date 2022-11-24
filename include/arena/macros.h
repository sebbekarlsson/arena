#ifndef ARENA_MACROS_H
#define ARENA_MACROS_H


#ifndef NEW
#define NEW(T) ((T *)(calloc(1, sizeof(T))))
#endif

#ifndef OR
#define OR(a, b) (a ? a : b)
#endif

#ifndef MIN
#define MIN(a, b) (a < b ? a : b)
#endif

#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif


#define ARENA_ALIGN_UP(x, n) (((x) + (n) - 1) & ~((n) - 1))

#define ARENA_IS_POWER_OF_2(x) ((x != 0) && ((x & (x - 1)) == 0))


#define ARENA_WARNING(...)                                                      \
  {                                                                            \
    printf("(ARENA)(Warning)(%s): \n", __func__);   \
    fprintf(__VA_ARGS__);                                                      \
  }
#define ARENA_WARNING_RETURN(ret, ...)                                          \
  {                                                                            \
    printf("\n****\n");                                                        \
    printf("(ARENA)(Warning)(%s): \n", __func__);   \
    fprintf(__VA_ARGS__);                                                      \
    printf("\n****\n");                                                        \
    return ret;                                                                \
  }

#endif

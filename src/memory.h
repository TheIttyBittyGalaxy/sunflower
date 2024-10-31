#ifndef MEMORY_H
#define MEMORY_H

#define NEW(type) (type *)malloc(sizeof(type));

#define INIT_ARRAY(array) \
    array = NULL;         \
    array##_count = 0;

#define EXTEND_ARRAY(array, type)                                               \
    (                                                                           \
        (array = (type *)((array##_count++ == 0)                                \
                              ? malloc(sizeof(type))                            \
                              : realloc(array, sizeof(type) * array##_count))), \
        (array + array##_count - 1))

#endif
#ifndef SUB_STRING_H
#define SUB_STRING_H

#include <stdlib.h>

typedef struct
{
    const char *str;
    size_t len;
} sub_string;

// CLEANUP: Create are_substrings_equal(sub_string a, sub_string b)

#define NULL_SUB_STRING ((sub_string){.str = NULL, .len = 0})

#endif
#ifndef SUB_STRING_H
#define SUB_STRING_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct
{
    const char *str;
    size_t len;
} sub_string;

bool substrings_match(sub_string a, sub_string b);
bool substring_is(sub_string sub, const char *string_literal);

#define NULL_SUB_STRING ((sub_string){.str = NULL, .len = 0})

#endif
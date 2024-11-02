#include <string.h>

#include "sub_string.h"

bool substrings_match(sub_string a, sub_string b)
{
    return a.len == b.len && (strncmp(a.str, b.str, a.len) == 0);
}

bool substring_is(sub_string sub, const char *string_literal)
{
    if (sub.len != strlen(string_literal))
        return false;

    return strncmp(sub.str, string_literal, sub.len) == 0;
}
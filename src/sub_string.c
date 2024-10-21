#include <string.h>

#include "sub_string.h"

bool substrings_match(sub_string a, sub_string b)
{
    return a.len == b.len && (strncmp(a.str, b.str, a.len) == 0);
}
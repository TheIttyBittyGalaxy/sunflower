#ifndef PROGRAM_H
#define PROGRAM_H

#include <stdlib.h>

// Node
typedef struct
{
    // TODO: Replace these with a single "sub string" struct
    const char *name;
    size_t name_len;

    // TODO: Replace these with a single "sub string" struct
    const char *property_name;
    size_t property_name_len;

    // TODO: Replace these with a single "sub string" struct
    const char *property_kind_name;
    size_t property_kind_name_len;
} Node;

// Program
typedef struct
{
    Node node;
} Program;

// Create program
Program *create_program();

#endif
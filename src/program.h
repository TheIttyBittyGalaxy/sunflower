#ifndef PROGRAM_H
#define PROGRAM_H

#include <stdlib.h>

#include "sub_string.h"

// Macros

// CLEANUP: Move these marcos to their own source file

#define NEW(type) (type *)malloc(sizeof(type));

#define INIT_ARRAY(array) \
    array = NULL;         \
    array##_count = 0;

#define EXTEND_ARRAY(array, type)                                         \
    ({                                                                    \
        if (array##_count++ == 0)                                         \
            array = (type *)malloc(sizeof(type));                         \
        else                                                              \
            array = (type *)realloc(array, sizeof(type) * array##_count); \
        (array + array##_count - 1);                                      \
    })

// Property
typedef struct
{
    sub_string name;
    sub_string kind_name;
} Property;

// Node
typedef struct
{
    sub_string name;
    Property *properties;
    size_t properties_count;
} Node;

// Program
typedef struct
{
    Node *nodes;
    size_t nodes_count;
} Program;

// Strings & printing
void print_program(const Program *program);
void print_node(const Node *node);

#endif
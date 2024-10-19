#ifndef PROGRAM_H
#define PROGRAM_H

#include <stdlib.h>

#include "sub_string.h"

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
    size_t property_count;
} Node;

// Program
typedef struct
{
    Node node;
} Program;

// Strings & printing
void print_program(const Program *program);
void print_node(const Node *node);

#endif
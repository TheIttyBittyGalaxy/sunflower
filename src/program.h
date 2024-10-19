#ifndef PROGRAM_H
#define PROGRAM_H

#include <stdlib.h>

#include "sub_string.h"

// Node
typedef struct
{
    sub_string name;
    sub_string property_name;
    sub_string property_kind_name;
} Node;

// Program
typedef struct
{
    Node node;
} Program;

// Create program
Program *create_program();

#endif
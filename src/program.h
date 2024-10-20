#ifndef PROGRAM_H
#define PROGRAM_H

#include <stdint.h>
#include <stdlib.h>

#include "expression.h"
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

// Variable
// Variable is declared differently here as it is forward declared in "expression.h"
// CLEANUP: Is there a better way of doing this?
struct Variable
{
    sub_string name;
    sub_string node_name;
};

// Rule
typedef struct
{
    Variable *variables;
    size_t variables_count;
    Expression *expression;
} Rule;

// Program
typedef struct
{
    Node *nodes;
    size_t nodes_count;
    Rule *rules;
    size_t rules_count;
} Program;

// Strings & printing
void print_program(const Program *program);
void print_node(const Node *node);
void print_rule(const Rule *rule);

#endif
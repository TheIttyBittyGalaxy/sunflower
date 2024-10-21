#ifndef PROGRAM_H
#define PROGRAM_H

#include <stdint.h>
#include <stdlib.h>

#include "expression.h"
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
    size_t properties_count;
} Node;

// Placeholder
// Placeholder is declared differently here as it is forward declared in "expression.h"
// CLEANUP: Is there a better way of doing this?
struct Placeholder
{
    sub_string name;
    sub_string node_name;
};

// Rule
typedef struct
{
    Placeholder *placeholders;
    size_t placeholders_count;
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
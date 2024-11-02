#ifndef PROGRAM_H
#define PROGRAM_H

#include <stdint.h>
#include <stdlib.h>

#include "expression.h"
#include "sub_string.h"

// Forward declarations
typedef struct Property Property;
typedef struct Node Node;

// Property
struct Property
{
    sub_string name;
    sub_string type_name;
    ExprType type;
};

// Node
// Node is declared differently here as it is forward declared in "expression.h"
// CLEANUP: Is there a better way of doing this?
// CLEANUP: Is there a better name for this struct than "Node"? Something slightly more specific/less abstract might be helpful?
struct Node
{
    sub_string name;
    Property *properties;
    size_t properties_count;
};

// Placeholder
// Placeholder is declared differently here as it is forward declared in "expression.h"
// CLEANUP: Is there a better way of doing this?
struct Placeholder
{
    sub_string name;
    sub_string type_name;
    ExprType type;
    size_t index; // The placeholder's index in the rule's array of placeholders
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
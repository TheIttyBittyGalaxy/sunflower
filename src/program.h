#ifndef PROGRAM_H
#define PROGRAM_H

#include <stdint.h>
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

// Variable
typedef struct
{
    sub_string name;
    sub_string node_name;
} Variable;

// Expression
typedef enum
{
    UNRESOLVED_NAME,
    PROPERTY_NAME,
    NUMBER_LITERAL,
    VARIABLE,
    BIN_OP,

    ARC_VALUE,
} ExpressionKind;

typedef enum
{
    INDEX,

    LESS_THAN,
    MORE_THAN,
    LESS_THAN_OR_EQUAL,
    MORE_THAN_OR_EQUAL,

    EQUAL_TO,
} Operation;

typedef struct Expression Expression;

struct Expression
{
    ExpressionKind kind;
    union
    {
        struct // UNRESOLVED_NAME / PROPERTY_NAME
        {
            sub_string name;
        };
        struct // NUMBER_LITERAL
        {
            int number;
        };
        struct // VARIABLE
        {
            Variable *var;
        };
        struct // BIN_OP
        {
            Operation op;
            Expression *lhs;
            Expression *rhs;
        };
    };
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

// Precedence
typedef size_t Precedence;
const size_t MIN_PRECEDENCE = SIZE_MAX;
size_t precedence_of(Operation op);

// Strings & printing
const char *expression_kind_string(ExpressionKind kind);
const char *operation_string(Operation operation);
void print_program(const Program *program);
void print_node(const Node *node);
void print_rule(const Rule *rule);
void print_expression(const Expression *expr);

#endif
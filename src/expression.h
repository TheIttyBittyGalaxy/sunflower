#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <stdint.h>
#include <stdbool.h>

#include "sub_string.h"

// Forward declarations for Program nodes
// (We cannot include "program.h" as this would create a cyclic include)
typedef struct Placeholder Placeholder;
typedef struct Node Node;

// TypePrimitive
typedef enum
{
    TYPE_PRIMITIVE__INVALID,
    TYPE_PRIMITIVE__NUM,
    TYPE_PRIMITIVE__BOOL,
    TYPE_PRIMITIVE__NODE
} TypePrimitive;

// ExprType
typedef struct
{
    TypePrimitive type;
    Node *node;
} ExprType;

// ExprVariant
typedef enum
{
    EXPR_VARIANT__UNRESOLVED_NAME,

    EXPR_VARIANT__PROPERTY_NAME,
    EXPR_VARIANT__NUMBER_LITERAL,
    EXPR_VARIANT__PLACEHOLDER,
    EXPR_VARIANT__BIN_OP,

    EXPR_VARIANT__VARIABLE_REFERENCE_INDEX,
} ExprVariant;

// Operation
typedef enum
{
    OPERATION__INDEX,

    OPERATION__MUL,
    OPERATION__DIV,
    OPERATION__ADD,
    OPERATION__SUB,

    OPERATION__LESS_THAN,
    OPERATION__MORE_THAN,
    OPERATION__LESS_THAN_OR_EQUAL,
    OPERATION__MORE_THAN_OR_EQUAL,

    OPERATION__EQUAL_TO,
    OPERATION__NOT_EQUAL_TO,

    OPERATION__LOGICAL_AND,
    OPERATION__LOGICAL_OR,
} Operation;

// ExprValue
typedef struct
{
    TypePrimitive type;
    union
    {
        int num;
        bool boolean;
    };
} ExprValue;

// Expression
typedef struct Expression Expression;

struct Expression
{
    ExprVariant variant;
    union
    {
        struct // UNRESOLVED_NAME / PROPERTY_NAME
        {
            sub_string name;
        };
        // TODO: Generalise this to be a literal for any possible `ExprValue`
        struct // NUMBER_LITERAL
        {
            int number;
        };
        struct // PLACEHOLDER
        {
            Placeholder *placeholder;
        };
        struct // BIN_OP
        {
            Operation op;
            Expression *lhs;
            Expression *rhs;
            size_t index_property_index;
        };
        struct // VARIABLE_REFERENCE_INDEX
        {
            size_t variable_reference_index;
        };
    };
};

// Types
ExprType deduce_type_of(Expression *expr);

// Precedence
typedef size_t Precedence;
static const size_t MIN_PRECEDENCE = SIZE_MAX;
size_t precedence_of(Operation op);

// Strings & printing
const char *type_primitive_string(TypePrimitive primitive);
const char *expr_variant_string(ExprVariant variant);
const char *operation_string(Operation operation);
void print_expression(const Expression *expr);

#endif
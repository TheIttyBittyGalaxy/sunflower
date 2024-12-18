#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <stdint.h>
#include <stdbool.h>

#include "sub_string.h"

// Forward declarations for Program nodes
// (We cannot include "program.h" as this would create a cyclic include)
typedef struct Placeholder Placeholder;
typedef struct Node Node;
typedef struct Rule Rule;

// TypePrimitive
typedef enum
{
    TYPE_PRIMITIVE__INVALID,
    TYPE_PRIMITIVE__UNRESOLVED,

    TYPE_PRIMITIVE__NUMBER,
    TYPE_PRIMITIVE__BOOL,
    TYPE_PRIMITIVE__NODE
} TypePrimitive;

// ExprType
typedef struct
{
    TypePrimitive primitive;
    Node *node;
} ExprType;

// ExprVariant
typedef enum
{
    EXPR_VARIANT__UNRESOLVED_NAME,

    EXPR_VARIANT__LITERAL,
    EXPR_VARIANT__PLACEHOLDER_VALUE,
    EXPR_VARIANT__BIN_OP, // CLEANUP: Rename to `BINARY_OPERATION`
    EXPR_VARIANT__PROPERTY_ACCESS,

    EXPR_VARIANT__VARIABLE_REFERENCE_INDEX,
    EXPR_VARIANT__INSTANCE_REFERENCE_INDEX,
} ExprVariant;

// Operation
typedef enum
{
    OPERATION__ACCESS,

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
    TypePrimitive type_primitive;
    union
    {
        int number;
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
        struct // UNRESOLVED_NAME
        {
            sub_string name;
        };
        struct // LITERAL
        {
            ExprValue literal_value;
        };
        struct // PLACEHOLDER_VALUE
        {
            size_t placeholder_value_index;
        };
        struct // BIN_OP
        {
            Expression *lhs;
            Expression *rhs;
            Operation op;
        };
        struct // PROPERTY_ACCESS
        {
            sub_string property_name;
            size_t access_placeholder_index;
            size_t access_property_offset;
        };
        struct // VARIABLE_REFERENCE_INDEX
        {
            size_t variable_reference_index;
        };
        struct // INSTANCE_REFERENCE_INDEX
        {
            size_t instance_reference_index;
        };
    };
};

// Types
ExprType deduce_type_of(Rule *rule, Expression *expr);

// Precedence
typedef size_t Precedence;
size_t precedence_of(Operation op);

// Strings & printing
const char *type_primitive_string(TypePrimitive primitive);
const char *expr_variant_string(ExprVariant variant);
const char *operation_string(Operation operation);
void print_expr_type(const ExprType type);
void print_expr_value(const ExprValue value);
void print_expression(const Expression *expr);

#endif
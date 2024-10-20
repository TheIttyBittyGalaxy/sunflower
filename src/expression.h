#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <stdint.h>
#include <stdbool.h>

#include "sub_string.h"

// ExpressionKind
// CLEANUP: Prefix these with "EXPRESSION_KIND"
typedef enum
{
    UNRESOLVED_NAME,
    PROPERTY_NAME,
    NUMBER_LITERAL,
    VARIABLE,
    BIN_OP,

    ARC_VALUE, // CLEANUP: Figure out a better name for this? "ARC_VALUE" implies "the value of the arc", which does't make sense!
} ExpressionKind;

// Operation
// CLEANUP: Prefix these with "OPERATION"
typedef enum
{
    INDEX,

    LESS_THAN,
    MORE_THAN,
    LESS_THAN_OR_EQUAL,
    MORE_THAN_OR_EQUAL,

    EQUAL_TO,
} Operation;

// ValueKind
// CLEANUP: Rename this to something that won't be confused with values in the map itself?
// CLEANUP: Prefix these enums
typedef enum
{
    NUM_VAL,
    BOOL_VAL
} ValueKind;

// Value
// CLEANUP: Rename this to something that won't be confused with values in the map itself?
typedef struct
{
    ValueKind kind;
    union
    {
        int num;
        bool boolean;
    };
} Value;

// Forward declarations for references to Program nodes
// (We cannot include "program.h" as this would create a cyclic include)
typedef struct Variable Variable;

// Expression
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
        // ARC_VALUE (no attributes required)
    };
};

// Precedence
typedef size_t Precedence;
const size_t MIN_PRECEDENCE = SIZE_MAX;
size_t precedence_of(Operation op);

// Strings & printing
const char *expression_kind_string(ExpressionKind kind);
const char *operation_string(Operation operation);
const char *value_kind_string(ValueKind kind);
void print_expression(const Expression *expr);

#endif
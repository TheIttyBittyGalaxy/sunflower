#include <stdio.h>

#include "expression.h"
#include "program.h"

// Precedence
size_t precedence_of(Operation op)
{
    switch (op)
    {
    case INDEX:
        return 1;

    case LESS_THAN:
        return 2;
    case MORE_THAN:
        return 2;
    case LESS_THAN_OR_EQUAL:
        return 2;
    case MORE_THAN_OR_EQUAL:
        return 2;

    case EQUAL_TO:
        return 3;
    default:
    {
        fprintf(stderr, "Internal error: Could not determine precedence of %s operation", operation_string(op));
        exit(EXIT_FAILURE);
    }
    }
}

// Strings & printing
const char *expression_kind_string(ExpressionKind kind)
{
    if (kind == UNRESOLVED_NAME)
        return "UNRESOLVED_NAME";
    if (kind == PROPERTY_NAME)
        return "PROPERTY_NAME";
    if (kind == VARIABLE)
        return "VARIABLE";
    if (kind == BIN_OP)
        return "BIN_OP";

    if (kind == ARC_VALUE)
        return "ARC_VALUE";

    return "INVALID_EXPRESSION_KIND";
}

const char *operation_string(Operation operation)
{
    if (operation == INDEX)
        return "INDEX";
    if (operation == EQUAL_TO)
        return "EQUAL_TO";
    if (operation == LESS_THAN)
        return "LESS_THAN";
    if (operation == MORE_THAN)
        return "MORE_THAN";
    if (operation == LESS_THAN_OR_EQUAL)
        return "LESS_THAN_OR_EQUAL";
    if (operation == MORE_THAN_OR_EQUAL)
        return "MORE_THAN_OR_EQUAL";

    return "INVALID_OPERATION";
}

const char *value_kind_string(ValueKind kind)
{
    if (kind == NUM_VAL)
        return "NUM_VAL";
    if (kind == BOOL_VAL)
        return "BOOL_VAL";

    return "INVALID_VALUE_KIND";
}

void print_expression(const Expression *expr)
{
    switch (expr->kind)
    {
    case UNRESOLVED_NAME:
    {
        printf("%.*s?", expr->name.len, expr->name.str);
        break;
    }

    case PROPERTY_NAME:
    {
        printf("%.*s", expr->name.len, expr->name.str);
        break;
    }

    case NUMBER_LITERAL:
    {
        printf("%d", expr->number);
        break;
    }

    case VARIABLE:
    {
        printf("!%.*s", expr->var->name.len, expr->var->name.str);
        break;
    }

    case BIN_OP:
    {
        char op = (".<>≤≥=")[expr->op];
        if (precedence_of(expr->op) == 1)
        {
            print_expression(expr->lhs);
            putchar(op);
            print_expression(expr->rhs);
        }
        else
        {
            putchar('(');
            print_expression(expr->lhs);
            printf(" %c ", op);
            print_expression(expr->rhs);
            putchar(')');
        }
        break;
    }

    case ARC_VALUE:
    {
        printf("[%d]", expr->index);
        break;
    }

    default:
    {
        fprintf(stderr, "Internal error: Could print %s Expression", expression_kind_string(expr->kind));
        exit(EXIT_FAILURE);
    }
    }
}
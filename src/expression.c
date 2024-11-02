#include <stdio.h>

#include "expression.h"
#include "program.h"

// Types
ExprType deduce_type_of(Expression *expr)
{
    switch (expr->variant)
    {
    case EXPR_VARIANT__LITERAL:
        return (ExprType){.primitive = expr->literal_value.type_primitive, .node = NULL};

    case EXPR_VARIANT__PLACEHOLDER:
        return expr->placeholder->type;

    case EXPR_VARIANT__BIN_OP:
    {
        switch (expr->op)
        {
        case OPERATION__ACCESS:
        {
            fprintf(stderr, "Internal error: Attempt to get type of unresolved INDEX BIN_OP\n");
            print_expression(expr);
            exit(EXIT_FAILURE);
        }

        case OPERATION__MUL:
        case OPERATION__DIV:
        case OPERATION__ADD:
        case OPERATION__SUB:
            return (ExprType){.primitive = TYPE_PRIMITIVE__NUMBER, .node = NULL};

        case OPERATION__LESS_THAN:
        case OPERATION__MORE_THAN:
        case OPERATION__LESS_THAN_OR_EQUAL:
        case OPERATION__MORE_THAN_OR_EQUAL:
        case OPERATION__EQUAL_TO:
        case OPERATION__NOT_EQUAL_TO:
        case OPERATION__LOGICAL_AND:
        case OPERATION__LOGICAL_OR:
            return (ExprType){.primitive = TYPE_PRIMITIVE__BOOL, .node = NULL};
        }
    }

    case EXPR_VARIANT__PROPERTY_ACCESS:
    {
        Node *node = expr->subject->placeholder->type.node;
        Property *property = node->properties + expr->property_offset;
        return property->type;
    }

    default:
    {
        fprintf(stderr, "Internal error: Could not deduce type of %s Expression\n", expr_variant_string(expr->variant));
        print_expression(expr);
        exit(EXIT_FAILURE);
    }
    }
}

// Precedence
size_t precedence_of(Operation op)
{
    switch (op)
    {
    case OPERATION__ACCESS:
        return 1;

    case OPERATION__MUL:
    case OPERATION__DIV:
        return 2;

    case OPERATION__ADD:
    case OPERATION__SUB:
        return 3;

    case OPERATION__LESS_THAN:
    case OPERATION__MORE_THAN:
    case OPERATION__LESS_THAN_OR_EQUAL:
    case OPERATION__MORE_THAN_OR_EQUAL:
        return 4;

    case OPERATION__EQUAL_TO:
    case OPERATION__NOT_EQUAL_TO:
        return 5;

    case OPERATION__LOGICAL_AND:
    case OPERATION__LOGICAL_OR:
        return 6;

    default:
    {
        fprintf(stderr, "Internal error: Could not determine precedence of %s operation", operation_string(op));

        exit(EXIT_FAILURE);
    }
    }
}

// Strings & printing
const char *type_primitive_string(TypePrimitive primitive)
{
    if (primitive == TYPE_PRIMITIVE__INVALID)
        return "INVALID";
    if (primitive == TYPE_PRIMITIVE__UNRESOLVED)
        return "UNRESOLVED";

    if (primitive == TYPE_PRIMITIVE__NUMBER)
        return "NUM";
    if (primitive == TYPE_PRIMITIVE__BOOL)
        return "BOOL";
    if (primitive == TYPE_PRIMITIVE__NODE)
        return "NODE";

    return "<INVALID TYPE_PRIMITIVE>";
}

const char *expr_variant_string(ExprVariant variant)
{
    if (variant == EXPR_VARIANT__UNRESOLVED_NAME)
        return "UNRESOLVED_NAME";

    if (variant == EXPR_VARIANT__LITERAL)
        return "LITERAL";
    if (variant == EXPR_VARIANT__PLACEHOLDER)
        return "PLACEHOLDER";
    if (variant == EXPR_VARIANT__BIN_OP)
        return "BIN_OP";
    if (variant == EXPR_VARIANT__PROPERTY_ACCESS)
        return "PROPERTY_ACCESS";

    if (variant == EXPR_VARIANT__VARIABLE_REFERENCE_INDEX)
        return "VARIABLE_REFERENCE_INDEX";

    return "<INVALID EXPR_VARIANT>";
}

const char *operation_string(Operation operation)
{
    if (operation == OPERATION__ACCESS)
        return "ACCESS";

    if (operation == OPERATION__MUL)
        return "MUL";
    if (operation == OPERATION__DIV)
        return "DIV";
    if (operation == OPERATION__ADD)
        return "ADD";
    if (operation == OPERATION__SUB)
        return "SUB";

    if (operation == OPERATION__LESS_THAN)
        return "LESS_THAN";
    if (operation == OPERATION__MORE_THAN)
        return "MORE_THAN";
    if (operation == OPERATION__LESS_THAN_OR_EQUAL)
        return "LESS_THAN_OR_EQUAL";
    if (operation == OPERATION__MORE_THAN_OR_EQUAL)
        return "MORE_THAN_OR_EQUAL";

    if (operation == OPERATION__EQUAL_TO)
        return "EQUAL_TO";
    if (operation == OPERATION__NOT_EQUAL_TO)
        return "NOT_EQUAL_TO";

    if (operation == OPERATION__LOGICAL_AND)
        return "LOGICAL_AND";
    if (operation == OPERATION__LOGICAL_OR)
        return "LOGICAL_OR";

    return "<INVALID OPERATION>";
}

void print_expr_value(const ExprValue value)
{
    if (value.type_primitive == TYPE_PRIMITIVE__INVALID)
        printf("<invalid value>");
    else if (value.type_primitive == TYPE_PRIMITIVE__NUMBER)
        printf("%d", value.number);
    else if (value.type_primitive == TYPE_PRIMITIVE__BOOL)
        printf(value.boolean ? "true" : "false");
    else if (value.type_primitive == TYPE_PRIMITIVE__NODE)
        printf("<node value>");
    else
        printf("<VALUE WITH INVALID TYPE PRIMITIVE>");
}

void print_expression(const Expression *expr)
{
    switch (expr->variant)
    {
    case EXPR_VARIANT__UNRESOLVED_NAME:
    {
        printf("%.*s?", expr->name.len, expr->name.str);
        break;
    }

    case EXPR_VARIANT__LITERAL:
    {
        print_expr_value(expr->literal_value);
        break;
    }

    case EXPR_VARIANT__PLACEHOLDER:
    {
        printf("!%.*s", expr->placeholder->name.len, expr->placeholder->name.str);
        break;
    }

    case EXPR_VARIANT__BIN_OP:
    {
        const char *op = (const char *[]){".", "*", "/", "+", "-", "<", ">", "≤", "≥", "=", "≠", "AND", "OR"}[expr->op];
        if (precedence_of(expr->op) == 1)
        {
            print_expression(expr->lhs);
            printf(op);
            print_expression(expr->rhs);
        }
        else
        {
            putchar('(');
            print_expression(expr->lhs);
            printf(" %s ", op);
            print_expression(expr->rhs);
            putchar(')');
        }
        break;
    }

    case EXPR_VARIANT__PROPERTY_ACCESS:
    {
        printf("@[%d:%d]", expr->placeholder_index, expr->property_offset);
        break;
    }

    case EXPR_VARIANT__VARIABLE_REFERENCE_INDEX:
    {
        printf("[%d]", expr->variable_reference_index);
        break;
    }

    default:
    {
        fprintf(stderr, "Internal error: Could print %s Expression", expr_variant_string(expr->variant));
        exit(EXIT_FAILURE);
    }
    }
}
#include <stdio.h>

#include "expression.h"
#include "program.h"

// Types
TypeInfo deduce_type_of(Expression *expr)
{
    switch (expr->kind)
    {
    case NUMBER_LITERAL:
        return (TypeInfo){.type = TYPE_NUM, .node_type = NULL};

    case PLACEHOLDER:
        return (TypeInfo){.type = TYPE_NODE, .node_type = expr->placeholder->node_type};

    case BIN_OP:
    {
        switch (expr->op)
        {
        case INDEX:
        {
            Node *node = expr->lhs->placeholder->node_type;
            sub_string field_name = expr->rhs->name;
            Property *property = NULL;

            for (size_t i = 0; i < node->properties_count; i++)
            {
                property = node->properties + i;
                if (substrings_match(field_name, property->name))
                    break;
            }

            if (!property)
            {
                fprintf(stderr, "Internal error: Could not deduce type of INDEX Expression - Could not find the property of the corresponding node type\n");
                print_expression(expr);
                exit(EXIT_FAILURE);
            }

            return (TypeInfo){.type = property->type, .node_type = property->node_type};
        }

        case MUL:
        case DIV:
        case ADD:
        case SUB:
            return (TypeInfo){.type = TYPE_NUM, .node_type = NULL};

        case LESS_THAN:
        case MORE_THAN:
        case LESS_THAN_OR_EQUAL:
        case MORE_THAN_OR_EQUAL:
        case EQUAL_TO:
        case NOT_EQUAL_TO:
        case LOGICAL_AND:
        case LOGICAL_OR:
            return (TypeInfo){.type = TYPE_BOOL, .node_type = NULL};
        }
    }

    default:
    {
        fprintf(stderr, "Internal error: Could not deduce type of %s Expression\n", expression_kind_string(expr->kind));
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
    case INDEX:
        return 1;

    case MUL:
    case DIV:
        return 2;

    case ADD:
    case SUB:
        return 3;

    case LESS_THAN:
    case MORE_THAN:
    case LESS_THAN_OR_EQUAL:
    case MORE_THAN_OR_EQUAL:
        return 4;

    case EQUAL_TO:
    case NOT_EQUAL_TO:
        return 5;

    case LOGICAL_AND:
    case LOGICAL_OR:
        return 6;

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
    if (kind == NUMBER_LITERAL)
        return "NUMBER_LITERAL";
    if (kind == PLACEHOLDER)
        return "PLACEHOLDER";
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

    if (operation == MUL)
        return "MUL";
    if (operation == DIV)
        return "DIV";
    if (operation == ADD)
        return "ADD";
    if (operation == SUB)
        return "SUB";

    if (operation == LESS_THAN)
        return "LESS_THAN";
    if (operation == MORE_THAN)
        return "MORE_THAN";
    if (operation == LESS_THAN_OR_EQUAL)
        return "LESS_THAN_OR_EQUAL";
    if (operation == MORE_THAN_OR_EQUAL)
        return "MORE_THAN_OR_EQUAL";

    if (operation == EQUAL_TO)
        return "EQUAL_TO";
    if (operation == NOT_EQUAL_TO)
        return "NOT_EQUAL_TO";

    if (operation == LOGICAL_AND)
        return "LOGICAL_AND";
    if (operation == LOGICAL_OR)
        return "LOGICAL_OR";

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

    case PLACEHOLDER:
    {
        printf("!%.*s", expr->placeholder->name.len, expr->placeholder->name.str);
        break;
    }

    case BIN_OP:
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
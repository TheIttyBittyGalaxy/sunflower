#include <stdio.h>
#include <stdlib.h>

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

void print_program(const Program *program)
{
    printf("PROGRAM\n");

    printf("\tNODES:\n");
    for (size_t i = 0; i < program->nodes_count; i++)
        print_node(program->nodes + i);

    printf("\tRULES:\n");
    for (size_t i = 0; i < program->rules_count; i++)
        print_rule(program->rules + i);
}

void print_node(const Node *node)
{
    printf("\t\t%.*s { ", node->name.len, node->name.str);
    for (size_t i = 0; i < node->properties_count; i++)
    {
        if (i > 0)
            printf(", ");
        Property p = node->properties[i];
        printf("%.*s: %.*s", p.name.len, p.name.str, p.kind_name.len, p.kind_name.str);
    }
    printf(" }\n");
}

void print_rule(const Rule *rule)
{
    printf("\t\t");
    for (size_t i = 0; i < rule->variables_count; i++)
    {
        if (i > 0)
            printf(", ");
        Variable var = rule->variables[i];
        printf("%.*s %.*s", var.node_name.len, var.node_name.str, var.name.len, var.name.str);
    }
    printf(" -> ");
    print_expression(rule->expression);
    printf("\n");
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

    default:
    {
        fprintf(stderr, "Internal error: Could print %s Expression", expression_kind_string(expr->kind));
        exit(EXIT_FAILURE);
    }
    }
}
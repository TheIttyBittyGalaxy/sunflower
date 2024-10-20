#include <stdio.h>
#include <string.h>

#include "resolve.h"

// Resolve methods
void resolve_program(Program *program);
void resolve_node(Program *program, Node *node);
void resolve_rule(Program *program, Rule *rule);
Expression *resolve_expression(Program *program, Rule *rule, Expression *expr);

// Resolve
void resolve(Program *program)
{
    resolve_program(program);
}

void resolve_program(Program *program)
{
    for (size_t i = 0; i < program->nodes_count; i++)
        resolve_node(program, program->nodes + i);

    for (size_t i = 0; i < program->rules_count; i++)
        resolve_rule(program, program->rules + i);
}

void resolve_node(Program *program, Node *node)
{
    // TODO: Check that there are no duplicate property names
    // TODO: Check the kind of each property is valid
}

void resolve_rule(Program *program, Rule *rule)
{
    // TODO: Check that there are no duplicate variable names
    // TODO: Resolve variables types

    rule->expression = resolve_expression(program, rule, rule->expression);
}

Expression *resolve_expression(Program *program, Rule *rule, Expression *expr)
{
    if (expr->kind == UNRESOLVED_NAME)
    {
        sub_string name = expr->name;
        for (size_t i = 0; i < rule->variables_count; i++)
        {
            Variable *var = rule->variables + i;
            sub_string var_name = var->name;
            printf(" %.*s  %.*s\n", name.len, name.str, var_name.len, var_name.str);
            if (name.len == var_name.len && strncmp(name.str, var_name.str, name.len) == 0)
            {
                expr->kind = VARIABLE;
                expr->var = var;
                return expr;
            }
        }

        // TODO: Report line number and column
        fprintf(stderr, "Error. Variable %.*s does not exist\n", name.len, name.str);
        exit(EXIT_FAILURE);
    }

    if (expr->kind == BIN_OP)
    {
        if (expr->op == INDEX)
        {
            expr->lhs = resolve_expression(program, rule, expr->lhs);
            expr->rhs->kind = PROPERTY_NAME;
            // TODO: Check that the rhs is a valid property of the lhs's node type
        }
        else
        {
            expr->lhs = resolve_expression(program, rule, expr->lhs);
            expr->rhs = resolve_expression(program, rule, expr->rhs);
        }
    }

    return expr;
}

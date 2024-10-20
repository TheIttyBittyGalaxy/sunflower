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
    // TODO: Check that there are no duplicate placeholder names

    for (size_t i = 0; i < rule->placeholders_count; i++)
    {
        Placeholder *placeholder = rule->placeholders + i;

        for (size_t n = 0; n < program->nodes_count; n++)
        {
            Node *node = program->nodes + n;
            if (node->name.len == placeholder->node_name.len && (strncmp(node->name.str, placeholder->node_name.str, node->name.len) == 0))
            {
                placeholder->node_type = node;
                break;
            }
        }

        if (!placeholder->node_type)
        {
            fprintf(stderr, "Could not find node with name %.*s", placeholder->node_name.len, placeholder->node_name.str);
            exit(EXIT_FAILURE);
        }
    }

    rule->expression = resolve_expression(program, rule, rule->expression);
}

Expression *resolve_expression(Program *program, Rule *rule, Expression *expr)
{
    if (expr->kind == UNRESOLVED_NAME)
    {
        sub_string name = expr->name;
        for (size_t i = 0; i < rule->placeholders_count; i++)
        {
            Placeholder *placeholder = rule->placeholders + i;
            sub_string var_name = placeholder->name;
            if (name.len == var_name.len && strncmp(name.str, var_name.str, name.len) == 0)
            {
                expr->kind = PLACEHOLDER;
                expr->placeholder = placeholder;
                return expr;
            }
        }

        // TODO: Report line number and column
        fprintf(stderr, "Error. Placeholder %.*s does not exist\n", name.len, name.str);
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

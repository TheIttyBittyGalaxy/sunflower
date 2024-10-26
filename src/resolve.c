#include <stdio.h>

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
    for (size_t i = 0; i < node->properties_count; i++)
    {
        Property property = node->properties[i];

        // TODO: Check the kind of each property is valid

        // Check that name is not used by another property
        for (size_t j = i + 1; j < node->properties_count; j++)
        {
            Property other = node->properties[j];
            if (substrings_match(property.name, other.name))
            {
                fprintf(stderr, "Declaration for '%.*s' contains conflicting definitions for '%.*s' property", node->name.len, node->name.str, property.name.len, property.name.str);
                exit(EXIT_FAILURE);
            }
        }
    }
}

void resolve_rule(Program *program, Rule *rule)
{
    for (size_t i = 0; i < rule->placeholders_count; i++)
    {
        Placeholder *placeholder = rule->placeholders + i;

        // Check that name is not used by another placeholder
        for (size_t j = i + 1; j < rule->placeholders_count; j++)
        {
            Placeholder other = rule->placeholders[j];
            if (substrings_match(placeholder->name, other.name))
            {
                fprintf(stderr, "Rule contains multiple placeholders named '%.*s'", placeholder->name.len, placeholder->name.str);
                print_rule(rule);
                exit(EXIT_FAILURE);
            }
        }

        // Resolve the placeholder's node type
        for (size_t n = 0; n < program->nodes_count; n++)
        {
            Node *node = program->nodes + n;
            if (substrings_match(node->name, placeholder->node_name))
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
        sub_string unresolved_name = expr->name;
        for (size_t i = 0; i < rule->placeholders_count; i++)
        {
            Placeholder *placeholder = rule->placeholders + i;
            if (substrings_match(unresolved_name, placeholder->name))
            {
                expr->kind = PLACEHOLDER;
                expr->placeholder = placeholder;
                return expr;
            }
        }

        // TODO: Report line number and column
        fprintf(stderr, "Error. Placeholder %.*s does not exist\n", unresolved_name.len, unresolved_name.str);
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

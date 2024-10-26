#include <stdio.h>
#include <string.h>

#include "resolve.h"

// TODO: Create language errors, rather than terminating the entire program
// TODO: Update errors for maximum usability (i.e. clear error messages, line numbers, etc)

// Resolve methods
Expression *resolve_expression(Program *program, Rule *rule, Expression *expr);

void resolve(Program *program)
{
    // Check that node and property names are not duplicated
    for (size_t n = 0; n < program->nodes_count; n++)
    {
        Node *node = program->nodes + n;

        // TODO: Prevent illegal node names (e.g. `num`)

        // Check for duplicate names
        for (size_t j = n + 1; j < program->nodes_count; j++)
        {
            Node other = program->nodes[j];
            if (substrings_match(node->name, other.name))
            {
                fprintf(stderr, "There are conflicting declarations for the '%.*s' node", node->name.len, node->name.str);
                exit(EXIT_FAILURE);
            }
        }

        // Check for duplicate property names and resolve the type of each property
        for (size_t p = 0; p < node->properties_count; p++)
        {
            Property *property = node->properties + p;

            // Check for duplicate property names
            for (size_t j = p + 1; j < node->properties_count; j++)
            {
                Property other = node->properties[j];
                if (substrings_match(property->name, other.name))
                {
                    fprintf(stderr, "Declaration for '%.*s' contains conflicting definitions for '%.*s' property", node->name.len, node->name.str, property->name.len, property->name.str);
                    exit(EXIT_FAILURE);
                }
            }

            // Resolve the property's type
            if (property->type_name.len == 3 && (strncmp(property->type_name.str, "num", 3) == 0))
            {
                property->type = TYPE_NUM;
            }
            else
            {
                for (size_t j = 0; j < program->nodes_count; j++)
                {
                    Node *node = program->nodes + j;
                    if (substrings_match(node->name, property->type_name))
                    {
                        property->type = TYPE_NODE;
                        property->node_type = node;
                        break;
                    }
                }
            }

            if (property->type == TYPE_NULL)
            {
                fprintf(stderr, "Type '%.*s' of '%.*s' property does not exist.", property->type_name.len, property->type_name.str, property->name.len, property->name.str);
                exit(EXIT_FAILURE);
            }
        }
    }

    // Resolve rules
    for (size_t r = 0; r < program->rules_count; r++)
    {
        Rule *rule = program->rules + r;

        // Check for duplicate placeholder names and resolve the type of each placeholder
        for (size_t p = 0; p < rule->placeholders_count; p++)
        {
            Placeholder *placeholder = rule->placeholders + p;

            // Check for duplicate names
            for (size_t j = p + 1; j < rule->placeholders_count; j++)
            {
                Placeholder other = rule->placeholders[j];
                if (substrings_match(placeholder->name, other.name))
                {
                    fprintf(stderr, "Rule contains multiple placeholders named '%.*s'", placeholder->name.len, placeholder->name.str);
                    print_rule(rule);
                    exit(EXIT_FAILURE);
                }
            }

            // Resolve the placeholder's type
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

        // Resolve rule expression
        rule->expression = resolve_expression(program, rule, rule->expression);
    }
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

        fprintf(stderr, "Error. Placeholder %.*s does not exist\n", unresolved_name.len, unresolved_name.str);
        exit(EXIT_FAILURE);
    }

    if (expr->kind == BIN_OP)
    {
        if (expr->op == INDEX)
        {
            // TODO: Resolve index operations correctly! Right now we just assume the index is a valid shallow index into a placeholder
            expr->lhs = resolve_expression(program, rule, expr->lhs);
            expr->rhs->kind = PROPERTY_NAME;
        }
        else
        {
            // TODO: Type check operands
            expr->lhs = resolve_expression(program, rule, expr->lhs);
            expr->rhs = resolve_expression(program, rule, expr->rhs);
        }
    }

    return expr;
}

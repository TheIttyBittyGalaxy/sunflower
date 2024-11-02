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

        // Prevent illegal node names
        if (node->name.len == 3 && (strncmp(node->name.str, "num", 3) == 0))
        {
            fprintf(stderr, "Cannot name Node '%.*s' as this is an existing type", node->name.len, node->name.str);
            exit(EXIT_FAILURE);
        }

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
                property->type.primitive = TYPE_PRIMITIVE__NUMBER;
            }
            else
            {
                for (size_t j = 0; j < program->nodes_count; j++)
                {
                    Node *node = program->nodes + j;
                    if (substrings_match(node->name, property->type_name))
                    {
                        property->type.primitive = TYPE_PRIMITIVE__NODE;
                        property->type.node = node;
                        break;
                    }
                }
            }

            if (property->type.primitive == TYPE_PRIMITIVE__UNRESOLVED)
            {
                property->type.primitive = TYPE_PRIMITIVE__INVALID;
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
                if (substrings_match(node->name, placeholder->type_name))
                {
                    placeholder->type.primitive = TYPE_PRIMITIVE__NODE;
                    placeholder->type.node = node;
                    break;
                }
            }

            if (placeholder->type.primitive == TYPE_PRIMITIVE__UNRESOLVED)
            {
                placeholder->type.primitive = TYPE_PRIMITIVE__INVALID;
                fprintf(stderr, "Could not find node with name %.*s", placeholder->type_name.len, placeholder->type_name.str);
                exit(EXIT_FAILURE);
            }
        }

        // Resolve rule expression
        rule->expression = resolve_expression(program, rule, rule->expression);
    }
}

Expression *resolve_expression(Program *program, Rule *rule, Expression *expr)
{
    if (expr->variant == EXPR_VARIANT__UNRESOLVED_NAME)
    {
        sub_string unresolved_name = expr->name;
        for (size_t i = 0; i < rule->placeholders_count; i++)
        {
            Placeholder *placeholder = rule->placeholders + i;
            if (substrings_match(unresolved_name, placeholder->name))
            {
                expr->variant = EXPR_VARIANT__PLACEHOLDER;
                expr->placeholder = placeholder;
                return expr;
            }
        }

        fprintf(stderr, "Error. Placeholder %.*s does not exist\n", unresolved_name.len, unresolved_name.str);
        exit(EXIT_FAILURE);
    }

    if (expr->variant == EXPR_VARIANT__BIN_OP)
    {
        if (expr->op == OPERATION__ACCESS)
        {
            // TODO: Currently we only support shallow indexing of a placeholder value. Support nested indexing.

            // Check expressions
            Expression *subject = resolve_expression(program, rule, expr->lhs);
            if (subject->variant != EXPR_VARIANT__PLACEHOLDER)
            {
                fprintf(stderr, "Internal error: Indexing expressions which are not placeholders is not yet supported.\n");
                print_expression(expr);
                exit(EXIT_FAILURE);
            }

            if (expr->rhs->variant != EXPR_VARIANT__UNRESOLVED_NAME)
            {
                fprintf(stderr, "Index into placeholder is invalid.\n");
                print_expression(expr);
                exit(EXIT_FAILURE);
            }
            sub_string property_name = expr->rhs->name;

            // Convert BIN_OP to PROPERTY_ACCESS
            Placeholder *placeholder = subject->placeholder;
            Node *node = placeholder->type.node;

            expr->variant = EXPR_VARIANT__PROPERTY_ACCESS;
            expr->subject = subject;

            expr->placeholder_index = placeholder->index;
            for (size_t i = 0; i < node->properties_count; i++)
            {
                Property *property = node->properties + i;
                if (substrings_match(property_name, property->name))
                {
                    expr->property_offset = i;
                    break;
                }
            }
        }
        else
        {
            expr->lhs = resolve_expression(program, rule, expr->lhs);
            expr->rhs = resolve_expression(program, rule, expr->rhs);

            switch (expr->op)
            {

            // NOTE: OPERATION__ACCESS is handled separately, see above

            // Both operands should be numbers
            case OPERATION__MUL:
            case OPERATION__DIV:
            case OPERATION__ADD:
            case OPERATION__SUB:

            case OPERATION__LESS_THAN:
            case OPERATION__MORE_THAN:
            case OPERATION__LESS_THAN_OR_EQUAL:
            case OPERATION__MORE_THAN_OR_EQUAL:
            {
                if (deduce_type_of(expr->lhs).primitive != TYPE_PRIMITIVE__NUMBER)
                {
                    fprintf(stderr, "Expression is not a number.\n");
                    print_expression(expr->lhs);
                    exit(EXIT_FAILURE);
                }

                if (deduce_type_of(expr->rhs).primitive != TYPE_PRIMITIVE__NUMBER)
                {
                    fprintf(stderr, "Expression is not a number.\n");
                    print_expression(expr->rhs);
                    exit(EXIT_FAILURE);
                }

                break;
            }

            case OPERATION__EQUAL_TO:
            case OPERATION__NOT_EQUAL_TO:
            {
                ExprType lht = deduce_type_of(expr->lhs);
                ExprType rht = deduce_type_of(expr->rhs);

                if (
                    (lht.primitive != rht.primitive) ||
                    (lht.primitive == TYPE_PRIMITIVE__NODE && rht.primitive == TYPE_PRIMITIVE__NODE && lht.node != rht.node))
                {
                    fprintf(stderr, "LHS and RHS of comparison will never be the same.\n");
                    print_expression(expr);
                    exit(EXIT_FAILURE);
                }

                break;
            }

            // Both operands should be booleans
            // TODO: We should also allow any value which could be NULL
            case OPERATION__LOGICAL_AND:
            case OPERATION__LOGICAL_OR:
            {
                if (deduce_type_of(expr->lhs).primitive != TYPE_PRIMITIVE__BOOL)
                {
                    fprintf(stderr, "Expression is not a boolean.\n");
                    print_expression(expr->lhs);
                    exit(EXIT_FAILURE);
                }

                if (deduce_type_of(expr->rhs).primitive != TYPE_PRIMITIVE__BOOL)
                {
                    fprintf(stderr, "Expression is not a boolean.\n");
                    print_expression(expr->rhs);
                    exit(EXIT_FAILURE);
                }

                break;
            }

            default:
            {
                fprintf(stderr, "Internal error: Cannot resolve %s binary operation", operation_string(expr->op));
                exit(EXIT_FAILURE);
            }
            }
        }
    }

    return expr;
}

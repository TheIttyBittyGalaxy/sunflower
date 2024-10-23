#include <stdio.h>
#include <string.h>

#include "constraints.h"
#include "memory.h"

// ConversionResult
// When we are converting a rule into multiple arc constraints, `convert_expression` will take a rule
// and produce a single expression (to be used by the arc), as well as a `ConversionResult`. `create_constraints`
// can then use this to create an arc for every set of instances that the rule applies to.

typedef struct
{
    size_t placeholder_index;
    size_t property_offset;
} VariableInfo; // CLEANUP: Is there a better name for this?

typedef struct
{
    VariableInfo *var_info;
    union
    {
        size_t var_info_count;
        size_t var_count;
    };
} ConversionResult; // CLEANUP: Is there a better name for this?

size_t get_variable_index_for(ConversionResult *result, size_t placeholder_index, size_t property_offset)
{
    for (size_t i = 0; i < result->var_info_count; i++)
    {
        VariableInfo info = result->var_info[i];
        if (
            info.placeholder_index == placeholder_index &&
            info.property_offset == property_offset)
            return i;
    }

    VariableInfo *info = EXTEND_ARRAY(result->var_info, VariableInfo);
    info->placeholder_index = placeholder_index;
    info->property_offset = property_offset;
    return result->var_info_count - 1;
}

// Convert a rule's expression into an arc expression
Expression *convert_expression(ConversionResult *result, Rule *rule, Expression *program_expression)
{
    Expression *expr = NEW(Expression);

    switch (program_expression->kind)
    {
    case NUMBER_LITERAL:
    {
        memcpy(expr, program_expression, sizeof(Expression));
        break;
    }
    case BIN_OP:
    {
        if (program_expression->op == INDEX)
        {
            // TODO: This way of converting an index is a bit of a hack.
            //       This will need to change to generalise to most expressions.
            if (program_expression->lhs->kind != PLACEHOLDER || program_expression->rhs->kind != PROPERTY_NAME)
            {
                fprintf(stderr, "Internal error: Could not convert index expression into arc expression");
                print_expression(program_expression);
                exit(EXIT_FAILURE);
            }

            // CLEANUP: This could be handled better handled during 'resolve' instead.
            // CLEANUP: Having to determine the index of placeholder like this shouldn't really be required.
            //          Make adjustments upstream so that we never need to do this!
            expr->kind = ARC_VALUE;
            Placeholder *placeholder = program_expression->lhs->placeholder;
            Node *node = placeholder->node_type;

            size_t placeholder_index = -1;
            for (size_t i = 0; i < rule->placeholders_count; i++)
            {
                if (placeholder == (rule->placeholders + i))
                {
                    placeholder_index = i;
                    break;
                }
            }

            sub_string field_name = program_expression->rhs->name;
            for (size_t i = 0; i < node->properties_count; i++)
            {
                sub_string property_name = (node->properties + i)->name;
                if (substrings_match(field_name, property_name))
                {
                    expr->index = get_variable_index_for(result, placeholder_index, i);
                    break;
                }
            }
        }
        else
        {
            expr->kind = BIN_OP;
            expr->op = program_expression->op;
            expr->lhs = convert_expression(result, rule, program_expression->lhs);
            expr->rhs = convert_expression(result, rule, program_expression->rhs);
        }

        break;
    }
    default:
    {
        fprintf(stderr, "Attempt to convert %s program expression into an arc expression", expression_kind_string(program_expression->kind));
        print_expression(program_expression);
        exit(EXIT_FAILURE);
    }
    }

    return expr;
}

// Create a copy of an arc expresion where each the variable index for each arc value is rotated by a given amount
Expression *rotate_arc_expression(Expression *original_expr, size_t rotation, size_t variable_count)
{
    Expression *expr = NEW(Expression);

    switch (original_expr->kind)
    {
    case NUMBER_LITERAL:
    {
        memcpy(expr, original_expr, sizeof(Expression));
        break;
    }
    case BIN_OP:
    {
        expr->kind = BIN_OP;
        expr->op = original_expr->op;
        expr->lhs = rotate_arc_expression(original_expr->lhs, rotation, variable_count);
        expr->rhs = rotate_arc_expression(original_expr->rhs, rotation, variable_count);
        break;
    }
    case ARC_VALUE:
    {
        expr->kind = ARC_VALUE;
        expr->index = (original_expr->index + rotation) % variable_count;
        break;
    }
    default:
    {
        fprintf(stderr, "Attempt to copy %s arc expression", expression_kind_string(original_expr->kind));
        print_expression(original_expr);
        exit(EXIT_FAILURE);
    }
    }

    return expr;
}

// Create constraints
void create_arcs_from_rule(Constraints *constraints, Rule *rule, QuantumMap *quantum_map)
{
    ConversionResult result;
    INIT_ARRAY(result.var_info);

    // FIXME: The expression that is returned by `convert_expression` never has an "owning"
    //        reference created for it, meaning it's not clear how it would be freed? Once
    //        it is clearer how this should be done, fix this!
    Expression *arc_expression = convert_expression(&result, rule, rule->expression);

    for (size_t i = 0; i < result.var_info_count; i++)
    {
        VariableInfo info = result.var_info[i];
        printf("%d: [%d].%d\n", i, info.placeholder_index, info.property_offset);
    }
    printf("\n");

    if (result.var_count == 0)
    {
        fprintf(stderr, "Internal error: Somehow created an arc that constraints no values");
        exit(EXIT_FAILURE);
    }

    // Arcs that constrain a single variable (and thus have one placeholder)
    if (result.var_count == 1)
    {
        Placeholder *placeholder = rule->placeholders;

        for (size_t i = 0; i < quantum_map->instances_count; i++)
        {
            QuantumInstance *instance = quantum_map->instances + i;
            if (instance->node != placeholder->node_type)
                continue;

            Arc *arc = EXTEND_ARRAY(constraints->single_arcs, Arc);
            arc->expr = arc_expression;
            arc->variable_indexes_count = 1;
            arc->variable_indexes = (size_t *)malloc(sizeof(size_t));
            arc->variable_indexes[0] = instance->variables_array_index + result.var_info[0].property_offset;
        }

        return;
    }

    // Arcs that constrain a multiple variables (and thus may have multiple placeholders)
    Expression *arc_expressions = (Expression *)malloc(sizeof(Expression) * result.var_count);

    // FIXME: This almost certainly causes memory leaks?
    // CLEANUP: Rather than storing rotated expressions in memory, we should
    //          just rotate the ARC_VALUE expression nodes in real time
    arc_expressions[0] = *arc_expression;
    for (size_t rotation = 1; rotation < result.var_count; rotation++)
        arc_expressions[rotation] = *rotate_arc_expression(arc_expression, rotation, result.var_count);

    size_t total_placeholders = rule->placeholders_count;
    size_t *instance_index = (size_t *)malloc(sizeof(size_t) * total_placeholders);

    for (size_t i = 0; i < total_placeholders; i++)
        instance_index[i] = 0;

    while (true)
    {
        // Skip combinations of instances until we find a combination that patch the placeholders of the rule
        {
            bool complete = false;
            size_t n = 0;
            while (n < total_placeholders)
            {
                QuantumInstance *instance = quantum_map->instances + instance_index[n];
                printf("n: %d, instance_index[n]: %d, instance->node: %d, rule->placeholders[n].node_type: %d, match: %s\n",
                       n,
                       instance_index[n],
                       instance->node,
                       rule->placeholders[n].node_type,
                       instance->node == rule->placeholders[n].node_type ? "true" : "false");

                if (instance->node == rule->placeholders[n].node_type)
                {
                    n++;
                    continue;
                }

                instance_index[n]++;

                if (instance_index[n] == quantum_map->instances_count)
                {
                    n++;

                    if (n >= total_placeholders)
                    {
                        complete = true;
                        break;
                    }

                    instance_index[n]++;
                    for (size_t v = 1; v < n; v++)
                        instance_index[v] = 0;
                    n = 1;
                }
            }

            if (complete)
                break;
        }

        // Create an arc for each constrained variable
        for (size_t rotation = 0; rotation < result.var_count; rotation++)
        {
            Arc *arc = EXTEND_ARRAY(constraints->multi_arcs, Arc);
            arc->expr = arc_expressions + rotation;
            arc->variable_indexes_count = result.var_count;
            arc->variable_indexes = (size_t *)malloc(sizeof(size_t) * arc->variable_indexes_count);
            for (size_t v = 0; v < result.var_count; v++)
            {
                size_t n = (v + rotation) % result.var_count;
                VariableInfo info = result.var_info[v];
                QuantumInstance *instance = quantum_map->instances + instance_index[info.placeholder_index];
                arc->variable_indexes[n] = instance->variables_array_index + info.property_offset;
            }
        }

        // Increment to a new combination of instances
        {
            size_t n = 0;
            while (n < total_placeholders)
            {
                instance_index[n]++;

                if (instance_index[n] < quantum_map->instances_count)
                    break;

                instance_index[n] = 0;
                n++;
            }

            if (n >= total_placeholders)
                break;
        }
    }
}

Constraints create_constraints(Program *program, QuantumMap *quantum_map)
{
    Constraints constraints;
    INIT_ARRAY(constraints.single_arcs);
    INIT_ARRAY(constraints.multi_arcs);

    for (size_t i = 0; i < program->rules_count; i++)
        create_arcs_from_rule(&constraints, program->rules + i, quantum_map);

    return constraints;
}

// Printing & strings
void print_arc(Arc *arc)
{
    printf("\t");
    for (size_t i = 0; i < arc->variable_indexes_count; i++)
        printf("%03d  ", arc->variable_indexes[i]);
    print_expression(arc->expr);
    printf("\n");
}

void print_constraints(Constraints constraints)
{
    for (size_t i = 0; i < constraints.single_arcs_count; i++)
        print_arc(constraints.single_arcs + i);
    for (size_t i = 0; i < constraints.multi_arcs_count; i++)
        print_arc(constraints.multi_arcs + i);
}
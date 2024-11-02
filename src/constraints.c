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

            Placeholder *placeholder = program_expression->lhs->placeholder;

            expr->kind = ARC_VALUE;
            expr->index = get_variable_index_for(result, placeholder->index, program_expression->index_property_index);
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

// Create constraints
void create_arcs_from_rule(Constraints *constraints, Rule *rule, QuantumMap *quantum_map)
{
    ConversionResult result;
    INIT_ARRAY(result.var_info);

    // FIXME: The expression that is returned by `convert_expression` never has an "owning"
    //        reference created for it, meaning it's not clear how it would be freed? Once
    //        it is clearer how this should be done, fix this!
    Expression *arc_expression = convert_expression(&result, rule, rule->expression);

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
    size_t total_placeholders = rule->placeholders_count;
    size_t *instance_index = (size_t *)malloc(sizeof(size_t) * total_placeholders);

    for (size_t i = 0; i < total_placeholders; i++)
        instance_index[i] = 0;

    while (true)
    {
        // Skip combinations of instances until we find a combination that patch the placeholders of the rule

        // TODO: For right now, we say that two place holders of the same node type cannot represent the same instance.
        //       This is a "for now" solution, but I'm not sure what the semantics here really ought to be?

        // CLEANUP: This code _works_, but it probably isn't particularly efficient!

        // CLEANUP: Right now, this will create essentially duplicate arcs in any situation where there are
        //          multiple placeholders of the same node types. e.g. `Thing x Thing y: x.foo = y.foo` will result in:
        //          - 000.foo = 001.foo (2 rotations)
        //          - 001.foo = 000.foo (2 rotations)
        //          Once the semantics for rule selectors are decided, look into how this can best be optimised!
        {
            bool complete = false;
            size_t n = 0;
            while (n < total_placeholders)
            {
                QuantumInstance *instance = quantum_map->instances + instance_index[n];
                if (instance->node == rule->placeholders[n].node_type)
                {
                    bool no_repeats = true;
                    for (size_t v = 0; v < n; v++)
                    {
                        if (instance_index[v] == instance_index[n])
                        {
                            no_repeats = false;
                            n = v;
                            break;
                        }
                    }

                    if (no_repeats)
                    {
                        n++;
                        continue;
                    }
                }

                instance_index[n]++;

                if (instance_index[n] >= quantum_map->instances_count)
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
            arc->expr = arc_expression;
            arc->expr_rotation = rotation;
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
    for (size_t i = 0; i < arc->variable_indexes_count; i++)
        printf("%03d  ", (arc->variable_indexes[i] + arc->expr_rotation) % arc->variable_indexes_count);
    print_expression(arc->expr);
}

void print_constraints(Constraints constraints)
{
    for (size_t i = 0; i < constraints.single_arcs_count; i++)
    {
        print_arc(constraints.single_arcs + i);
        printf("\n");
    }

    for (size_t i = 0; i < constraints.multi_arcs_count; i++)
    {
        print_arc(constraints.multi_arcs + i);
        printf("\n");
    }
}
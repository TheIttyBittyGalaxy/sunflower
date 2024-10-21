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

size_t get_variable_index_for(ConversionResult *result, size_t property_offset)
{
    for (size_t i = 0; i < result->var_info_count; i++)
    {
        VariableInfo info = result->var_info[i];
        if (info.property_offset == property_offset)
            return i;
    }

    VariableInfo *info = EXTEND_ARRAY(result->var_info, VariableInfo);
    info->property_offset = property_offset;
    return result->var_info_count - 1;
}

// Convert a rule's expression into an arc expression
Expression *convert_expression(ConversionResult *result, Node *node, Expression *program_expression)
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
                fprintf(stderr, "Internal error, could not resolve index expression");
                print_expression(program_expression);
                exit(EXIT_FAILURE);
            }

            expr->kind = ARC_VALUE;

            sub_string prop_name = program_expression->rhs->name;
            for (size_t i = 0; i < node->properties_count; i++)
            {
                sub_string other_name = (node->properties + i)->name;
                if (substrings_match(prop_name, other_name))
                {
                    expr->index = get_variable_index_for(result, i);
                    break;
                }
            }
        }
        else
        {
            expr->kind = BIN_OP;
            expr->op = program_expression->op;
            expr->lhs = convert_expression(result, node, program_expression->lhs);
            expr->rhs = convert_expression(result, node, program_expression->rhs);
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
    // TODO: Create arcs for rules with multiple placeholders
    if (rule->placeholders_count != 1)
    {
        printf("Rules with multiple placeholders (or no placeholders?) not yet supported:\n");
        print_rule(rule);
        return;
    }

    // NOTE: The following code is designed for rules with exactly one placeholder.
    Placeholder *placeholder = rule->placeholders;

    ConversionResult result;
    INIT_ARRAY(result.var_info);

    // FIXME: The expression that is returned by `convert_expression` never has an "owning"
    //        reference created for it, meaning it's not clear how it would be freed? Once
    //        it is clearer how this should be done, fix this!
    Expression *arc_expression = convert_expression(&result, placeholder->node_type, rule->expression);

    if (result.var_count == 0)
    {
        fprintf(stderr, "Internal error: Somehow created an arc that constraints no values");
        exit(EXIT_FAILURE);
    }

    // Arcs that constrain a single variable
    if (result.var_count == 1)
    {
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

    // Arcs that constrain a multiple variables
    Expression *arc_expressions = (Expression *)malloc(sizeof(Expression) * result.var_count);

    // FIXME: This almost certainly causes memory leaks?
    arc_expressions[0] = *arc_expression;
    for (size_t rotation = 1; rotation < result.var_count; rotation++)
        arc_expressions[rotation] = *rotate_arc_expression(arc_expression, rotation, result.var_count);

    for (size_t i = 0; i < quantum_map->instances_count; i++)
    {
        QuantumInstance *instance = quantum_map->instances + i;
        if (instance->node != placeholder->node_type)
            continue;

        for (size_t rotation = 0; rotation < result.var_count; rotation++)
        {
            Arc *arc = EXTEND_ARRAY(constraints->multi_arcs, Arc);
            arc->expr = arc_expressions + rotation;
            arc->variable_indexes_count = result.var_count;
            arc->variable_indexes = (size_t *)malloc(sizeof(size_t) * arc->variable_indexes_count);
            for (size_t v = 0; v < result.var_count; v++)
            {
                size_t n = (v + rotation) % result.var_count;
                arc->variable_indexes[n] = instance->variables_array_index + result.var_info[v].property_offset;
            }
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
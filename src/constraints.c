#include <stdio.h>
#include <string.h>

#include "constraints.h"

// ArcInformation
// When we are converting a rule into multiple arc constraints, `convert_expression` will take a rule
// and produce a single expression (to be used by the arc), as well as a `ArcInformation`. `create_constraints`
// can then use this to create an arc for every set of instances that the rule applies to.

typedef struct
{
    size_t property_offset;
} ArcValueInfo; // CLEANUP: Is there a better name for this?

typedef struct
{
    Expression *exprs;
    size_t exprs_count;
    ArcValueInfo *value_info_map;
    size_t value_info_map_count;
} ArcInformation; // CLEANUP: Is there a better name for this?

size_t arc_value_index_for(ArcInformation *arc_info, size_t property_offset)
{
    for (size_t i = 0; i < arc_info->value_info_map_count; i++)
    {
        ArcValueInfo info = arc_info->value_info_map[i];
        if (info.property_offset == property_offset)
            return i;
    }

    ArcValueInfo *info = EXTEND_ARRAY(arc_info->value_info_map, ArcValueInfo);
    info->property_offset = property_offset;
    return arc_info->value_info_map_count - 1;
}

// Convert program expression into arc

Expression *convert_expression(ArcInformation *arc_info, Node *node, Expression *program_expression);
Expression *copy_arc_expr_with_rotation(Expression *original_expr, size_t rotation, size_t value_count);

ArcInformation convert(Node *node, Expression *program_expression)
{
    ArcInformation arc_info;
    INIT_ARRAY(arc_info.exprs);
    INIT_ARRAY(arc_info.value_info_map);

    Expression *expr = convert_expression(&arc_info, node, program_expression);

    arc_info.exprs_count = arc_info.value_info_map_count;
    arc_info.exprs = (Expression *)malloc(sizeof(Expression) * arc_info.exprs_count);

    arc_info.exprs[0] = *expr; // FIXME: This probably leaks memory!!
    for (size_t i = 1; i < arc_info.exprs_count; i++)
    {
        // FIXME: This probably also leaks memory!
        arc_info.exprs[i] = *copy_arc_expr_with_rotation(expr, i, arc_info.value_info_map_count);
    }
    return arc_info;
}

Expression *convert_expression(ArcInformation *arc_info, Node *node, Expression *program_expression)
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
            if (program_expression->lhs->kind != VARIABLE || program_expression->rhs->kind != PROPERTY_NAME)
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
                if (prop_name.len == other_name.len && (strncmp(prop_name.str, other_name.str, prop_name.len) == 0))
                {
                    expr->index = arc_value_index_for(arc_info, i);
                    break;
                }
            }
        }
        else
        {
            expr->kind = BIN_OP;
            expr->op = program_expression->op;
            expr->lhs = convert_expression(arc_info, node, program_expression->lhs);
            expr->rhs = convert_expression(arc_info, node, program_expression->rhs);
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

Expression *copy_arc_expr_with_rotation(Expression *original_expr, size_t rotation, size_t value_count)
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
        expr->lhs = copy_arc_expr_with_rotation(original_expr->lhs, rotation, value_count);
        expr->rhs = copy_arc_expr_with_rotation(original_expr->rhs, rotation, value_count);
        break;
    }
    case ARC_VALUE:
    {
        expr->kind = ARC_VALUE;
        expr->index = (original_expr->index + rotation) % value_count;
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
Constraints create_constraints(Program *program, QuantumMap *quantum_map)
{
    Constraints constraints;
    INIT_ARRAY(constraints.arcs);

    for (size_t i = 0; i < program->rules_count; i++)
    {
        Rule *rule = program->rules + i;

        // TODO: Create arcs for rules with multiple variables
        if (rule->variables_count != 1)
        {
            printf("Rules with multiple variables (or no variables?) not yet supported:\n");
            print_rule(rule);
            continue;
        }

        // NOTE: The following code is designed for rules with exactly one variable.
        Variable *var = rule->variables;

        // CLEANUP: Ideally, this resolution should have happened at a previous stage
        Node *var_node = NULL;
        for (size_t n = 0; n < program->nodes_count; n++)
        {
            Node *node = program->nodes + n;
            if (node->name.len == var->node_name.len && (strncmp(node->name.str, var->node_name.str, node->name.len) == 0))
            {
                var_node = node;
                break;
            }
        }

        if (!var_node)
        {
            fprintf(stderr, "Could not find node with name %.*s", var->node_name.len, var->node_name.str);
            exit(EXIT_FAILURE);
        }

        // TODO: The expression that is returned by `convert` never has an "owning" reference
        //       created for it, meaning it's not clear how it would be freed? Once it is clearer
        //       how this should be done, fix this!
        ArcInformation arc_info = convert(var_node, rule->expression);

        for (size_t j = 0; j < quantum_map->instances_count; j++)
        {
            QuantumInstance *instance = quantum_map->instances + j;
            if (instance->node != var_node)
                continue;

            for (size_t rotation = 0; rotation < arc_info.value_info_map_count; rotation++)
            {
                Arc *arc = EXTEND_ARRAY(constraints.arcs, Arc);
                arc->expr = arc_info.exprs + rotation;
                arc->value_indexes_count = arc_info.value_info_map_count;
                arc->value_indexes = (size_t *)malloc(sizeof(size_t) * arc->value_indexes_count);

                for (size_t v = 0; v < arc_info.value_info_map_count; v++)
                {
                    size_t n = (v + rotation) % arc_info.value_info_map_count;
                    arc->value_indexes[n] = instance->index_to_values_array + arc_info.value_info_map[v].property_offset;
                }
            }
        }
    }

    return constraints;
}

// Printing & strings
void print_arc(Arc *arc)
{
    printf("\t");
    for (size_t i = 0; i < arc->value_indexes_count; i++)
        printf("%03d  ", arc->value_indexes[i]);
    print_expression(arc->expr);
    printf("\n");
}

void print_constraints(Constraints constraints)
{
    for (size_t i = 0; i < constraints.arcs_count; i++)
        print_arc(constraints.arcs + i);
}
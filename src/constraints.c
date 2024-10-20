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
} VariableInfo; // CLEANUP: Is there a better name for this?

typedef struct
{
    Expression *exprs;
    size_t exprs_count;
    VariableInfo *var_info_map;
    size_t var_info_map_count;
} ArcInformation; // CLEANUP: Is there a better name for this?

size_t arc_value_index_for(ArcInformation *arc_info, size_t property_offset)
{
    for (size_t i = 0; i < arc_info->var_info_map_count; i++)
    {
        VariableInfo info = arc_info->var_info_map[i];
        if (info.property_offset == property_offset)
            return i;
    }

    VariableInfo *info = EXTEND_ARRAY(arc_info->var_info_map, VariableInfo);
    info->property_offset = property_offset;
    return arc_info->var_info_map_count - 1;
}

// Convert program expression into arc

Expression *convert_expression(ArcInformation *arc_info, Node *node, Expression *program_expression);
Expression *copy_arc_expr_with_rotation(Expression *original_expr, size_t rotation, size_t variable_count);

ArcInformation convert(Node *node, Expression *program_expression)
{
    ArcInformation arc_info;
    INIT_ARRAY(arc_info.exprs);
    INIT_ARRAY(arc_info.var_info_map);

    Expression *expr = convert_expression(&arc_info, node, program_expression);

    arc_info.exprs_count = arc_info.var_info_map_count;
    arc_info.exprs = (Expression *)malloc(sizeof(Expression) * arc_info.exprs_count);

    arc_info.exprs[0] = *expr; // FIXME: This probably leaks memory!!
    for (size_t i = 1; i < arc_info.exprs_count; i++)
    {
        // FIXME: This probably also leaks memory!
        arc_info.exprs[i] = *copy_arc_expr_with_rotation(expr, i, arc_info.var_info_map_count);
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

Expression *copy_arc_expr_with_rotation(Expression *original_expr, size_t rotation, size_t variable_count)
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
        expr->lhs = copy_arc_expr_with_rotation(original_expr->lhs, rotation, variable_count);
        expr->rhs = copy_arc_expr_with_rotation(original_expr->rhs, rotation, variable_count);
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
Constraints create_constraints(Program *program, QuantumMap *quantum_map)
{
    Constraints constraints;
    INIT_ARRAY(constraints.single_arcs);
    INIT_ARRAY(constraints.multi_arcs);

    for (size_t i = 0; i < program->rules_count; i++)
    {
        Rule *rule = program->rules + i;

        // TODO: Create arcs for rules with multiple placeholders
        if (rule->placeholders_count != 1)
        {
            printf("Rules with multiple placeholders (or no placeholders?) not yet supported:\n");
            print_rule(rule);
            continue;
        }

        // NOTE: The following code is designed for rules with exactly one placeholder.
        Placeholder *placeholder = rule->placeholders;

        // CLEANUP: Ideally, this resolution should have happened at a previous stage
        Node *placeholder_node_type = NULL;
        for (size_t n = 0; n < program->nodes_count; n++)
        {
            Node *node = program->nodes + n;
            if (node->name.len == placeholder->node_name.len && (strncmp(node->name.str, placeholder->node_name.str, node->name.len) == 0))
            {
                placeholder_node_type = node;
                break;
            }
        }

        if (!placeholder_node_type)
        {
            fprintf(stderr, "Could not find node with name %.*s", placeholder->node_name.len, placeholder->node_name.str);
            exit(EXIT_FAILURE);
        }

        // TODO: The expression that is returned by `convert` never has an "owning" reference
        //       created for it, meaning it's not clear how it would be freed? Once it is clearer
        //       how this should be done, fix this!
        ArcInformation arc_info = convert(placeholder_node_type, rule->expression);

        for (size_t j = 0; j < quantum_map->instances_count; j++)
        {
            QuantumInstance *instance = quantum_map->instances + j;
            if (instance->node != placeholder_node_type)
                continue;

            // Arcs that constrain a single variable
            if (arc_info.var_info_map_count == 1)
            {
                Arc *arc = EXTEND_ARRAY(constraints.single_arcs, Arc);
                arc->expr = arc_info.exprs;
                arc->variable_indexes_count = arc_info.var_info_map_count;
                arc->variable_indexes = (size_t *)malloc(sizeof(size_t) * arc->variable_indexes_count);
                for (size_t v = 0; v < arc_info.var_info_map_count; v++)
                    arc->variable_indexes[v] = instance->variables_array_index + arc_info.var_info_map[v].property_offset;
            }

            // Arcs that constrain multiple variables
            else
            {
                for (size_t rotation = 0; rotation < arc_info.var_info_map_count; rotation++)
                {
                    Arc *arc = EXTEND_ARRAY(constraints.multi_arcs, Arc);

                    arc->expr = arc_info.exprs + rotation;
                    arc->variable_indexes_count = arc_info.var_info_map_count;
                    arc->variable_indexes = (size_t *)malloc(sizeof(size_t) * arc->variable_indexes_count);

                    for (size_t v = 0; v < arc_info.var_info_map_count; v++)
                    {
                        size_t n = (v + rotation) % arc_info.var_info_map_count;
                        arc->variable_indexes[n] = instance->variables_array_index + arc_info.var_info_map[v].property_offset;
                    }
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
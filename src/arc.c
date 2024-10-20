#include <stdio.h>
#include <string.h>

#include "arc.h"

// Convert program expression into arc
typedef struct
{
    size_t value_index;
    Expression *expr;
} ConvertResult;

Expression *convert_expression(ConvertResult *result, QuantumInstance *instance, Expression *program_expression);

ConvertResult convert(QuantumInstance *instance, Expression *program_expression)
{
    ConvertResult result;
    result.expr = convert_expression(&result, instance, program_expression);
    return result;
}

Expression *convert_expression(ConvertResult *result, QuantumInstance *instance, Expression *program_expression)
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

            sub_string index_name = program_expression->rhs->name;
            for (size_t i = 0; i < instance->node->properties_count; i++)
            {
                sub_string prop_name = (instance->node->properties + i)->name;
                if (index_name.len == prop_name.len && (strncmp(index_name.str, prop_name.str, index_name.len) == 0))
                {
                    result->value_index = instance->index_to_values_array + i;
                    break;
                }
            }

            expr->kind = ARC_VALUE;
        }
        else
        {
            expr->kind = BIN_OP;
            expr->op = program_expression->op;
            expr->lhs = convert_expression(result, instance, program_expression->lhs);
            expr->rhs = convert_expression(result, instance, program_expression->rhs);
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

// Create arcs
ArcArray create_arcs(Program *program, QuantumMap *quantum_map)
{
    ArcArray array;
    INIT_ARRAY(array.arcs);

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

        // NOTE: The following code is designed for rules with exactly one variable,
        //       which is indexed exactly once.
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

        for (size_t j = 0; j < quantum_map->instances_count; j++)
        {
            QuantumInstance *instance = quantum_map->instances + j;
            if (instance->node != var_node)
                continue;

            Arc *arc = EXTEND_ARRAY(array.arcs, Arc);
            ConvertResult result = convert(instance, rule->expression);
            arc->value_index = result.value_index;
            arc->expr = result.expr;
        }
    }

    return array;
}

// Printing & strings
void print_arc(Arc *arc)
{
    printf("\t%03d ", arc->value_index);
    print_expression(arc->expr);
    printf("\n");
}

void print_arcs(ArcArray arcs)
{
    for (size_t i = 0; i < arcs.arcs_count; i++)
        print_arc(arcs.arcs + i);
}
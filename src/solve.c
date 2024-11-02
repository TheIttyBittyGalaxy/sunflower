#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "expression.h"
#include "solve.h"

// Evaluate arc expression

#define NUM_RESULT(result) ((ExprValue){ \
    .type = TYPE_PRIMITIVE__NUM,         \
    .num = result})

#define BOOL_RESULT(result) ((ExprValue){ \
    .type = TYPE_PRIMITIVE__BOOL,         \
    .boolean = result})

ExprValue evaluate_arc_expression(Arc *arc, Expression *expr, ExprValue *given_values)
{

    switch (expr->variant)
    {
    case EXPR_VARIANT__NUMBER_LITERAL:
        return NUM_RESULT(expr->number);

    case EXPR_VARIANT__BIN_OP:
    {
        ExprValue lhs = evaluate_arc_expression(arc, expr->lhs, given_values);
        ExprValue rhs = evaluate_arc_expression(arc, expr->rhs, given_values);

        if (expr->op == OPERATION__MUL)
            return NUM_RESULT(lhs.num * rhs.num);
        if (expr->op == OPERATION__DIV)
            return NUM_RESULT(lhs.num / rhs.num);
        if (expr->op == OPERATION__ADD)
            return NUM_RESULT(lhs.num + rhs.num);
        if (expr->op == OPERATION__SUB)
            return NUM_RESULT(lhs.num - rhs.num);

        if (expr->op == OPERATION__LESS_THAN)
            return BOOL_RESULT(lhs.num < rhs.num);
        if (expr->op == OPERATION__MORE_THAN)
            return BOOL_RESULT(lhs.num > rhs.num);
        if (expr->op == OPERATION__LESS_THAN_OR_EQUAL)
            return BOOL_RESULT(lhs.num <= rhs.num);
        if (expr->op == OPERATION__MORE_THAN_OR_EQUAL)
            return BOOL_RESULT(lhs.num <= rhs.num);

        if (expr->op == OPERATION__EQUAL_TO)
            return BOOL_RESULT(lhs.type == rhs.type && lhs.num == rhs.num); // FIXME: Using `num` regardless of the type is probably error prone?
        if (expr->op == OPERATION__NOT_EQUAL_TO)
            return BOOL_RESULT(lhs.type != rhs.type || lhs.num != rhs.num); // FIXME: Using `num` regardless of the type is probably error prone?

        if (expr->op == OPERATION__LOGICAL_AND)
            return BOOL_RESULT(lhs.boolean && rhs.boolean);
        if (expr->op == OPERATION__LOGICAL_OR)
            return BOOL_RESULT(lhs.boolean || rhs.boolean);

        fprintf(stderr, "Unable to evaluate %s binary operation", operation_string(expr->op));
        exit(EXIT_FAILURE);
    }

    case EXPR_VARIANT__VARIABLE_REFERENCE_INDEX:
    {
        size_t index = (expr->variable_reference_index + arc->expr_rotation) % arc->variable_indexes_count;
        return given_values[index];
    }

    default:
    {
        fprintf(stderr, "Unable to evaluate %s expression", expr_variant_string(expr->variant));
        print_expression(expr);
        exit(EXIT_FAILURE);
    }
    }

#undef BOOL_RESULT
#undef NUM_RESULT
}

// Apply arc constraints
void enforce_single_arc_constrains(QuantumMap *quantum_map, Constraints constraints)
{
    ExprValue given_value;
    given_value.type = TYPE_PRIMITIVE__NUM; // TODO: This is temporary. Eventually not all variables will be numbers.

    for (size_t arc_index = 0; arc_index < constraints.single_arcs_count; arc_index++)
    {
        Arc *arc = constraints.single_arcs + arc_index;

        size_t var_index = arc->variable_indexes[0];
        uint64_t var_bitfield = quantum_map->variables[var_index];

        for (int value = 0; value < 64; value++)
        {
            uint64_t value_bitfield = (1ULL << value);
            if (!(var_bitfield & value_bitfield))
                continue;

            given_value.num = value; // TODO: This is temporary. Eventually not all variables will be numbers.
            bool result = evaluate_arc_expression(arc, arc->expr, &given_value).boolean;

            if (!result)
                var_bitfield -= value_bitfield;
        }

        quantum_map->variables[var_index] = var_bitfield;
    }
}

// TODO: Support for more than a fixed number of variables.
#define MAX_VARIABLES 16

void enforce_multi_arc_constraints(QuantumMap *quantum_map, Constraints constraints)
{
    // Array to store bitfield for each variable constrained by an arc
    uint64_t var_bitfield[MAX_VARIABLES];
#define primary_bitfield (var_bitfield[0]) // Access the first element of `var_bitfields` as `primary_bitfield`

    // Initialise array of possible values for each variable
    int var_value[MAX_VARIABLES];
#define primary_value (var_value[0]) // Access the first element of `var_values` as `primary_value`

    // Initialise array of expression values
    ExprValue expression_values[MAX_VARIABLES];
    for (size_t i = 0; i < MAX_VARIABLES; i++)
        expression_values[i].type = TYPE_PRIMITIVE__NUM; // TODO: This is temporary. Eventually not all variables will be numbers.

    // Enforce each arc
    for (size_t arc_index = 0; arc_index < constraints.multi_arcs_count; arc_index++)
    {
        Arc *arc = constraints.multi_arcs + arc_index;
        size_t primary_index = arc->variable_indexes[0];
        size_t total_variables = arc->variable_indexes_count;

        // Ensure arc is not on too many variables
        if (arc->variable_indexes_count > MAX_VARIABLES)
        {
            fprintf(stderr, "Internal error: We are currently unable to enforce arcs that constrain more than %d variables.", MAX_VARIABLES);
            exit(EXIT_FAILURE);
        }

        // Store bitfield of each variable that is constrained by the arc
        for (size_t i = 0; i < total_variables; i++)
        {
            size_t var_index = arc->variable_indexes[i];
            var_bitfield[i] = quantum_map->variables[var_index];
        }

        // Test each potential value for the first variable to see if it should be eliminated
        for (primary_value = 0; primary_value < 64; primary_value++)
        {
            // Skip this value if it is already not a possibility
            if (!value_in_bitfield(primary_value, primary_bitfield))
                continue;

            // Reset the value of each variable to 0. These will be incremented as we test different combinations of possible values.
            for (size_t n = 1; n < MAX_VARIABLES; n++)
                var_value[n] = 0;

            // Determine if this value is a valid possibility
            bool primary_value_is_valid_possibility = false;
            while (true)
            {
                // For as long as the set of non-primary variable values is not possible, increment the set.
                {
                    bool end_of_possible_values = false;
                    size_t n = 1;
                    while (n < total_variables)
                    {
                        // NOTE: This loop is written in such a way that if var_bitfield[1] is 0, the loop will run indefinitely!
                        //       This is a quick and easy way to stop that from happening, and just quit the function quickly
                        //       so that `solve` can report an error.
                        if (var_bitfield[n] == 0)
                        {
                            end_of_possible_values = true;
                            return;
                        }

                        if (value_in_bitfield(var_value[n], var_bitfield[n]))
                        {
                            n++;
                            continue;
                        }

                        var_value[n]++;

                        while (var_value[n] == 64)
                        {
                            n++;

                            if (n >= total_variables)
                            {
                                end_of_possible_values = true;
                                break;
                            }

                            var_value[n]++;
                        }

                        for (size_t v = 1; v < n; v++)
                            var_value[v] = 0;

                        n = 1;
                    }

                    if (end_of_possible_values)
                        break;
                }

                // Evaluate the set of possible variables to determine if the primary value is a valid possibility
                for (size_t v = 0; v < total_variables; v++)
                    expression_values[v].num = var_value[v]; // TODO: This is temporary. Eventually not all variables will be numbers.
                bool result = evaluate_arc_expression(arc, arc->expr, expression_values).boolean;

                if (result)
                {
                    primary_value_is_valid_possibility = true;
                    break;
                }

                // Increment the set of variable values (excluding the primary variable)
                {
                    size_t n = 1;
                    while (n < total_variables)
                    {
                        var_value[n]++;

                        if (var_value[n] < 64)
                            break;

                        var_value[n] = 0;
                        n++;
                    }

                    if (n >= total_variables)
                        break;
                }
            }

            if (!primary_value_is_valid_possibility)
                primary_bitfield -= (1ULL << primary_value);
        }

        quantum_map->variables[primary_index] = primary_bitfield;
    }
#undef var_bitfield
#undef var_value
}

// Reset solution values
void reset_solution_values(QuantumMap *quantum_map, int ignore_index_and_before)
{
    for (size_t i = 0; i < quantum_map->instances_count; i++)
    {
        QuantumInstance *instance = quantum_map->instances + i;
        Node *node = instance->node;
        for (size_t p = 0; p < node->properties_count; p++)
        {
            int v = instance->variables_array_index + p;
            if (v <= ignore_index_and_before)
                continue;

            Property *property = node->properties + p;

            if (property->type == TYPE_PRIMITIVE__NUM)
            {
                quantum_map->variables[v] = UINT64_MAX;
            }

            else if (property->type == TYPE_PRIMITIVE__NODE)
            {
                uint64_t bitfield = 0;
                for (size_t k = 0; k < 64; k++)
                {
                    if (k >= quantum_map->instances_count)
                        break;

                    if (quantum_map->instances[k].node == property->node_type)
                        bitfield = bitfield | (1ULL << k);
                }

                quantum_map->variables[v] = bitfield;
            }

            else
            {
                fprintf(stderr, "Internal error: Encountered %s while resetting variables", type_category_string(property->type));
                exit(EXIT_FAILURE);
            }
        }
    }
}

// Solve
void solve(QuantumMap *quantum_map, Constraints constraints)
{
    reset_solution_values(quantum_map, -1);

    int *value_for = (int *)malloc(sizeof(int) * quantum_map->variables_count);
    uint64_t *remaining_values_for = (uint64_t *)malloc(sizeof(uint64_t) * quantum_map->variables_count);

    int i = -1;
    bool reapply_single_arc_constraints = true;
    while (i < (int)quantum_map->variables_count)
    {
        // printf("\r%d / %d            ", i, quantum_map->variables_count);

        // printf("%02d / %02d    ", i, quantum_map->variables_count);
        // for (int n = 0; n < quantum_map->variables_count; n++)
        //     n <= i ? printf("  %02d", value_for[n]) : printf("    ");
        // printf("\n");

        // 1. Apply constraints
        if (reapply_single_arc_constraints)
        {
            enforce_single_arc_constrains(quantum_map, constraints);
            reapply_single_arc_constraints = false;
        }
        enforce_multi_arc_constraints(quantum_map, constraints);

        // 2. Check if solution is valid
        bool valid_solution = true;
        for (size_t n = 0; n < quantum_map->variables_count; n++)
        {
            if (quantum_map->variables[n] == 0)
            {
                valid_solution = false;
                break;
            }
        }

        // 3. If solution is valid, collapse the next variable to a possible value
        if (valid_solution)
        {
            i++;
            remaining_values_for[i] = quantum_map->variables[i];

            int value = rand() % 64;
            while (!value_in_bitfield(value, remaining_values_for[i]))
                value = (value + 1) % 64;

            uint64_t bitfield = 1ULL << value;
            value_for[i] = value;
            quantum_map->variables[i] = bitfield;
            remaining_values_for[i] -= bitfield;

            continue;
        }

        // 4. If solution is not valid
        while (true)
        {
            // 4.1. If we have exhausted all possible solutions, error
            if (i == -1)
            {
                fprintf(stderr, "Could not find a valid solution");
                exit(EXIT_FAILURE);
            }

            // 4.2. If we have exhausted all possible values, fall back to the previous variable
            if (remaining_values_for[i] == 0)
            {
                i--;
                continue;
            }

            // 4.3. Select a random remaining value the variable could collapse to
            int value = rand() % 64;
            while (!value_in_bitfield(value, remaining_values_for[i]))
                value = (value + 1) % 64;

            value_for[i] = value;
            remaining_values_for[i] -= 1ULL << value;

            break;
        }

        // 5. Reset solution values
        reset_solution_values(quantum_map, i);
        for (int n = 0; n <= i; n++)
            quantum_map->variables[n] = 1ULL << value_for[n];

        reapply_single_arc_constraints = true;
    }
    // printf("\n");
}

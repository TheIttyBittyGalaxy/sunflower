#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "expression.h"
#include "solve.h"

// Evaluate arc expression
Value evaluate_arc_expression(Expression *expr, Value *given_values)
{
    switch (expr->kind)
    {
    case NUMBER_LITERAL:
    {
        return (Value){
            .kind = NUM_VAL,
            .num = expr->number};
    }

    case BIN_OP:
    {
        Value lhs = evaluate_arc_expression(expr->lhs, given_values);
        Value rhs = evaluate_arc_expression(expr->rhs, given_values);

        bool result = false;

        if (expr->op == EQUAL_TO)
            result = lhs.kind == rhs.kind && lhs.num == rhs.num; // FIXME: Using `num` regardless of the kind is probably error prone?

        else if (expr->op == LESS_THAN)
            result = lhs.num < rhs.num;

        else if (expr->op == MORE_THAN)
            result = lhs.num > rhs.num;

        else if (expr->op == LESS_THAN_OR_EQUAL)
            result = lhs.num <= rhs.num;

        else if (expr->op == MORE_THAN_OR_EQUAL)
            result = lhs.num <= rhs.num;

        return (Value){
            .kind = BOOL_VAL,
            .boolean = result};
    }

    case ARC_VALUE:
    {
        return given_values[expr->index];
    }

    default:
    {
        fprintf(stderr, "Unable to evaluate %s expression", expression_kind_string(expr->kind));
        print_expression(expr);
        exit(EXIT_FAILURE);
    }
    }
}

// Apply arc constraints
void enforce_single_arc_constrains(QuantumMap *quantum_map, Constraints constraints)
{
    Value given_value;
    given_value.kind = NUM_VAL; // TODO: This is temporary. Eventually not all variables will be numbers.

    for (size_t arc_index = 0; arc_index < constraints.single_arcs_count; arc_index++)
    {
        Arc *arc = constraints.single_arcs + arc_index;

        size_t var_index = arc->variable_indexes[0];
        uint64_t var_bitfield = quantum_map->variables[var_index];

        for (int value = 0; value < 64; value++)
        {
            uint64_t value_bitfield = (1ULL << value);
            if ((var_bitfield & value_bitfield) == 0)
                continue;

            given_value.num = value; // TODO: This is temporary. Eventually not all variables will be numbers.
            bool result = evaluate_arc_expression(arc->expr, &given_value).boolean;

            if (!result)
                var_bitfield -= value_bitfield;
        }

        quantum_map->variables[var_index] = var_bitfield;
    }
}

// CLEANUP: This code is confusing! Most of the complexity arises from trying to iterate the
//          other values. With some care, I imagine this could be implemented in a better way.

// TODO: Support for more than a fixed number of variables.
#define MAX_VARIABLES 16

void enforce_multi_arc_constraints(QuantumMap *quantum_map, Constraints constraints)
{
    // Array to store bitfield for each variable constrained by an arc
    uint64_t var_bitfields[MAX_VARIABLES];
#define primary_bitfield (var_bitfields[0]) // Access the first element of `var_bitfields` as `primary_bitfield`

    // Initialise array of possible values for each variable
    int var_values[MAX_VARIABLES];
#define primary_value (var_values[0]) // Access the first element of `var_values` as `primary_value`

    // Initialise array of expression values
    Value expression_values[MAX_VARIABLES];
    for (size_t i = 0; i < MAX_VARIABLES; i++)
        expression_values[i].kind = NUM_VAL; // TODO: This is temporary. Eventually not all variables will be numbers.

    // Enforce each arc
    for (size_t arc_index = 0; arc_index < constraints.multi_arcs_count; arc_index++)
    {
#define DEBUG(x) printf("> %d\n", x);

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
            var_bitfields[i] = quantum_map->variables[var_index];
        }

        // Test each potential value for the first variable to see if it should be eliminated
        for (primary_value = 0; primary_value < 64; primary_value++)
        {
            // Skip this value if it is already not a possibility
            if ((primary_bitfield & (1ULL << primary_value)) == 0)
                continue;

            // Reset the value of each variable to 0. These will be incremented as we test different combinations of possible values.
            for (size_t n = 1; n < MAX_VARIABLES; n++)
                var_values[n] = 0;

            // Determine if this value is a valid possibility
            bool primary_value_is_valid_possibility = false;
            while (true)
            {
                // Increment the value of each variable (excluding the first) until we encounter a set of variable values that are all possible
                size_t var_index = 1;
                while (var_index < total_variables)
                {
                    // While the nth value is not a possibility, increment it.
                    while ((var_bitfields[var_index] & (1ULL << var_values[var_index])) == 0)
                    {
                        var_values[var_index]++;
                        if (var_values[var_index] == 64)
                            break;
                    }

                    // If we've iterated through all possibilities for the nth value
                    if (var_values[var_index] == 64)
                    {
                        while (var_values[var_index] == 64)
                        {
                            var_index++;
                            if (var_index >= total_variables)
                                break;
                            else
                                var_values[var_index]++;
                        }

                        if (var_index >= total_variables)
                            break;

                        for (size_t s = 1; s < var_index; s++)
                            var_values[s] = 0;

                        var_index = 1;
                    }

                    // The nth value is now possible. Increment n to ensure that the next value is also possible.
                    else if (var_index + 1 < total_variables)
                    {
                        var_index++;
                    }

                    else
                    {
                        break;
                    }
                }

                if (var_index >= total_variables)
                    break;

                // Evaluate the set of possible variables to determine if the primary value is a valid possibility
                for (size_t n = 0; n < total_variables; n++)
                    expression_values[n].num = var_values[n]; // TODO: This is temporary. Eventually not all variables will be numbers.
                bool result = evaluate_arc_expression(arc->expr, expression_values).boolean;

                if (result)
                {
                    primary_value_is_valid_possibility = true;
                    break;
                }

                // Increment the set of variable values (excluding the primary variable)
                var_index = 1;
                var_values[var_index]++;

                while (var_values[var_index] == 64)
                {
                    var_values[var_index] = 0;
                    var_index++;
                    if (var_index >= total_variables)
                        break;
                    else
                        var_values[var_index]++;
                }

                if (var_index >= total_variables)
                    break;
            }

            if (!primary_value_is_valid_possibility)
                primary_bitfield -= (1ULL << primary_value);
        }

        quantum_map->variables[primary_index] = primary_bitfield;
    }
#undef var_bitfield
#undef var_value
}

// Solve
void solve(QuantumMap *quantum_map, Constraints constraints)
{
    enforce_single_arc_constrains(quantum_map, constraints);
    enforce_multi_arc_constraints(quantum_map, constraints);

    for (size_t i = 0; i < quantum_map->variables_count; i++)
    {
        uint64_t var_bitfield = quantum_map->variables[i];
        if (var_bitfield == 0)
        {
            // TODO: Handle this situation
            fprintf(stderr, "During solving, we reached a state where a variable  was reduced to zero possibilities.");
            exit(EXIT_FAILURE);
        }

        // Reduce variable to a single possibility
        size_t value = rand() % 64;
        while ((var_bitfield & (1ULL << value)) == 0)
            value = (value + 1) % 64;
        quantum_map->variables[i] = 1ULL << value;

        enforce_multi_arc_constraints(quantum_map, constraints);
    }
}
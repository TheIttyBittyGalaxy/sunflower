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
    given_value.kind = NUM_VAL; // TODO: This is temporary. Eventually not all values will be numbers.

    for (size_t i = 0; i < constraints.single_arcs_count; i++)
    {
        Arc *arc = constraints.single_arcs + i;

        size_t var_index = arc->value_indexes[0];
        uint64_t var_bitfield = quantum_map->values[var_index];

        for (int value = 0; value < 64; value++)
        {
            uint64_t value_bitfield = (1ULL << value);
            if ((var_bitfield & value_bitfield) == 0)
                continue;

            given_value.num = value; // TODO: This is temporary. Eventually not all values will be numbers.
            bool result = evaluate_arc_expression(arc->expr, &given_value).boolean;

            if (!result)
                var_bitfield -= value_bitfield;
        }

        quantum_map->values[var_index] = var_bitfield;
    }
}

// CLEANUP: This code is confusing! Most of the complexity arises from trying to iterate the
//          other values. With some care, I imagine this could be implemented in a better way.

void enforce_multi_arc_constraints(QuantumMap *quantum_map, Constraints constraints)
{
    for (size_t arc_index = 0; arc_index < constraints.multi_arcs_count; arc_index++)
    {
        Arc *arc = constraints.multi_arcs + arc_index;

        size_t number_of_values = arc->value_indexes_count;
        Value *given_values = (Value *)malloc(sizeof(Value) * number_of_values);

        for (size_t i = 0; i < number_of_values; i++)
            given_values[i].kind = NUM_VAL; // TODO: This is temporary. Eventually not all values will be numbers.

        size_t first_value_index = arc->value_indexes[0];
        uint64_t first_value_field = quantum_map->values[first_value_index];

        int *other_values = (int *)malloc(sizeof(int) * number_of_values);

        for (int value = 0; value < 64; value++)
        {
            if ((first_value_field & (1ULL << value)) == 0)
                continue;

            // 1. Init `i`s for each other value
            for (size_t n = 1; n < number_of_values; n++)
                other_values[n] = 0;

            bool first_value_is_possible = false;
            while (true)
            {
                // 2. If the current set of other values is not possible, increment until it is
                size_t n = 1;
                while (n < number_of_values)
                {
                    size_t other_value_index = arc->value_indexes[n];
                    uint64_t other_value_field = quantum_map->values[other_value_index];

                    // While the nth value is not a possibility, increment it.
                    while ((other_value_field & (1ULL << other_values[n])) == 0)
                    {
                        other_values[n]++;
                        if (other_values[n] == 64)
                            break;
                    }

                    // If we've iterated through all possibilities for the nth value
                    if (other_values[n] == 64)
                    {
                        while (other_values[n] == 64)
                        {
                            n++;
                            if (n >= number_of_values)
                                break;
                            else
                                other_values[n]++;
                        }

                        if (n >= number_of_values)
                            break;

                        for (size_t s = 1; s < n; s++)
                            other_values[s] = 0;

                        n = 1;
                    }

                    // The nth value is now possible. Increment n to ensure that the next value is also possible.
                    else if (n + 1 < number_of_values)
                    {
                        n++;
                    }

                    else
                    {
                        break;
                    }
                }

                if (n >= number_of_values)
                    break;

                // 3. Evaluate the expression given that possibility

                // TODO: Allow this to accept multiple values
                given_values[0].num = value;
                for (size_t v = 1; v < number_of_values; v++)
                    given_values[v].num = other_values[v];

                Value result = evaluate_arc_expression(arc->expr, given_values);

                // 4. if the result is true, we know the first value is possible
                if (result.boolean == true)
                {
                    first_value_is_possible = true;
                    break;
                }

                // 5. Increment the other value(s)
                n = 1;
                other_values[n]++;

                while (other_values[n] == 64)
                {
                    other_values[n] = 0;
                    n++;
                    if (n >= number_of_values)
                        break;
                    else
                        other_values[n]++;
                }

                if (n >= number_of_values)
                    break;
            }

            if (!first_value_is_possible)
            {
                first_value_field -= (1ULL << value);
            }
        }

        quantum_map->values[first_value_index] = first_value_field;
    }
}

// Solve
void solve(QuantumMap *quantum_map, Constraints constraints)
{
    enforce_single_arc_constrains(quantum_map, constraints);
    enforce_multi_arc_constraints(quantum_map, constraints);

    for (size_t i = 0; i < quantum_map->values_count; i++)
    {
        uint64_t value_field = quantum_map->values[i];
        if (value_field == 0)
        {
            // TODO: Handle this situation
            fprintf(stderr, "During solving, we reached a state where a value was reduced to zero possibilities.");
            exit(EXIT_FAILURE);
        }

        // Reduce value to a single possibility
        size_t value = rand() % 64;
        while ((value_field & (1ULL << value)) == 0)
            value = (value + 1) % 64;
        quantum_map->values[i] = 1ULL << value;

        enforce_multi_arc_constraints(quantum_map, constraints);
    }
}
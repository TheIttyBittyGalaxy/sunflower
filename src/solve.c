#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "expression.h"
#include "solve.h"

// Evaluate arc expression
Value evaluate_arc_expression(Expression *expr, Value given_value)
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
        Value lhs = evaluate_arc_expression(expr->lhs, given_value);
        Value rhs = evaluate_arc_expression(expr->rhs, given_value);

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
        return given_value;
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
void apply_arc_constraints(QuantumMap *quantum_map, ArcArray arcs)
{
    for (size_t i = 0; i < arcs.arcs_count; i++)
    {
        Arc *arc = arcs.arcs + i;
        uint64_t value_field = quantum_map->values[arc->value_index];

        for (int value = 0; value < 64; value++)
        {
            if ((value_field & (1ULL << value)) == 0)
                continue;

            Value result = evaluate_arc_expression(arc->expr, (Value){.kind = NUM_VAL, .num = value});
            if (result.boolean == false)
                value_field -= (1ULL << value);
        }

        quantum_map->values[arc->value_index] = value_field;
    }
}

// Solve
void solve(QuantumMap *quantum_map, ArcArray arcs)
{
    apply_arc_constraints(quantum_map, arcs);

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
        size_t bit = rand() % 64;
        while (!(value_field & (1ULL << bit)))
            bit = (bit + 1) % 64;
        quantum_map->values[i] = 1ULL << bit;

        apply_arc_constraints(quantum_map, arcs);
    }
}
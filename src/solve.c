#include <stdio.h>
#include <stdlib.h>

#include "solve.h"

void solve(QuantumMap *quantum_map, Program *program)
{
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

        // TODO: Apply constraints onto remaining values
    }
}
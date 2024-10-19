#include <stdio.h>

#include "solve.h"

void solve(QuantumMap *quantum_map, Program *program)
{
    for (size_t i = 0; i < quantum_map->values_count; i++)
    {
        uint64_t value = quantum_map->values[i];
        if (value == 0)
        {
            // TODO: Handle this situation
            fprintf(stderr, "During solving, we reached a state where a value was reduced to zero possibilities.");
            exit(EXIT_FAILURE);
        }

        // Reduce value to a single possibility
        // TODO: Do this randomly
        size_t d = 0;
        while (!(value & (1ULL << d)))
            d++;
        quantum_map->values[i] = 1ULL << d;

        // TODO: Apply constraints onto remaining values
    }
}
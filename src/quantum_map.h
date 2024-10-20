#ifndef QUANTUM_MAP_H
#define QUANTUM_MAP_H

#include <stdint.h>

#include "program.h"

// CLEANUP: Split data structure and `create_quantum_map` into separate source files.

// CLEANUP: Figure out a better name for this than "quantum map"

// QuantumInstance
typedef struct
{
    Node *node;
    size_t index_to_values_array;
} QuantumInstance;

// QuantumMap
typedef struct
{
    QuantumInstance *instances;
    size_t instances_count;
    uint64_t *values;
    size_t values_count;
} QuantumMap;

// Create quantum map
QuantumMap *create_quantum_map(Program *program);

// Printing & strings
void print_value(uint64_t value);
void print_quantum_map(QuantumMap *quantum_map);

#endif
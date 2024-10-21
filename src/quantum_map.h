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
    size_t variables_array_index;
} QuantumInstance;

// QuantumMap
typedef struct
{
    QuantumInstance *instances;
    size_t instances_count;
    uint64_t *variables;
    size_t variables_count;
} QuantumMap;

// Create quantum map
QuantumMap *create_quantum_map(Program *program);

// Bitfields
bool value_in_bitfield(int value, uint64_t bitfield);

// Printing & strings
void print_bitfield(uint64_t bitfield);
void print_quantum_map(QuantumMap *quantum_map);

#endif
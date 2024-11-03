#include <stdio.h>

#include "memory.h"
#include "quantum_map.h"

// Create quantum map

// TODO: Instead of having a fixed "INSTANCES_PER_NODE_DEC", have the number of
//       instances of each node type be passed as an argument into the function
const size_t INSTANCES_PER_NODE_DEC = 8;
QuantumMap *create_quantum_map(Program *program)
{
    QuantumMap *quantum_map = NEW(QuantumMap);
    quantum_map->instances_count = program->nodes_count * INSTANCES_PER_NODE_DEC;
    quantum_map->instances = (QuantumInstance *)malloc(sizeof(QuantumInstance) * quantum_map->instances_count);

    size_t instance_index = 0;
    size_t var_index = 0;
    for (size_t i = 0; i < program->nodes_count; i++)
    {
        Node *node = program->nodes + i;
        for (size_t j = 0; j < INSTANCES_PER_NODE_DEC; j++)
        {
            QuantumInstance *instance = quantum_map->instances + instance_index;
            instance->node = node;
            instance->variables_array_index = var_index;

            instance_index++;
            var_index += node->properties_count;
        }
    }

    quantum_map->variables_count = var_index;
    quantum_map->variables = (uint64_t *)malloc(sizeof(uint64_t) * quantum_map->variables_count);

    return quantum_map;
}

// Bitfields
bool value_in_bitfield(int value, uint64_t bitfield)
{
    return (bitfield & (1ULL << value)) > 0;
}

// Printing & strings
void print_bitfield(uint64_t bitfield)
{
    for (int i = 63; i >= 0; i--)
        putchar(value_in_bitfield(i, bitfield) ? '1' : '0');
}

void print_quantum_map(QuantumMap *quantum_map)
{
    for (size_t i = 0; i < quantum_map->instances_count; i++)
    {
        QuantumInstance *instance = quantum_map->instances + i;
        Node *node = instance->node;

        printf("%03d ", i);
        printf("%.*s\n", node->name.len, node->name.str);

        for (size_t p = 0; p < node->properties_count; p++)
        {
            Property *property = node->properties + p;
            uint64_t *var_bitfield = quantum_map->variables + (instance->variables_array_index + p);
            printf("\t%.*s:\t", property->name.len, property->name.str);
            print_bitfield(*var_bitfield);
            printf("\n");
        }
    }
}
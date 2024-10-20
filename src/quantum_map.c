#include <stdio.h>

#include "program.h" // CLEANUP: This is only being used for access to the "NEW" macro
#include "quantum_map.h"

// Create quantum map

// TODO: Instead of having a fixed "INSTANCES_PER_NODE_DEC", have the number of
//       instances of each node type be passed as an argument into the function
const size_t INSTANCES_PER_NODE_DEC = 2;
QuantumMap *create_quantum_map(Program *program)
{
    QuantumMap *quantum_map = NEW(QuantumMap);
    quantum_map->instances_count = program->nodes_count * INSTANCES_PER_NODE_DEC;
    quantum_map->instances = (QuantumInstance *)malloc(sizeof(QuantumInstance) * quantum_map->instances_count);

    size_t instance_index = 0;
    size_t value_index = 0;
    for (size_t i = 0; i < program->nodes_count; i++)
    {
        Node *node = program->nodes + i;
        for (size_t j = 0; j < INSTANCES_PER_NODE_DEC; j++)
        {
            QuantumInstance *instance = quantum_map->instances + instance_index;
            instance->node = node;
            instance->index_to_values_array = value_index;

            instance_index++;
            value_index += node->properties_count;
        }
    }

    quantum_map->values_count = value_index;
    quantum_map->values = (uint64_t *)malloc(sizeof(uint64_t) * quantum_map->values_count);
    for (size_t i = 0; i < quantum_map->values_count; i++)
        quantum_map->values[i] = UINT64_MAX;

    return quantum_map;
}

// Printing & strings
void print_value(uint64_t value)
{
    for (int i = 63; i >= 0; i--)
    {
        putchar((value & (1ULL << i)) ? '1' : '0');
    }
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
            uint64_t *value = quantum_map->values + (instance->index_to_values_array + p);
            printf("\t%.*s: ", property->name.len, property->name.str);
            print_value(*value);
            printf("\n");
        }
    }
}
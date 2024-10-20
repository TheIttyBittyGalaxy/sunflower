#include "collapse.h"
#include "program.h" // CLEANUP: This is only being used for access to the "NEW" macro

CollapsedMap *collapse(QuantumMap *quantum_map)
{
    CollapsedMap *collapsed_map = NEW(CollapsedMap);
    size_t instances_count = quantum_map->instances_count;

    collapsed_map->instances = (CollapsedInstance *)malloc(sizeof(CollapsedInstance) * instances_count);
    collapsed_map->instances_count = instances_count;

    for (size_t i = 0; i < instances_count; i++)
    {
        QuantumInstance *quantum_instance = quantum_map->instances + i;
        CollapsedInstance *collapsed_instance = collapsed_map->instances + i;
        Node *node = quantum_instance->node;

        collapsed_instance->node = node;
        collapsed_instance->variables = (uint_least8_t *)malloc(sizeof(uint_least8_t) * node->properties_count);

        for (size_t p = 0; p < node->properties_count; p++)
        {
            uint64_t value_map = quantum_map->variables[quantum_instance->variables_array_index + p];

            uint_least8_t value = 0;
            while (!(value_map & (1ULL << value)))
                value++;

            collapsed_instance->variables[p] = value;
        }
    }

    return collapsed_map;
}
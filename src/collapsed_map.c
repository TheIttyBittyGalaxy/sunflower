#include <stdio.h>

#include "collapsed_map.h"

void print_collapsed_map(CollapsedMap *collapsed_map)
{
    for (size_t i = 0; i < collapsed_map->instances_count; i++)
    {
        CollapsedInstance *instance = collapsed_map->instances + i;
        Node *node = instance->node;

        printf("%03d ", i);
        printf("%.*s\n", node->name.len, node->name.str);

        for (size_t p = 0; p < node->properties_count; p++)
        {
            Property *property = node->properties + p;
            uint_least8_t value = instance->values[p];
            printf("\t%.*s: %d\n", property->name.len, property->name.str, value);
        }
    }
}
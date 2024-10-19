#ifndef COLLAPSED_MAP_H
#define COLLAPSED_MAP_H

#include <stdint.h>

#include "program.h"

// CollapsedInstance
typedef struct
{
    Node *node;
    uint_least8_t *values;
} CollapsedInstance;

// CollapsedMap
typedef struct
{
    CollapsedInstance *instances;
    size_t instances_count;
} CollapsedMap;

// Printing & strings
void print_collapsed_map(CollapsedMap *collapsed_map);

#endif
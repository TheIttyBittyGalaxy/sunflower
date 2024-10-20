#ifndef ARC_H
#define ARC_H

#include <stdlib.h>

#include "program.h"
#include "quantum_map.h"

// CLEANUP: Split data structure and `create_arcs` into separate source files.

// Arc
typedef struct
{
    size_t value_index;
    Expression *expr; // CLEANUP: Factor `Expression` into it's own source file
} Arc;

// ArcArray
typedef struct
{
    Arc *arcs;
    size_t arcs_count;
} ArcArray;

// Create arcs
ArcArray create_arcs(Program *program, QuantumMap *quantum_map);

// Printing & strings
void print_arc(Arc *arc);
void print_arcs(ArcArray arcs);

#endif
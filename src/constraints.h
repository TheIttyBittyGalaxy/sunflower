#ifndef ARC_H
#define ARC_H

#include <stdlib.h>

#include "expression.h"
#include "program.h"
#include "quantum_map.h"

// CLEANUP: Split data structure and `create_constraints` into separate source files.

// Arc
typedef struct
{
    size_t *value_indexes;
    size_t value_indexes_count;
    Expression *expr;
} Arc;

// Constraints
typedef struct
{
    Arc *arcs;
    size_t arcs_count;
} Constraints;

// Create constraints
Constraints create_constraints(Program *program, QuantumMap *quantum_map);

// Printing & strings
void print_arc(Arc *arc);
void print_constraints(Constraints constraints);

#endif
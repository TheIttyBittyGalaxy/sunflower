#ifndef CONSTRAINTS_H
#define CONSTRAINTS_H

#include <stdlib.h>

#include "expression.h"
#include "program.h"
#include "quantum_map.h"

// CLEANUP: Split data structure and `create_constraints` into separate source files.

// Arc
typedef struct
{
    size_t *instance_indexes;
    size_t instance_indexes_count;
    size_t *variable_indexes;
    size_t variable_indexes_count;
    size_t expr_rotation;
    Expression *expr;
} Arc;

// Constraints
typedef struct
{
    Arc *single_arcs;
    size_t single_arcs_count;
    Arc *multi_arcs;
    size_t multi_arcs_count;
} Constraints;

// Create constraints
Constraints create_constraints(Program *program, QuantumMap *quantum_map);

// Printing & strings
void print_arc(Arc *arc);
void print_constraints(Constraints constraints);

#endif
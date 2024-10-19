#include <stdlib.h>

#include "program.h"

Program *create_program()
{
    Program *program = (Program *)malloc(sizeof(Program));

    // program->node.name = {.str = NULL, .len = 0};
    // program->node.property_name = {.str = NULL, .len = 0};
    // program->node.property_kind_name = {.str = NULL, .len = 0};

    return program;
}
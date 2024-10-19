#include <stdlib.h>

#include "program.h"

Program *create_program()
{
    Program *program = (Program *)malloc(sizeof(Program));

    program->node.name = NULL;
    program->node.name_len = 0;
    program->node.property_name = NULL;
    program->node.property_name_len = 0;
    program->node.property_kind_name = NULL;
    program->node.property_kind_name_len = 0;

    return program;
}
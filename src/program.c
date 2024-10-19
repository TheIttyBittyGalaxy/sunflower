#include <stdlib.h>

#include "program.h"

Program *create_program()
{
    Program *program = (Program *)malloc(sizeof(Program));

    program->node.name = NULL_SUB_STRING;
    program->node.property_name = NULL_SUB_STRING;
    program->node.property_kind_name = NULL_SUB_STRING;

    return program;
}
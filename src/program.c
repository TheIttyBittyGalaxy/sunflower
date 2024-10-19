#include <stdlib.h>

#include "program.h"

Program *create_program()
{
    Program *program = (Program *)malloc(sizeof(Program));

    program->node.name = NULL_SUB_STRING;

    return program;
}
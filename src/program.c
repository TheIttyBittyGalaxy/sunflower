#include <stdio.h>
#include <stdlib.h>

#include "program.h"

// Strings & printing
void print_program(const Program *program)
{
    printf("PROGRAM\n");
    for (size_t i = 0; i < program->nodes_count; i++)
        print_node(program->nodes + i);
}

void print_node(const Node *node)
{
    printf("%.*s\n", node->name.len, node->name.str);
    for (size_t i = 0; i < node->property_count; i++)
    {
        Property p = node->properties[i];
        printf("\t- %.*s: %.*s\n", p.name.len, p.name.str, p.kind_name.len, p.kind_name.str);
    }
}
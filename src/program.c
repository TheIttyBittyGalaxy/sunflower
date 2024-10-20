#include <stdio.h>

#include "program.h"

// Strings & printing
void print_program(const Program *program)
{
    printf("PROGRAM\n");

    printf("\tNODES:\n");
    for (size_t i = 0; i < program->nodes_count; i++)
        print_node(program->nodes + i);

    printf("\tRULES:\n");
    for (size_t i = 0; i < program->rules_count; i++)
        print_rule(program->rules + i);
}

void print_node(const Node *node)
{
    printf("\t\t%.*s { ", node->name.len, node->name.str);
    for (size_t i = 0; i < node->properties_count; i++)
    {
        if (i > 0)
            printf(", ");
        Property p = node->properties[i];
        printf("%.*s: %.*s", p.name.len, p.name.str, p.kind_name.len, p.kind_name.str);
    }
    printf(" }\n");
}

void print_rule(const Rule *rule)
{
    printf("\t\t");
    for (size_t i = 0; i < rule->variables_count; i++)
    {
        if (i > 0)
            printf(", ");
        Variable var = rule->variables[i];
        printf("%.*s %.*s", var.node_name.len, var.node_name.str, var.name.len, var.name.str);
    }
    printf(" -> ");
    print_expression(rule->expression);
    printf("\n");
}

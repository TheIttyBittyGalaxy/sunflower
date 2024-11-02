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
        printf("%.*s: ", p.name.len, p.name.str);

        if (p.type == TYPE_PRIMITIVE__INVALID)
            printf("INVALID TYPE (%.*s)", p.type_name.len, p.type_name.str);
        else if (p.type == TYPE_PRIMITIVE__NUMBER)
            printf("NUM");
        else if (p.type == TYPE_PRIMITIVE__BOOL)
            printf("BOOL");
        else if (p.type == TYPE_PRIMITIVE__NODE)
            printf("NODE %.*s (%.*s)", p.node_type->name.len, p.node_type->name.str, p.type_name.len, p.type_name.str);
        else
            printf("INVALID (%.*s)", p.type_name.len, p.type_name.str);
    }
    printf(" }\n");
}

void print_rule(const Rule *rule)
{
    printf("\t\t");
    for (size_t i = 0; i < rule->placeholders_count; i++)
    {
        if (i > 0)
            printf(", ");
        Placeholder placeholder = rule->placeholders[i];
        printf("%.*s %.*s", placeholder.node_name.len, placeholder.node_name.str, placeholder.name.len, placeholder.name.str);
    }
    printf(" -> ");
    print_expression(rule->expression);
    printf("\n");
}

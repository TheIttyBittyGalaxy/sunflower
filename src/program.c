#include <stdio.h>

#include "program.h"

// Strings & printing
const char *type_category_string(TypeCategory type_category)
{
    if (type_category == TYPE_NULL)
        return "TYPE_NULL";
    if (type_category == TYPE_NUM)
        return "TYPE_NUM";
    if (type_category == TYPE_NODE)
        return "TYPE_NODE";

    return "INVALID_TYPE_CATEGORY";
}

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

        if (p.type == TYPE_NULL)
            printf("NULL (%.*s)", p.type_name.len, p.type_name.str);
        else if (p.type == TYPE_NUM)
            printf("NUM");
        else if (p.type == TYPE_NODE)
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

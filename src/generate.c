#include <stdio.h>
#include <stdlib.h>

#include "generate.h"

// generate
void generate(Program *program)
{
    FILE *file = fopen("out/main.c", "w");

    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

#define PRINT(...)              \
    fprintf(file, __VA_ARGS__); \
    fprintf(file, "\n")

#define WRITE(...)              \
    fprintf(file, __VA_ARGS__); \
    fprintf(file, "\n")

    // Includes
    PRINT("#include <stdbool.h>");
    PRINT("#include <stdint.h>");
    PRINT("#include <stdio.h>");
    PRINT("#include <stdlib.h>");
    PRINT("#include <string.h>");

    // Open main
    PRINT("int main(int argc, char *argv[])");
    PRINT("{");

    // Throw error if there are no arguments
    PRINT("    if (argc < 2) {");
    PRINT("        fprintf(stderr, \"Usage: %%s node_name=node_count\\n\", argv[0]);");
    PRINT("        return EXIT_FAILURE;");
    PRINT("    }");

    // Store count for reach node type
    PRINT("    size_t %.*s_count = 0;", program->node.name_len, program->node.name);

    // Process arguments
    PRINT("    for (int i = 1; i < argc; i++) {");
    PRINT("        char *arg = argv[i];");
    PRINT("        char *equals_sign = strchr(arg, '=');");

    // Throw error if argument does not contain an equal sign
    PRINT("        if (equals_sign == NULL) {");
    PRINT("            fprintf(stderr, \"Invalid argument '%%s'\\n\", arg);");
    PRINT("            return EXIT_FAILURE;");
    PRINT("        }");

    // Update node count
    // TODO: Output program should give error if a node has it's count specified more than once
    PRINT("        *equals_sign = '\\0';");
    PRINT("        int value = atoi(equals_sign + 1);"); // TODO: Output program should give an error if the argument value is invalid
    PRINT("        if (strcmp(arg, \"%.*s\") == 0)", program->node.name_len, program->node.name);
    PRINT("            %.*s_count = value;", program->node.name_len, program->node.name);
    PRINT("        else {");

    // Give error if there is no node with the name given
    PRINT("            fprintf(stderr, \"Invalid argument '%%s', there is no node with this name\\n\", argv[i]);");
    PRINT("            return EXIT_FAILURE;");
    PRINT("        }");

    PRINT("    }");

    // Output counts
    PRINT("    printf(\"%.*s: %%d\\n\", %.*s_count);", program->node.name_len, program->node.name, program->node.name_len, program->node.name);

    // Node values
    PRINT("    size_t value_count = %.*s_count * 1;", program->node.name_len, program->node.name);
    PRINT("    uint64_t* values = (uint64_t *)malloc(value_count * sizeof(uint64_t));");
    PRINT("    for (size_t i = 0; i < value_count; i++) values[i] = UINT64_MAX;");

    // Close main
    PRINT("    return EXIT_SUCCESS;");
    PRINT("}");

    fclose(file);
}
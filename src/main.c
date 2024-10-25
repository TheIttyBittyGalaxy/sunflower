#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "constraints.h"
#include "collapse.h"
#include "collapsed_map.h"
#include "parse.h"
#include "resolve.h"
#include "program.h"
#include "quantum_map.h"
#include "solve.h"
#include "token.h"
#include "tokenise.h"

#define PRINT_HEADING(text) printf("\x1b[32m" text "\n\x1b[0m")

int main(int argc, char const *argv[])
{
    // Validate arguments
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <file_path> [-all] [-t] [-p] [-r] [-q] [-c] [-s] [-f]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Parse flags
    bool flag_output_tokens = false;        // -t
    bool flag_output_parse = false;         // -p
    bool flag_output_resolve = false;       // -r
    bool flag_output_quantum_map = false;   // -q
    bool flag_output_constraints = false;   // -c
    bool flag_output_solved_map = false;    // -s
    bool flag_output_collapsed_map = false; // -f

    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "-all") == 0)
        {
            flag_output_tokens = true;
            flag_output_parse = true;
            flag_output_resolve = true;
            flag_output_quantum_map = true;
            flag_output_constraints = true;
            flag_output_solved_map = true;
            flag_output_collapsed_map = true;
        }
        else if (strcmp(argv[i], "-t") == 0)
            flag_output_tokens = true;
        else if (strcmp(argv[i], "-p") == 0)
            flag_output_parse = true;
        else if (strcmp(argv[i], "-r") == 0)
            flag_output_resolve = true;
        else if (strcmp(argv[i], "-q") == 0)
            flag_output_quantum_map = true;
        else if (strcmp(argv[i], "-c") == 0)
            flag_output_constraints = true;
        else if (strcmp(argv[i], "-s") == 0)
            flag_output_solved_map = true;
        else if (strcmp(argv[i], "-f") == 0)
            flag_output_collapsed_map = true;
        else
        {
            fprintf(stderr, "Usage: %s <file_path> [-all] [-t] [-p] [-r] [-q] [-c] [-s] [-f]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }

    // Initialise RNG
    srand(time(NULL));

    // Read source file
    const char *source_path = argv[1];
    PRINT_HEADING("READING SOURCE FILE");
    char *source_text = NULL;
    {
        FILE *source_file = fopen(source_path, "rb");

        if (source_file == NULL)
        {
            fprintf(stderr, "Error reading file %s\n", source_path);
            return EXIT_FAILURE;
        }

        fseek(source_file, 0, SEEK_END);
        long file_size = ftell(source_file);

        source_text = (char *)malloc(file_size + 1);
        if (source_text == NULL)
        {
            fclose(source_file);
            fprintf(stderr, "Unable to allocate memory to store source text\n");
            return EXIT_FAILURE;
        }

        fseek(source_file, 0, SEEK_SET);
        fread(source_text, 1, file_size, source_file);
        source_text[file_size] = '\0';

        fclose(source_file);
    }

    // Tokenise
    PRINT_HEADING("TOKENISING");
    TokenArray source_tokens = tokenise(source_text);

    if (flag_output_tokens)
    {
        print_tokens(source_tokens);
        printf("\n");
    }

    // Parse
    PRINT_HEADING("PARSING");
    Program *program = parse(source_tokens);

    if (flag_output_parse)
    {
        print_program(program);
        printf("\n");
    }

    // Resolve
    PRINT_HEADING("RESOLVING");
    resolve(program);

    if (flag_output_resolve)
    {
        print_program(program);
        printf("\n");
    }

    // Create quantum-map
    PRINT_HEADING("CREATING QUANTUM MAP");
    QuantumMap *quantum_map = create_quantum_map(program);

    if (flag_output_quantum_map)
    {
        printf("%d node instances, resulting in %d variables\n", quantum_map->instances_count, quantum_map->variables_count);
        print_quantum_map(quantum_map);
        printf("\n");
    }

    // Create constraints
    PRINT_HEADING("CREATING CONSTRAINTS");
    Constraints constraints = create_constraints(program, quantum_map);

    if (flag_output_constraints)
    {
        print_constraints(constraints);
        printf("\n");
    }

    // Solve quantum-map
    PRINT_HEADING("SOLVING QUANTUM MAP");
    solve(quantum_map, constraints);

    if (flag_output_solved_map)
    {
        print_quantum_map(quantum_map);
        printf("\n");
    }

    // Collapse quantum-map to regular map
    PRINT_HEADING("COLLAPSING MAP");
    CollapsedMap *collapsed_map = collapse(quantum_map);

    if (flag_output_collapsed_map)
    {
        print_collapsed_map(collapsed_map);
        printf("\n");
    }

    PRINT_HEADING("COMPILER COMPLETE");
    return EXIT_SUCCESS;
}

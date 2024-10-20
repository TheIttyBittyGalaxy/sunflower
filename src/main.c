#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "collapse.h"
#include "collapsed_map.h"
#include "parse.h"
#include "resolve.h"
#include "program.h"
#include "quantum_map.h"
#include "solve.h"
#include "token.h"
#include "tokenise.h"

// MAIN //

int main(int argc, char const *argv[])
{
    // Validate arguments
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Initialise RNG
    srand(time(NULL));

    // Read source file
    const char *source_path = argv[1];
    printf("Reading source file %s\n", source_path);
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
    printf("\n");

    // Tokenise
    printf("Tokenising\n");
    TokenArray source_tokens = tokenise(source_text);

    printf("Result:\n");
    print_tokens(source_tokens);
    printf("\n");

    // Parse
    printf("Parsing\n");
    Program *program = parse(source_tokens);

    printf("Result:\n");
    print_program(program);
    printf("\n");

    // Resolve
    printf("Resolving\n");
    resolve(program);

    printf("Result:\n");
    print_program(program);
    printf("\n");

    // Create quantum-map
    printf("Creating quantum map\n");
    QuantumMap *quantum_map = create_quantum_map(program);
    printf("%d node instances, resulting in %d values\n", quantum_map->instances_count, quantum_map->values_count);

    printf("Result:\n");
    print_quantum_map(quantum_map);
    printf("\n");

    // Solve quantum-map
    printf("Solving quantum map\n");
    solve(quantum_map, program);

    printf("Result:\n");
    print_quantum_map(quantum_map);
    printf("\n");

    // Collapse quantum-map to regular map
    printf("Collapsing map\n");
    CollapsedMap *collapsed_map = collapse(quantum_map);

    printf("Result:\n");
    print_collapsed_map(collapsed_map);
    printf("\n");

    printf("Compiler complete\n");
    return EXIT_SUCCESS;
}

#include <stdio.h>

#include "parse.h"
#include "program.h"
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

    printf("Tokens:\n");
    print_tokens(source_tokens);
    printf("\n");

    // Parse
    printf("Parsing\n");
    Program *program = create_program();
    parse(program, source_tokens);

    printf("Result:\n");
    Node node = program->node;
    print_program(program);
    printf("\n");

    // TODO: interpret and solve

    printf("Compiler complete\n");
    return EXIT_SUCCESS;
}

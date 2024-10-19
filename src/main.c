#include <stdio.h>

#include "generate.h"
#include "parse.h"
#include "program.h"
#include "token.h"
#include "tokenise.h"

// MAIN //

int main(int argc, char const *argv[])
{
    printf("Running compiler\n\n");

    // TODO: load sunflower program
    // TEMP: Use hardcoded program instead
    const char *source_text = "Thing{value:num\n}";

    printf("Tokenising\n");
    TokenArray source_tokens = tokenise(source_text);

    printf("Tokens:\n");
    print_tokens(source_tokens);
    printf("\n");

    printf("Parsing\n");
    Program *program = create_program();
    parse(program, source_tokens);

    printf("Result:\n");
    printf("\tname = %.*s\n", program->node.name_len, program->node.name);
    printf("\tprop = %.*s\n", program->node.property_name_len, program->node.property_name);
    printf("\tkind = %.*s\n", program->node.property_kind_name_len, program->node.property_kind_name);
    printf("\n");

    printf("Generating:\n");
    generate(program);

    printf("Compiler complete\n");
    return 0;
}

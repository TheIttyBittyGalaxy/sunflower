#include <stdio.h>

#include "token.h"
#include "tokenise.h"
#include "util.h"

// MAIN //

int main(int argc, char const *argv[])
{
    printf("start\n");

    // TODO: load sunflower program

    // TEMP: Use hardcoded program instead
    const char *source_text = "Node{value:num\n}";

    // TODO: tokenise sunflower program
    printf("tokenise\n");
    TokenArray source_tokens = tokenise(source_text);

    printf("tokens\n");
    print_tokens(source_tokens);

    // TODO: parse sunflower program

    // TODO: generate executable

    printf("complete\n");
    return 0;
}

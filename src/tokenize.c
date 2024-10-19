#include <stdlib.h>

#include "tokenise.h"
#include "util.h"

// Utility functions
boolean is_word(const char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

boolean is_numeral(const char c)
{
    return (c >= '0' && c <= '9');
}

boolean is_word_or_numeral(const char c)
{
    return is_word(c) || is_numeral(c);
}

// Tokenise
TokenArray tokenise(const char *const src)
{
    TokenArray tokens;
    initialise_token_array(&tokens, 8); // TODO: What is a good starting value for the length of the array?

    const char *c = src;
    const char *line_start = src;
    size_t line = 1;

    while (*c != '\0')
    {
        Token t = {
            .kind = INVALID_TOKEN,
            .str = c,
            .length = 1,
            .line = line,
            .column = (size_t)(c - line_start) + 1};

        if (*c == ':')
        {
            t.kind = COLON;
            c++;
        }
        else if (*c == '{')
        {
            t.kind = OPEN;
            c++;
        }
        else if (*c == '}')
        {
            t.kind = CLOSE;
            c++;
        }
        else if (*c == '\n')
        {
            t.kind = LINE;
            c++;
            line++;
            line_start = c;
        }
        else if (*c == ' ' || *c == '\t' || *c == '\r')
        {
            c++;
            continue; // Skip this character without generating a token
        }
        else if (is_word(*c))
        {
            t.kind = NAME;
            while (is_word_or_numeral(*(++c)))
                t.length++;
        }
        else
        {
            c++;
        }

        append_token_to_array(&tokens, t);
    }

    Token eof;
    eof.str = c;
    eof.length = 1;
    eof.kind = END_OF_FILE;
    eof.line = line;
    eof.column = c - line_start + 1;
    append_token_to_array(&tokens, eof);

    return tokens;
}
